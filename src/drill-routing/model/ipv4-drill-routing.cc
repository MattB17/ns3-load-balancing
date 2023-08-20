#include "ipv4-drill-routing.h"

#include "ns3/channel.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/simulator.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <limits>
#include <random>
#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DrillRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4DrillRouting);

Ipv4DrillRouting::Ipv4DrillRouting(Ptr<Ipv4GlobalRouting> globalRouting)
    : m_d(2), m_ipv4(nullptr), m_globalRouting(globalRouting) {
    NS_LOG_FUNCTION(this);
}

Ipv4DrillRouting::~Ipv4DrillRouting() {
	NS_LOG_FUNCTION(this);
}

TypeId Ipv4DrillRouting::GetTypeId() {
	static TypeId tid = TypeId("ns3::Ipv4DrillRouting")
	    .SetParent<Object>()
	    .SetGroupName("Internet")
	    .AddAttribute("d", "Sample d random output queues",
	    	          UintegerValue(2),
	    	          MakeUintegerAccessor(&Ipv4DrillRouting::m_d),
	    	          MakeUintegerChecker<uint32_t>());
	return tid;
}

Ptr<Ipv4Route> Ipv4DrillRouting::RouteOutput(Ptr<Packet> packet,
	                                         const Ipv4Header& header,
	                                         Ptr<NetDevice> oif,
	                                         Socket::SocketErrno& sockerr) {
    NS_LOG_FUNCTION(this << packet << &header << oif << &sockerr);
    // Delegate to Global Routing. DRILL is only implemented in the network
    // and does not extend to the hosts.
    Ptr<Ipv4Route> rtentry = m_globalRouting->RouteOutput(
    	packet, header, oif, sockerr);
    return rtentry;
}

// Receive an input packet on input device `idev`.
bool Ipv4DrillRouting::RouteInput(Ptr<const Packet> p,
                                  const Ipv4Header& header,
                                  Ptr<const NetDevice> idev,
                                  UnicastForwardCallback ucb,
                                  MulticastForwardCallback mcb,
                                  LocalDeliverCallback lcb,
                                  ErrorCallback ecb) {
    NS_LOG_LOGIC(this << " Route Input: " << p << "IP header: " << header);
    uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);
    NS_ASSERT(iif >= 0);
    
    // Check if it is the intended destination. If so, then we call the local
    // callback (lcb) to push it up the stack.
    if (m_ipv4->IsDestinationAddress(header.GetDestination(), iif)) {
        if (!lcb.IsNull()) {
            NS_LOG_LOGIC("Local delivery to " << header.GetDestination());
            lcb(p, header, iif);
            return true;
        } else {
            // The local delivery callback is null. This may be a multicast
            // or broadcast packet, so return false so that another multicast
            // routing protocol can handle it.
            return false;
        }
    }
    
    // DRILL only supports unicast.
    if (header.GetDestination().IsMulticast() ||
        header.GetDestination().IsBroadcast()) {
        NS_LOG_ERROR(this << " DRILL routing only supports unicast");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
    }
    
    // Check if the input device supports IP forwarding.
    if (m_ipv4->IsForwarding(iif) == false) {
        NS_LOG_ERROR(this << " Forwarding is disabled for this interface");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
    }
    
    std::vector<Ipv4RoutingTableEntry*> allRoutes = LookupDrillRoutes(
        header.GetDestination());
    if (allRoutes.empty()) {
        NS_LOG_ERROR(this << "DRILL routing cannot find route to "
                     << header.GetDestination());
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
    }
    
    // Used to keep track of the least loaded interface.
    uint32_t leastLoadedInterface = 0;
    uint32_t leastLoad = std::numeric_limits<uint32_t>::max();
    
    // Shuffle so that we can randomly sample `m_d` ports.
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allRoutes.begin(), allRoutes.end(), g);
    
    // We start by checking the previously least loaded port.
    auto itr = m_previousBestQueueMap.find(header.GetDestination());
    if (itr != m_previousBestQueueMap.end()) {
        leastLoadedInterface = itr->second;
        leastLoad = CalculateQueueLength(itr->second);
    }
    
    // If we have at least `m_d` routes, then we sample `m_d`. Otherwise, we
    // sample all.
    uint32_t sampleNum = m_d < allRoutes.size() ? m_d : allRoutes.size();
    
    // We've already randomly shuffled so we can just go through the first
    // `sample_num` entries in the shuffled array.
    for (uint32_t samplePort = 0; samplePort < sampleNum; samplePort++) {
        uint32_t sampleLoad = CalculateQueueLength(
            allRoutes[samplePort]->GetInterface());
        if (sampleLoad < leastLoad) {
            leastLoad = sampleLoad;
            leastLoadedInterface = allRoutes[samplePort]->GetInterface();
        }
    }
    
    NS_LOG_LOGIC(this << " Drill routing choses interface: "
                 << leastLoadedInterface << ", since its load is: "
                 << leastLoad);
    
    m_previousBestQueueMap[header.GetDestination()] = leastLoadedInterface;
    Ptr<Ipv4Route> route = ConstructIpv4Route(leastLoadedInterface,
                                              header.GetDestination());
    ucb(route, p, header);
    return true;
}

