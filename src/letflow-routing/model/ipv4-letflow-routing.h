/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_LETFLOW_ROUTING_H
#define IPV4_LETFLOW_ROUTING_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

namespace ns3 {

struct LetFlowFlowlet {
	uint32_t port;
	Time activeTime;
};

// This LetFlow routing class is implemented in each switch.
class Ipv4LetFlowRouting : public Ipv4GlobalRouting {
public:
	Ipv4LetFlowRouting();
	~Ipv4LetFlowRouting();

	static TypeId GetTypeId();

	// Inherited from Ipv4RoutingProtocol.
	Ptr<Ipv4Route> RouteOutput(
		Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif,
		Socket::SocketErrno& sockerr) override;
	bool RouteInput(
		Ptr<const Packet> p, const Ipv4Header& header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) override;
	void NotifyInterfaceUp(uint32_t interface) override;
	void NotifyInterfaceDown(uint32_t interface) override;
	void NotifyAddAddress(uint32_t interface,
		                  Ipv4InterfaceAddress address) override;
	void NotifyRemoveAddress(uint32_t interface,
		                     Ipv4InterfaceAddress address) override;
	void SetIpv4(Ptr<Ipv4> ipv4) override;
	void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
		                   Time::Unit unit = Time::S) const override;

    /**
     * \brief Add a host route to the routing table.
     * 
     * \param dst The Ipv4Address representing the destination for this route.
     * \param nextHop The Ipv4Address of the next hop in the route.
     * \param interface The network interface index used to send packets to
     *        the destination (a.k.a. the port).
     */
	void AddHostRouteTo(
		Ipv4Address dst, Ipv4Address nextHop, uint32_t interface);

    /**
     * \brief Add a host route to the routing table.
     * 
     * \param dst The Ipv4Address representing the destination for this route.
     * \param interface The network interface index used to send packets to
     *        the destination (a.k.a. the port).
     */
	void AddHostRouteTo(Ipv4Address dst, uint32_t interface);

	/**
	 * \brief Add a network route to the routing table.
	 * 
	 * \param network The Ipv4Address of the network for this route.
	 * \param networkMask The Ipv4Mask to extract the network.
	 * \param nextHop The next hop in the route to the destination network.
	 * \param interface The network interface index used to send packets to
	 *        the destination (a.k.a. the port).
	 */
	void AddNetworkRouteTo(Ipv4Address network,
		                   Ipv4Mask networkMask,
		                   Ipv4Address nextHop,
		                   uint32_t interface);

	/**
	 * \brief Add a network route to the routing table.
	 * 
	 * \param network The Ipv4Address of the network for this route.
	 * \param networkMask The Ipv4Mask to extract the network.
	 * \param interface The network interface index used to send packets to
	 *        the destination (a.k.a. the port).
	 */
	void AddNetworkRouteTo(Ipv4Address network,
		                   Ipv4Mask networkMask,
		                   uint32_t interface);

	/**
	 * \brief Add an external route to the routing table.
	 * 
	 * \param network The Ipv4Address network for this route.
	 * \param networkMask The Ipv4Mask to extract the network.
	 * \param nextHop The next hop Ipv4Address.
	 * \param interface The network interface index used to send packet to
	 *        the destination (a.k.a. the port).
	 */
	void AddASExternalRouteTo(Ipv4Address network,
		                      Ipv4Mask networkMask,
		                      Ipv4Address nextHop,
		                      uint32_t interface);

	/**
	 * \brief Get the number of individual unicast routes that have been
	 * added to the routing table.
	 */
	uint32_t GetNRoutes() const;

	/**
	 * \brief Get a route from the unicast routing table.
	 * 
	 * \param i The index (into the routing table) of the route to retrieve.
	 * 
	 * \return If the route is set, a pointer to that Ipv4RoutingTableEntry
	 * is returned, otherwise, nullptr is returned.
	 */
	Ipv4RoutingTableEntry* GetRoute(uint32_t i) const;

	virtual void DoDispose(void) override;

	/**
	 * \brief Lookup in the LetFlow forwarding table for the destination.
	 * 
	 * \param dst The destination address.
	 * \param oif output interface if any (put nullptr otherwise).
	 * 
	 * \return vector of Ipv4RoutingTableEntry objects for routes to the
	 * destination
	 */
	std::vector<Ipv4RoutingTableEntry*> LookupLetFlowRoutes(
		Ipv4Address dst, Ptr<NetDevice> oif = nullptr);
	
	Ptr<Ipv4Route> ConstructIpv4Route(uint32_t port, Ipv4Address dstAddress);

	void SetFlowletTimeout(Time timeout);

private:
	// Flowlet timeout (minimum inter-packet gap denoting different flowlets).
	Time m_flowletTimeout;

	// Ipv4 address associated with this router.
	Ptr<Ipv4> m_ipv4;

	// Flowlet table.
	std::map<uint32_t, LetFlowFlowlet> m_flowletTable;

	// Routes to hosts.
	std::vector<Ipv4RoutingTableEntry*> m_hostRoutes;
	// Routes to networks.
	std::vector<Ipv4RoutingTableEntry*> m_networkRoutes;
};

}  // namespace ns3

#endif  // IPV4_LETFLOW_ROUTING_H