/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_LETFLOW_ROUTING_H
#define IPV4_LETFLOW_ROUTING_H

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
class Ipv4LetFlowRouting {
public:
	Ipv4LetFlowRouting();
	~Ipv4LetFlowRouting();

	static TypeId GetTypeId(void);

	void SetFlowletTimeout(Time timeout);

private:
	// Flowlet timeout (minimum inter-packet gap denoting different flowlets).
	Time m_flowletTimeout;

	// Ipv4 address associated with this router.
	Ptr<Ipv4> m_ipv4;

	// Flowlet table.
	std::map<uint32_t, LetFlowFlowlet> m_flowletTable;

	std::vector<LetFlowRouteEntry> m_routeEntryList;
};

}  // namespace ns3

#endif  // IPV4_LETFLOW_ROUTING_H