#ifndef IPV4_DRB_ROUTING_H
#define IPV4_DRB_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-address.h"

#include <map>
#include <set>

namespace ns3
{

enum DrbRoutingMode {
    PER_DEST = 0,
    PER_FLOW = 1
};

/**
 * \ingroup drb-routing
 * \brief DRB Load Balancing Protocol
 * 
 * Implements the DRB per-packet load balancing scheme from the paper
 * "Per-packet Load-balanced, Low-Latency Routing for Clos-based Data Center
 * Networks"
 */
class Ipv4DrbRouting : public Ipv4RoutingProtocol {
public:
    Ipv4DrbRouting();
    ~Ipv4DrbRouting();

    bool AddPath(uint32_t path);
    bool AddPath(uint32_t weight, uint32_t path);

    bool AddWeightedPathToDsts(
        uint32_t weight, uint32_t path,
        const std::set<Ipv4Address>& dstIPs = std::set<Ipv4Address>());

    bool AddWeightedPathToDst(
        Ipv4Address dstAddr, uint32_t weight, uint32_t path);

    // Inherited from Ipv4RoutingProtocol
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
    virtual void NotifyAddAddress(
        uint32_t interface, Ipv4InterfaceAddress address);
    virtual void NotifyRemoveAddress(
        uint32_t interface, Ipv4InterfaceAddress address);
    virtual void SetIpv4(Ptr<Ipv4> ipv4);
    virtual void PrintRoutingTable(
        Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

    virtual void DoDispose();

private:
    // Default paths to use when a path to the destination is not known.
    // Corresponds to just randomly selecting a path in hopes that a future
    // hop will know the path to the destination.
    std::vector<uint32_t> m_allPaths;
    // Destination specific paths. For each destination, a list of weighted
    // paths is maintained to that destination.
    std::map<Ipv4Address, std::vector<uint32_t>> m_dstPaths;
    // Maintains a map of flow IDs to paths. So if a flow already has an
    // associated path we can just reuse that path.
    std::map<uint32_t, uint32_t> m_flowPathMap;
    enum DrbRoutingMode m_mode;

    Ptr<Ipv4> m_ipv4;
};


}  // namespace ns3

#endif  // IPV4_DRB_ROUTING_H
