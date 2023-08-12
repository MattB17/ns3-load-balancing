/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ipv4-letflow-routing.h"

#include "ns3/channel.h"
#include "ns3/flow-id-tag.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4LetFlowRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4LetFlowRouting);

TypeId Ipv4LetFlowRouting::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::Ipv4LetFlowRouting")
	    .SetParent<Object>()
	    .SetGroupName("Internet")
	    .AddAttribute("FlowletTimeout",
	    	            "The minimum inter-packet arrival gap to denote a new "
	    	            "flowlet",
	    	            TimeValue(MicroSeconds(50)),
	    	            MakeTimeAccessor(&Ipv4LetFlowRouting::m_flowletTimeout),
	    	            MakeTimeChecker());
	return tid;
}

// Set the flowlet timeout to 50 microseconds.
Ipv4LetFlowRouting::Ipv4LetFlowRouting(Ptr<Ipv4GlobalRouting> globalRouting)
  : m_flowletTimeout(MicroSeconds(50)),
    m_ipv4(nullptr),
    m_globalRouting(globalRouting) 
{
    NS_LOG_FUNCTION(this);
}

Ipv4LetFlowRouting::~Ipv4LetFlowRouting() {
	NS_LOG_FUNCTION(this);
}

Ptr<Ipv4Route> Ipv4LetFlowRouting::RouteOutput(Ptr<Packet> packet,
	                                           const Ipv4Header& header,
	                                           Ptr<NetDevice> oif,
	                                           Socket::SocketErrno& sockerr) {
	NS_LOG_FUNCTION(this << packet << &header << oif << &sockerr);
	// Delegate to Global Routing. LetFlow is only implemented in the network
	// and does not extend to the hosts.
	return m_globalRouting->RouteOutput(packet, header, oif, sockerr);
}

// Receive an input packet on input device `idev`.
bool Ipv4LetFlowRouting::RouteInput(Ptr<const Packet> p,
	                                const Ipv4Header& header,
	                                Ptr<const NetDevice> idev,
	                                UnicastForwardCallback ucb,
	                                MulticastForwardCallback mcb,
	                                LocalDeliverCallback lcb,
	                                ErrorCallback ecb) {
	NS_LOG_LOGIC(this << " Route Input: " << p << " IP header: " << header);
	NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);

	Ptr<Packet> packet = ConstCast<Packet>(p);
	Ipv4Address dstAddress = header.GetDestination();

	// LetFlow routing only supports unicast.
	if (dstAddress.IsMulticast() || dstAddress.IsBroadcast()) {
		NS_LOG_ERROR(this <<  " LetFlow routing only supports unicast");
		ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
		return false;
	}

	// Check if the input device supports IP forwarding.
	uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);
	if (m_ipv4->IsForwarding(iif) == false) {
		NS_LOG_ERROR(this << " Forwarding is disabled for this interface");
		ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
		return false;
	}

	// Packet arrival time.
	Time now = Simulator::Now();

	// Extract the flow ID.
	uint32_t flowId = 0;
	// A class to hold the flow ID and handle serialization and
	// deserialization.
	FlowIdTag flowIdTag;
	// Flow ID for the packet.
	bool flowIdFound = packet->PeekPacketTag(flowIdTag);
	if (!flowIdFound) {
		NS_LOG_ERROR(this << " LetFlow routing cannot extract the flow ID, "
			         << "falling back to global routing");
		return m_globalRouting->RouteInput(p, header, idev, ucb, mcb, lcb, ecb);
	}
	// If the flow ID was found, extract it.
	flowId = flowIdTag.GetFlowId();

	// We need to select a port for the packet.
	uint32_t selectedPort;

    // We first examine the flowlet table to see if it is part of an active
    // flowlet.
	std::map<uint32_t, struct LetFlowFlowlet>::iterator flowletItr =
	    m_flowletTable.find(flowId);
	// If the flowlet table entry is valid, return the port.
	if (flowletItr != m_flowletTable.end()) {
	    // Get the flowlet from the iterator.
	    LetFlowFlowlet flowlet = flowletItr->second;
	    // If the interpacket gap is less than the flowlet timeout.
	    if (now - flowlet.activeTime <= m_flowletTimeout) {
	    	NS_LOG_LOGIC(this << " Found active flowlet for " << flowId);
	    	// Update the flowlet last active time and get the port.
	    	flowlet.activeTime = now;
	    	selectedPort = flowlet.port;

	    	// Construct the route.
	    	Ptr<Ipv4Route> route = Ipv4LetFlowRouting::ConstructIpv4Route(
	    		selectedPort, dstAddress);
	    	ucb(route, packet, header);

	    	// Update the flowlet table.
	    	m_flowletTable[flowId] = flowlet;

	    	return true;
	    }
	}

	// Otherwise, the flowlet either timed out or we don't have a flowlet
	// entry, so we get all known routes to the destination.
	std::vector<Ipv4RoutingTableEntry*> routeEntries =
	    Ipv4LetFlowRouting::LookupLetFlowRoutes(dstAddress);

	// Return error if there are no routing entries to the destination.
	if (routeEntries.empty()) {
	    NS_LOG_ERROR(this << " LetFlow routing cannot find routing entry");
	    ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
	    return false;
	}

	// Otherwise, there is a route so we need to decide through which port we
	// should forward the packet.
	NS_LOG_LOGIC(this << " Creating new flowlet for " << flowId);
	selectedPort = routeEntries[rand() % routeEntries.size()]->GetInterface();

	// Construct the flowlet and route.
	LetFlowFlowlet flowlet;
	flowlet.port = selectedPort;
	flowlet.activeTime = now;
	// Update the flowlet table.
	m_flowletTable[flowId] = flowlet;

	// Construct the route.
	Ptr<Ipv4Route> route = Ipv4LetFlowRouting::ConstructIpv4Route(
		selectedPort, dstAddress);
	ucb(route, packet, header);
	return true;
}

