#ifndef IPV4_DRILL_ROUTING_H
#define IPV4_DRILL_ROUTING_H

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

/**
 * \defgroup drill-routing Load balancing with DRILL.
 * 
 * This section documents the API of the DRILL load balancing module. This
 * module uses DRILL to balance flows by making routing choices on a per
 * packet basis. It does so by keeping track of the least congested links.
 * Then at each decision point it randomly samples two links and takes the
 * previously least congested link. Among the three, whichever has the
 * smallest queue is chosen as the gateway to send the packet.
 */

namespace ns3
{

/**
 * \ingroup drill-routing
 * 
 * \brief DRILL routing protocol for IPv4 stacks.
 */
class Ipv4DrillRouting {
public:
    Ipv4DrillRouting(Ptr<Ipv4GlobalRouting> globalRouting);
    ~Ipv4DrillRouting();

    static TypeId GetTypeId();

    virtual void DoDispose();
private:
    /**
     * \brief Lookup in the DRILL forwarding table for the destination.
     * 
     * \param dst The destination address.
     * \param oif Output interface if any (put nullptr otherwise).
     * 
     * \returns Vector of Ipv4RoutingTableEntry objects for routes to the
     * destination.
     */
    std::vector<Ipv4RoutingTableEntry*> LookupDrillRoutes(
        Ipv4Address dst, Ptr<NetDevice> oif = nullptr);

    /**
     * \brief Calculates the queue length of a given interface.
     * 
     * This is used by DRILL to decide how favourable a certain port is. Ports
     * with smaller queues are more favourable for routing.
     * 
     * \param interface The interface for which the queue length is computed.
     * 
     * \returns an int representing the number of bits in the queue.
     */
    uint32_t CalculateQueueLength(uint32_t interface);

    /**
     * \brief constructs a route to the destination.
     * 
     * \param port the port from which the route will send packets.
     * \param dstAddr the destination address for the route.
     * 
     * \returns Ptr<Ipv4Route> from the chosen port to the destination.
     */
    Ptr<Ipv4Route> ConstructIpv4Route(uint32_t port, Ipv4Address dstAddr);

    // The d value used in DRILL routing to decide how many ports to
    // sample from.
    uint32_t m_d;

    // A map storing the port with the lowest queue for each destination based
    // on the previous iteration of DRILL (last time it sent a packet).
    std::map<Ipv4Address, uint32_t> m_previousBestQueueMap;

    // Ipv4 address associated with this router.
    Ptr<Ipv4> m_ipv4;

    // A pointer to an Ipv4GlobalRouting object. DRILL only changes routes
    // to balance loads but we leverage the existing global routing
    // capabilities to pre-install routes and maintain the routing table.
    Ptr<Ipv4GlobalRouting> m_globalRouting;
};

}  // namespace ns3

#endif  // IPV4_DRILL_ROUTING_H
