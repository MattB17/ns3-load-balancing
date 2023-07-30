/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_LETFLOW_ROUTING_H
#define IPV4_LETFLOW_ROUTING_H

#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

namespace ns3 {

struct LetFlowFlowlet {
	uint32_t port;
	Time activeTime;
};

struct LetFlowRouteEntry {
	Ipv4Address network;
	Ipv4Mask networkMask;
	uint32_t port;
};

// This LetFlow routing class is implemented in each switch.
class Ipv4LetFlowRouting : public Ipv4GlobalRouting {
public:
	Ipv4LetFlowRouting();
	~Ipv4LetFlowRouting();

	static TypeId GetTypeId(void);

	void AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port);

	// Inherited from Ipv4RoutingProtocol.
	virtual Ptr<Ipv4Route> RouteOutput(
		Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif,
		Socket::SocketErrno& sockerr);
	virtual bool RouteInput(
		Ptr<const Packet> p, const Ipv4Header& header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb);
	virtual void NotifyInterfaceUp(uint32_t interface);
	virtual void NotifyInterfaceDown(uint32_t interface);
	virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
	virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
	virtual void SetIpv4(Ptr<Ipv4> ipv4);
	virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
		                           Time::Unit unit = Time::S) const;

	virtual void DoDispose(void);

	std::vector<LetFlowRouteEntry> LookupLetFlowRouteEntries(Ipv4Address dst);
	Ptr<Ipv4Route> ConstructIpv4Route(uint32_t port, Ipv4Address dstAddress);

	void SetFlowletTimeout(Time timeout);

private:
	// Flowlet timeout (minimum inter-packet gap denoting different flowlets).
	Time m_flowletTimeout;

	// Ipv4 address associated with this router.
	Ptr<Ipv4> m_ipv4;

	// Flowlet table.
	std::map<uint32_t, LetFlowFlowlet> m_flowletTable;

	std::vector<LetFlowRouteEntry> m_routeEntryList;

	// Routes to hosts.
	std::vector<Ipv4RoutingTableEntry*> m_hostRoutes;
	// Routes to networks.
	std::vector<Ipv4RoutingTableEntry*> m_networkRoutes;
};

}  // namespace ns3

#endif  // IPV4_LETFLOW_ROUTING_H