void Ipv4LetFlowRouting::NotifyInterfaceUp(uint32_t interface) {
	m_globalRouting->NotifyInterfaceUp(interface);
}

void Ipv4LetFlowRouting::NotifyInterfaceDown(uint32_t interface) {
	m_globalRouting->NotifyInterfaceDown(interface);
}

void Ipv4LetFlowRouting::NotifyAddAddress(
	uint32_t interface, Ipv4InterfaceAddress address) {
	m_globalRouting->NotifyAddAddress(interface, address);
}

void Ipv4LetFlowRouting::NotifyRemoveAddress(
	uint32_t interface, Ipv4InterfaceAddress address) {
	m_globalRouting->NotifyRemoveAddress(interface, address);
}

// Set the IPv4 address.
void Ipv4LetFlowRouting::SetIpv4(Ptr<Ipv4> ipv4) {
	NS_LOG_LOGIC(this << " Setting up IPv4: " << ipv4);
	NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
	m_ipv4 = ipv4;
}

void Ipv4LetFlowRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
	                                       Time::Unit unit) const {
	m_globalRouting->PrintRoutingTable(stream, unit);
}

void Ipv4LetFlowRouting::DoDispose(void) {
	NS_LOG_FUNCTION(this);
	m_ipv4 = nullptr;
	m_globalRouting->DoDispose();
	m_globalRouting = nullptr;
}

uint32_t Ipv4LetFlowRouting::GetNRoutes() const {
	NS_LOG_FUNCTION(this);
	return m_globalRouting->GetNRoutes();
}

Ipv4RoutingTableEntry* Ipv4LetFlowRouting::GetRoute(uint32_t i) const {
	NS_LOG_FUNCTION(this << " " << i);
	return m_globalRouting->GetRoute(i);
}

std::vector<Ipv4RoutingTableEntry*>
Ipv4LetFlowRouting::LookupLetFlowRoutes(Ipv4Address dst, Ptr<NetDevice> oif) {
	NS_LOG_FUNCTION(this  << dst  << oif);
	return m_globalRouting->GetRoutesToDst(dst, oif);
}

Ptr<Ipv4Route> Ipv4LetFlowRouting::ConstructIpv4Route(
	uint32_t port, Ipv4Address dstAddress) {
	// Port and channel to send from on this router.
	Ptr<NetDevice> dev = m_ipv4->GetNetDevice(port);
	Ptr<Channel> channel = dev->GetChannel();

	// Get the device on the other end of the channel.
	uint32_t otherEnd = (channel->GetDevice(0) == dev) ? 1 : 0;
	// Get the node and interface to send to at the next hop.
	Ptr<Node> nextHop = channel->GetDevice(otherEnd)->GetNode();
	// Interface index.
	uint32_t nextIf = channel->GetDevice(otherEnd)->GetIfIndex();
	// Get the IP address of the next hop.
	Ipv4Address nextHopAddr =
	    nextHop->GetObject<Ipv4>()->GetAddress(nextIf, 0).GetLocal();

	// Construct the route. Note the route just tells the router what the
	// next hop to send to is given the destination.
	Ptr<Ipv4Route> route = Create<Ipv4Route>();
	route->SetOutputDevice(m_ipv4->GetNetDevice(port));
	route->SetGateway(nextHopAddr);
	route->SetSource(m_ipv4->GetAddress(port, 0).GetLocal());
	route->SetDestination(dstAddress);
	return route;
}

void Ipv4LetFlowRouting::SetFlowletTimeout(Time timeout) {
	m_flowletTimeout = timeout;
}

}  // namespace ns3