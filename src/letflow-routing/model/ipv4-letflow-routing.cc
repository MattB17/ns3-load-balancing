/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ipv4-letflow-routing.h"

#include "ns3/channel.h"
#include "ns3/flow-id-tag.h"
#include "ns3/ipv4-route.h"
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
    m_rand = CreateObject<UniformRandomVariable>();
}

Ipv4LetFlowRouting::~Ipv4LetFlowRouting() {
	NS_LOG_FUNCTION(this);
}

Ptr<Ipv4Route> Ipv4LetFlowRouting::RouteOutput(Ptr<Packet> packet,
	                                           const Ipv4Header& header,
	                                           Ptr<NetDevice> oif,
	                                           Socket::SocketErrno& sockerr) {
	NS_LOG_FUNCTION(this << packet << &header << oif << &sockerr);
	NS_LOG_LOGIC(this << "Route Output for " << packet);
	// Delegate to Global Routing. LetFlow is only implemented in the network
	// and does not extend to the hosts.
	Ptr<Ipv4Route> rtentry = m_globalRouting->RouteOutput(
		packet, header, oif, sockerr);
	return rtentry;
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
	uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);
	NS_ASSERT(iif >= 0);

	// Check if it this is the intended destination. If so then we call the
	// local callback (lcb) to push it up the stack.
	if (m_ipv4->IsDestinationAddress(header.GetDestination(), iif)) {
		  if (!lcb.IsNull()) {
			    NS_LOG_LOGIC("Local delivery to " << header.GetDestination());
			    lcb(p, header, iif);
			    return true;
		  } else {
			    // The local delivery callback is null.  This may be a multicast
          // or broadcast packet, so return false so that another
          // multicast routing protocol can handle it.
			    return false;
		  }
	}

	// LetFlow routing only supports unicast.
	if (header.GetDestination().IsMulticast() ||
		  header.GetDestination().IsBroadcast()) {
		  NS_LOG_ERROR(this <<  " LetFlow routing only supports unicast");
		  ecb(p, header, Socket::ERROR_NOROUTETOHOST);
		  return false;
	}

	// Check if the input device supports IP forwarding.
	if (m_ipv4->IsForwarding(iif) == false) {
		  NS_LOG_ERROR(this << " Forwarding is disabled for this interface");
		  ecb(p, header, Socket::ERROR_NOROUTETOHOST);
		  return false;
	}

	// Packet arrival time.
	Time now = Simulator::Now();
	std::vector<Ipv4RoutingTableEntry*> routeEntries =
	    LookupLetFlowRoutes(header.GetDestination());

	// Extract the flow ID.
	uint32_t flowId = 0;
	FlowIdTag flowIdTag;
	// Flow ID for the packet.
	bool flowIdFound = p->PeekPacketTag(flowIdTag);
	if (!flowIdFound) {
		  NS_LOG_LOGIC(this << " LetFlow routing cannot extract the flow ID, "
		  	  << "picking path at random");
		  return m_globalRouting->RouteInput(p, header, idev, ucb, mcb, lcb, ecb);
	}
	// If the flow ID was found, extract it.
	flowId = flowIdTag.GetFlowId();

  // We first examine the flowlet table to see if it is part of an active
  // flowlet.
	auto flowletItr = m_flowletTable.find(flowId);
	// If the flowlet table entry is valid, return the route.
	if (flowletItr != m_flowletTable.end()) {
	    // Get the flowlet from the iterator.
	    LetFlowFlowlet flowlet = flowletItr->second;
	    // If the interpacket gap is less than the flowlet timeout.
	    if (now - flowlet.activeTime <= m_flowletTimeout) {
	    	  NS_LOG_LOGIC(this << " Found active flowlet for " << flowId);
	    	  // Update the flowlet last active time and get the route.
	    	  flowlet.activeTime = now;

	    	  // Update the flowlet table.
	    	  m_flowletTable[flowId] = flowlet;

	    	  ucb(flowlet.routeEntry, p, header);

	    	  return true;
	    } else {
	    	NS_LOG_LOGIC("Flowlet for " << flowId << " timed out. Packet gap: "
	    		  << now - flowlet.activeTime << " timeout period: "
	    		  << m_flowletTimeout);
	    }
	}

	// Otherwise, the flowlet either timed out or we don't have a flowlet
	// entry.

	// Return error if there are no routing entries to the destination.
	if (routeEntries.empty()) {
	    NS_LOG_ERROR(this << " LetFlow routing cannot find routing entry");
	    ecb(p, header, Socket::ERROR_NOROUTETOHOST);
	    return false;
	}

	// Otherwise, there is a route so we need to decide through which port we
	// should forward the packet.
	uint32_t selectedIdx = m_rand->GetInteger(0, routeEntries.size() - 1);
	NS_LOG_LOGIC(this << " Creating new flowlet for " << flowId
		  << " selected route number: " << selectedIdx);
	Ipv4RoutingTableEntry* route = routeEntries.at(selectedIdx);
	// Construct the route.
	Ptr<Ipv4Route> routeEntry = Ipv4LetFlowRouting::ConstructIpv4Route(route);

	// Construct the flowlet and route.
	LetFlowFlowlet flowlet;
	flowlet.routeEntry = routeEntry;
	flowlet.activeTime = now;
	// Update the flowlet table.
	m_flowletTable[flowId] = flowlet;

	ucb(routeEntry, p, header);
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
	NS_ASSERT(m_ipv4 == nullptr && ipv4 != nullptr);
	m_ipv4 = ipv4;
	m_globalRouting->SetIpv4(ipv4);
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

Ptr<Ipv4Route>
Ipv4LetFlowRouting::ConstructIpv4Route(Ipv4RoutingTableEntry* route) {
	if (route) {
		  Ipv4Address srcAddr = m_ipv4->GetAddress(
		  	  route->GetInterface(), 0).GetLocal();
		  NS_LOG_FUNCTION(this << srcAddr);
	    Ptr<Ipv4Route> routeEntry = Create<Ipv4Route>();
	    routeEntry->SetDestination(route->GetDest());
	    routeEntry->SetSource(srcAddr);
	    routeEntry->SetGateway(route->GetGateway());
	    uint32_t interfaceIdx = route->GetInterface();
	    routeEntry->SetOutputDevice(m_ipv4->GetNetDevice(interfaceIdx));
	    return routeEntry;
	}
	return nullptr;
}

void Ipv4LetFlowRouting::SetFlowletTimeout(Time timeout) {
	m_flowletTimeout = timeout;
}

}  // namespace ns3