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
class Ipv4LetFlowRouting : public Ipv4RoutingProtocol {
public:
	Ipv4LetFlowRouting(Ptr<Ipv4GlobalRouting> globalRouting);
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

    /**
     * \brief sets the timeout period for flowlets.
     * 
     * Whenever the time between two consecutive packets exceeds the flowlet
     * timeout, a new flowlet is created.
     * 
     * \param timeout the length of time for the flowlet timeout.
     */
	void SetFlowletTimeout(Time timeout);

private:
	/**
	 * \brief chooses a route to the destination at random.
	 * 
	 * This function is called when either we have a packet with no flow ID
	 * (ie. an error or control packet), or if a flowlet timed out and we
	 * need to choose a new route for the flow.
	 * 
	 * \param routeEntries the set of routes to the destination.
	 * \param selectedPort the port which will be selected to send the packet.
	 * \param dstAddr the address of the destination for the flow.
	 * 
	 * \returns Ptr<IpvRoute> chosen to the destination.
	 */
    Ptr<Ipv4Route> ChooseRandomRoute(
    	std::vector<Ipv4RoutingTableEntry*>& routeEntries,
    	uint32_t& selectedPort, Ipv4Address dstAddr);

    /**
     * \brief constructs a route to the destination.
     * 
     * \param port the port from which the route will send packets.
     * \param dstAddr the destination address for the route.
     * 
     * \returns Ptr<Ipv4Route> from the chosen port to the destination.
     */
    Ptr<Ipv4Route> ConstructIpv4Route(uint32_t port, Ipv4Address dstAddr);


    /// A uniform random number generator for randomly routing packets among ECMP
    Ptr<UniformRandomVariable> m_rand;

	// Flowlet timeout (minimum inter-packet gap denoting different flowlets).
	Time m_flowletTimeout;

	// Ipv4 address associated with this router.
	Ptr<Ipv4> m_ipv4;

	// Flowlet table.
	std::map<uint32_t, LetFlowFlowlet> m_flowletTable;

	// A pointer to an Ipv4GlobalRouting object. LetFlow only changes routes
	// to balance loads but we leverage the existing global routing
	// capabilities to pre-install routes and maintain the routing table.
	Ptr<Ipv4GlobalRouting> m_globalRouting;
};

}  // namespace ns3

#endif  // IPV4_LETFLOW_ROUTING_H