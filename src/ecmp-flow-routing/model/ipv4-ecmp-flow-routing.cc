#include "ipv4-ecmp-flow-routing.h"

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

#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4EcmpFlowRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4EcmpFlowRouting);

TypeId Ipv4EcmpFlowRouting::GetTypeId() {
	static TypeId tid = TypeId("ns3::Ipv4EcmpFlowRouting")
	    .SetParent<Object>()
	    .SetGroupName("Internet");
	return tid;
}

Ipv4EcmpFlowRouting::Ipv4EcmpFlowRouting(Ptr<Ipv4GlobalRouting> globalRouting)
    : m_ipv4(nullptr), m_globalRouting(globalRouting)
{
	NS_LOG_FUNCTION(this);
}

Ipv4EcmpFlowRouting::~Ipv4EcmpFlowRouting() {
	NS_LOG_FUNCTION(this);
}

Ptr<Ipv4Route> Ipv4EcmpFlowRouting::PickEcmpRoute(Ipv4Address dst,
	                                              const Ipv4Header& header,
	                                              uint32_t flowId,
	                                              Ptr<NetDevice> oif) {
	NS_LOG_FUNCTION(this << dst << header << flowId << oif);
	NS_LOG_LOGIC("Looking for route to destination " << dst);
	Ptr<Ipv4Route> rtentry = nullptr;
	std::vector<Ipv4RoutingTableEntry*> allRoutes = m_globalRouting->GetRoutesToDst(dst, oif);

	if (!allRoutes.empty()) {
		uint32_t selectIndex;
		// If the flow ID is 0, it may be the socket setup endpoint request.
		// So we simply return the first available route to indicate that
		// the address is not local.
		if (flowId != 0) {
			std::stringstream hash_string;
			hash_string << flowId;
			hash_string << header.GetTtl();
			uint32_t hashedVal = Hash32(hash_string.str());
			selectIndex = hashedVal % allRoutes.size();
			NS_LOG_LOGIC("Per flow ECMP is enabled, selected index: "
				<< selectIndex << " for flow: " << flowId);
		} else {
			selectIndex = 0;
			NS_LOG_LOGIC("Flow ID was 0, picking default route");
		}

		Ipv4RoutingTableEntry* route = allRoutes.at(selectIndex);
		rtentry = Create<Ipv4Route>();
		rtentry->SetDestination(route->GetDest());
		rtentry->SetSource(m_ipv4->GetAddress(route->GetInterface(), 0).GetLocal());
		rtentry->SetGateway(route->GetGateway());
		uint32_t interfaceIdx = route->GetInterface();
		rtentry->SetOutputDevice(m_ipv4->GetNetDevice(interfaceIdx));
		return rtentry;
	}
	return nullptr;
}

Ptr<Ipv4Route>
Ipv4EcmpFlowRouting::RouteOutput(Ptr<Packet> p,
	                             const Ipv4Header& header,
	                             Ptr<NetDevice> oif,
	                             Socket::SocketErrno& sockerr) {
	NS_LOG_FUNCTION(this << p << &header << oif << &sockerr);
	// Recover the flow ID.
	uint32_t flowId = ExtractFlowId(p);

	// ECMP flow-level routing does not support multicast.
	if (header.GetDestination().IsMulticast()) {
		NS_LOG_LOGIC("Multicast destination - returning false");
		return nullptr;
	}
	// Check if this is a unicast packet that we have a route for.
	NS_LOG_LOGIC("Unicast destination - looking for route");
	Ptr<Ipv4Route> rtentry = PickEcmpRoute(
		header.GetDestination(), header, flowId, oif);
	if (rtentry) {
		sockerr = Socket::ERROR_NOTERROR;
	} else {
		sockerr = Socket::ERROR_NOROUTETOHOST;
	}
	return rtentry;
}

