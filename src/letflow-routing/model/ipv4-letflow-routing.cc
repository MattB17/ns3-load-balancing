/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ipv4-letflow-routing.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/flow-id-tag.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4LetFlowRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4LetFlowRouting);

// Set the flowlet timeout to 50 microseconds.
Ipv4LetFlowRouting::Ipv4LetFlowRouting(): m_flowletTimeout(MicroSeconds(50)),
                                          m_ipv4(0) {
    NS_LOG_FUNCTION(this);
}

Ipv4LetFlowRouting::~Ipv4LetFlowRouting() {
	NS_LOG_FUNCTION(this);
}

TypeId Ipv4LetFlowRouting::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::Ipv4LetFlowRouting")
	    .SetParent<Object>()
	    .SetGroupName("Internet")
	    .AddConstructor<Ipv4LetFlowRouting>();
	return tid;
}

void Ipv4LetFlowRouting::AddRoute(Ipv4Address network, Ipv4Mask networkMask,
	                              uint32_t port) {
	NS_LOG_LOGIC(this << " Add LetFlow routing entry: " << network << "/"
		<< networkMask << " would go through port: " << port);
	LetFlowRouteEntry letFlowRouteEntry;
	letFlowRouteEntry.network = network;
	letFlowRouteEntry.networkMask = networkMask;
	letFlowRouteEntry.port = port;
	m_routeEntryList.push_back(letFlowRouteEntry);
}

Ptr<Ipv4Route> Ipv4LetFlowRouting::RouteOutput(
	Ptr<Packet> packet, const Ipv4Header& header, Ptr<NetDevice> oif,
	Socket::SocketErrno& sockerr) {
	NS_LOG_ERROR(
		this << " LetFlow routing is not supported for local routing output");
	return 0;
}

// Receive an input packet on input device `idev`.
bool Ipv4LetFlowRouting::RouteInput(
	Ptr<const Packet> p, const Ipv4Header& header, Ptr<const NetDevice> idev,
	UnicastForwardCallback ucb, MulticastForwardCallback mcb,
	LocalDeliverCallback lcb, ErrorCallback ecb) {
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
		NS_LOG_ERROR(this << " LetFlow routing cannot extract the flow ID");
		ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
		return false;
	}
	// If the flow ID was found, extract it.
	flowId = flowIdTag.GetFlowId();

	// Getting the routing entries to the destination.
	std::vector<LetFlowRouteEntry> routeEntries =
	    Ipv4LetFlowRouting::LookupLetFlowRouteEntries(dstAddress);

	// Return error if there are no routing entries to the destination.
	if (routeEntries.empty()) {
	    NS_LOG_ERROR(this << " LetFlow routing cannot find routing entry");
	    ecb(packet, header, Socket::ERROR_NOROUTETOHOST);
	    return false;
	}

	// Otherwise, there is a route so we need to decide through which port we
	// should forward the packet.
	uint32_t selectedPort;

	std::map<uint32_t, struct LetFlowFlowlet>::iterator flowletItr =
	    m_flowletTable.find(flowId);
	// If the flowlet table entry is valid, return the port.
	if (flowletItr != m_flowletTable.end()) {
	    // Get the flowlet from the iterator.
	    LetFlowFlowlet flowlet = flowletItr->second;
	    // If the interpacket gap is less than the flowlet timeout.
	    if (now - flowlet.activeTime <= m_flowletTimeout) {
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

	// Otherwise no flowlet entry was found or it timed out.
	// So select a random port among the known routes to the destination.
	selectedPort = routeEntries[rand() % routeEntries.size()].port;

	// Construct the flowlet and route.
	LetFlowFlowlet flowlet;
	flowlet.port = selectedPort;
	flowlet.activeTime = now;
	Ptr<Ipv4Route> route = Ipv4LetFlowRouting::ConstructIpv4Route(
		selectedPort, dstAddress);
	ucb(route, packet, header);

	// Update the flowlet table.
	m_flowletTable[flowId] = flowlet;

	return true;
}

void Ipv4LetFlowRouting::NotifyInterfaceUp(uint32_t interface) {}

void Ipv4LetFlowRouting::NotifyInterfaceDown(uint32_t interface) {}

void Ipv4LetFlowRouting::NotifyAddAddress(
	uint32_t interface, Ipv4InterfaceAddress address) {
}

void Ipv4LetFlowRouting::NotifyRemoveAddress(
	uint32_t interface, Ipv4InterfaceAddress address) {
}

// Set the IPv4 address.
void Ipv4LetFlowRouting::SetIpv4(Ptr<Ipv4> ipv4) {
	NS_LOG_LOGIC(this << " Setting up IPv4: " << ipv4);
	NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
	m_ipv4 = ipv4;
}

void Ipv4LetFlowRouting::PrintRoutingTable(
	Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {
}

void Ipv4LetFlowRouting::DoDispose(void) {
	m_ipv4 = 0;
	Ipv4RoutingProtocol::DoDispose();
}

std::vector<LetFlowRouteEntry>
Ipv4LetFlowRouting::LookupLetFlowRouteEntries(Ipv4Address dst) {
	std::vector<LetFlowRouteEntry> letFlowRouteEntries;
	std::vector<LetFlowRouteEntry>::iterator itr = m_routeEntryList.begin();
	for (; itr != m_routeEntryList.end(); itr++) {
		// If the network mask matches the destination add it to the flowlet
		// entries.
		if ((*itr).networkMask.IsMatch(dst, (*itr).network)) {
			letFlowRouteEntries.push_back(*itr);
		}
	}
	return letFlowRouteEntries;
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