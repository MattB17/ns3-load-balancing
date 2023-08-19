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

	  // Get the device on the other end of the channel to get the gateway for the
	  // route. There should be two devices attached to the channel at indices 0
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
