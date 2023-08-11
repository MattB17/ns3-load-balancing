#ifndef ECMP_FLOW_ROUTING_H
#define ECMP_FLOW_ROUTING_H

#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

/**
 * \defgroup ecmp-flow-routing ECMP flow-level routing
 * 
 * This section documents the API of the flow-level ECMP module. This module
 * uses ECMP on flows, so each flow is randomly assigned to a path and then
 * all packets of the flow traverse the same path. This is in contrast to
 * pure random ECMP in which each packet is sent down a random path,
 * regardless of the path that other packets in the same flow have taken.
 * In times of congestion, this can lead to severe reordering.
 */

namespace ns3
{

/**
 * \ingroup ecmp-flow-routing
 * 
 * \brief ECMP flow level routing protocol for IPv4 stacks.
 */

class Ipv4EcmpFlowRouting : public Ipv4RoutingProtocol {
public:
    Ipv4EcmpFlowRouting(Ptr<Ipv4GlobalRouting> globalRouting);
    ~Ipv4EcmpFlowRouting();

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
private:
    /**
     * \brief Choose a route according to ECMP flow-level routing.
     * 
     * The flow ID is used to ensure that every packet for a flow uses
     * the same route, but the route for each flow is chosen at random.
     * 
     * \param dst The destination address.
     * \param header The packet header for the packet being routed.
     * \param flowId The ID of the flow to which the packet belongs.
     * \param oif The output interface if any (put nullptr otherwise).
     * 
     * \return Ipv4Route to route the packet to the destination.
     */
    Ptr<Ipv4Route> PickEcmpRoute(
        Ipv4Address dst, const Ipv4Header& header,
        uint32_t flowId, Ptr<NetDevice> oif = nullptr);

    /**
     * \brief Extracts the flow ID for a packet.
     * 
     * The flow ID is used by ECMP to make sure that all packets associated
     * with a flow traverse the same path.
     * 
     * \param p The packet from which the flow ID is extracted.
     * 
     * \return int The flow ID identifying the packet for the flow.
     */
    uint32_t ExtractFlowId(Ptr<Packet> p);

    // Ipv4 address associated with this router.
    Ptr<Ipv4> m_ipv4;

    // A pointer to an Ipv4GlobalRouting object. ECMP flow-level only assigns
    // routes to balance loads but we leverage the existing global routing
    // capabilities to pre-install routes and maintain the routing table.
    Ptr<Ipv4GlobalRouting> m_globalRouting;
};

}  // namespace ns3

#endif  // ECMP_FLOW_ROUTING_H