bool Ipv4EcmpFlowRouting::RouteInput(Ptr<const Packet> p,
	                                 const Ipv4Header& header,
	                                 Ptr<const NetDevice> idev,
	                                 UnicastForwardCallback ucb,
	                                 MulticastForwardCallback mcb,
	                                 LocalDeliverCallback lcb,
	                                 ErrorCallback ecb) {
	NS_LOG_FUNCTION(this << p << header << header.GetSource()
		<< header.GetDestination() << idev << &lcb << &ecb);
	// Check if input device supports IP.
	NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
	uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);

	if (m_ipv4->IsDestinationAddress(header.GetDestination(), iif)) {
		if (!lcb.IsNull()) {
			NS_LOG_LOGIC("Local delivery to " << header.GetDestination());
			lcb(p, header, iif);
			return true;
		} else {
			// The local delivery callback is null. This may be a multicast
			// or broadcast packet, so return false so that another
			// multicast routing protocol can handle it.
			return false;
		}
	}

	// Check if input device supports IP forwarding.
	if (m_ipv4->IsForwarding(iif) == false) {
		NS_LOG_LOGIC("Forwarding disabled for this interface");
		ecb(p, header, Socket::ERROR_NOROUTETOHOST);
		return true;
	}
	// Next, try to find a route.
	NS_LOG_LOGIC("Unicast destination - looking up route");
	uint32_t flowId = ExtractFlowId(ConstCast<Packet>(p));
	Ptr<Ipv4Route> rtentry = PickEcmpRoute(
		header.GetDestination(), header, flowId);
	if (rtentry) {
		NS_LOG_LOGIC("Found unicast destination - calling unicast callback");
		ucb(rtentry, p, header);
		return true;
	}
	NS_LOG_LOGIC("Did not find unicast destination - returning false");
	return false;
}

void Ipv4EcmpFlowRouting::NotifyInterfaceUp(uint32_t interface) {
	m_globalRouting->NotifyInterfaceUp(interface);
}

void Ipv4EcmpFlowRouting::NotifyInterfaceDown(uint32_t interface) {
	m_globalRouting->NotifyInterfaceDown(interface);
}

void Ipv4EcmpFlowRouting::NotifyAddAddress(uint32_t interface,
	                                       Ipv4InterfaceAddress address) {
	m_globalRouting->NotifyAddAddress(interface, address);
}

void Ipv4EcmpFlowRouting::NotifyRemoveAddress(uint32_t interface,
	                                          Ipv4InterfaceAddress address) {
	m_globalRouting->NotifyRemoveAddress(interface, address);
}

void Ipv4EcmpFlowRouting::SetIpv4(Ptr<Ipv4> ipv4) {
	NS_LOG_LOGIC(this << " Setting up IPv4: " << ipv4);
	NS_ASSERT(m_ipv4 == nullptr && ipv4 != nullptr);
	m_ipv4 = ipv4;
}

void Ipv4EcmpFlowRouting::DoDispose() {
	NS_LOG_FUNCTION(this);
	m_ipv4 = nullptr;
	m_globalRouting->DoDispose();
	m_globalRouting = nullptr;
}

uint32_t Ipv4EcmpFlowRouting::GetNRoutes() const {
	NS_LOG_FUNCTION(this);
	return m_globalRouting->GetNRoutes();
}

Ipv4RoutingTableEntry* Ipv4EcmpFlowRouting::GetRoute(uint32_t i) const {
	NS_LOG_FUNCTION(this << " " << i);
	return m_globalRouting->GetRoute(i);
}

void Ipv4EcmpFlowRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
	                                        Time::Unit unit) const {
	m_globalRouting->PrintRoutingTable(stream, unit);
}

uint32_t Ipv4EcmpFlowRouting::ExtractFlowId(Ptr<Packet> p) {
	uint32_t flowId = 0;
	if (p != nullptr) {
		FlowIdTag flowIdTag;
		bool found = p->PeekPacketTag(flowIdTag);
		if (found) {
			flowId = flowIdTag.GetFlowId();
		}
	}
	return flowId;
}

}  // namespace ns3