void Ipv4DrillRouting::NotifyInterfaceUp(uint32_t interface) {
    m_globalRouting->NotifyInterfaceUp(interface);
}

void Ipv4DrillRouting::NotifyInterfaceDown(uint32_t interface) {
    m_globalRouting->NotifyInterfaceDown(interface);
}

void Ipv4DrillRouting::NotifyAddAddress(uint32_t interface,
                                        Ipv4InterfaceAddress address) {
    m_globalRouting->NotifyAddAddress(interface, address);
}

void Ipv4DrillRouting::NotifyRemoveAddress(uint32_t interface,
                                           Ipv4InterfaceAddress address) {
    m_globalRouting->NotifyRemoveAddress(interface, address);
}

void Ipv4DrillRouting::SetIpv4(Ptr<Ipv4> ipv4) {
    NS_LOG_LOGIC(this << " Setting up IPv4: " << ipv4);
    NS_ASSERT(m_ipv4 == nullptr && ipv4 != nullptr);
    m_ipv4 = ipv4;
    m_globalRouting->SetIpv4(ipv4);
}

void Ipv4DrillRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                         Time::Unit unit) const {
    m_globalRouting->PrintRoutingTable(stream, unit);
}

uint32_t Ipv4DrillRouting::GetNRoutes() const {
    NS_LOG_FUNCTION(this);
    return m_globalRouting->GetNRoutes();
}

Ipv4RoutingTableEntry* Ipv4DrillRouting::GetRoute(uint32_t i) const {
    NS_LOG_FUNCTION(this << " " << i);
    return m_globalRouting->GetRoute(i);
}

void Ipv4DrillRouting::DoDispose() {
	NS_LOG_FUNCTION(this);
	m_ipv4 = nullptr;
	m_globalRouting->DoDispose();
	m_globalRouting = nullptr;
}

std::vector<Ipv4RoutingTableEntry*>
Ipv4DrillRouting::LookupDrillRoutes(Ipv4Address dst, Ptr<NetDevice> oif) {
	NS_LOG_FUNCTION(this << dst << oif);
	return m_globalRouting->GetRoutesToDst(dst, oif);
}

uint32_t Ipv4DrillRouting::CalculateQueueLength(uint32_t interface) {
	Ptr<Ipv4L3Protocol> ipv4L3Protocol = DynamicCast<Ipv4L3Protocol>(m_ipv4);
	if (!ipv4L3Protocol) {
		NS_LOG_ERROR(
			this << "Drill routing only works at the ipv4L3Protocol layer");
		return 0;
	}

	uint32_t queueLength = 0;
	const Ptr<NetDevice> netDevice = this->m_ipv4->GetNetDevice(interface);

	if (netDevice->IsPointToPoint()) {
		Ptr<PointToPointNetDevice> p2pNetDevice =
		    DynamicCast<PointToPointNetDevice>(netDevice);
		if (p2pNetDevice) {
		    queueLength += p2pNetDevice->GetQueue()->GetNBytes();
		}
	}

	Ptr<TrafficControlLayer> tc =
	    ipv4L3Protocol->GetObject<TrafficControlLayer>();
	if (!tc) {
		return queueLength;
	}
	Ptr<QueueDisc> queueDisc = tc->GetRootQueueDiscOnDevice(netDevice);
	if (queueDisc) {
		queueLength += queueDisc->GetNBytes();
	}
	return queueLength;
}

Ptr<Ipv4Route>
Ipv4DrillRouting::ConstructIpv4Route(uint32_t port, Ipv4Address dstAddr) {
    // Port and channel to send from on this router.
    Ptr<NetDevice> dev = m_ipv4->GetNetDevice(port);
    Ptr<Channel> channel = dev->GetChannel();

    // Get the device on the other end of the channel to get the gateway for
    // the route. There should be two devices attached to the channel at
    // indices 0
    // and 1. dev is the one we send through, so we want the other one. If the
    // device at index 0 is dev then pick index 1, otherwise pick index 0.
    uint32_t otherEnd = (channel->GetDevice(0) == dev) ? 1 : 0;
    // Get the node and interface to send to at the next hop.
    Ptr<Node> nextHop = channel->GetDevice(otherEnd)->GetNode();
    // Interface index.
    uint32_t nextIf = channel->GetDevice(otherEnd)->GetIfIndex();
    // Lastly, recover the IP Address of the next hop.
    Ipv4Address nextHopAddr =
        nextHop->GetObject<Ipv4>()->GetAddress(nextIf, 0).GetLocal();

    // Construct the route. Note the route just tells the router what the
    // next hop to send to is given the destination.
    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetOutputDevice(dev);
    route->SetGateway(nextHopAddr);
    route->SetSource(m_ipv4->GetAddress(port, 0).GetLocal());
    route->SetDestination(dstAddr);
    return route;
}

}  // namespace ns3
