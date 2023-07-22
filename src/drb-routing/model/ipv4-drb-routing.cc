#include "ipv4-drb-routing.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/flow-id-tag.h"
#include "ns3/ipv4-xpath-tag.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DrbRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4DrbRouting);

Ipv4DrbRouting::Ipv4DrbRouting() : m_mode(PER_FLOW) {
	NS_LOG_FUNCTION(this);
}

Ipv4DrbRouting::~Ipv4DrbRouting() {
	NS_LOG_FUNCTION(this);
}

TypeId Ipv4DrbRouting::GetTypeId() {
	static TypeId tid = TypeId("ns3::Ipv4DrbRouting")
	    .SetParent<Object>()
	    .SetGroupName("DRBRouting")
	    .AddConstructor<Ipv4DrbRouting>()
	    .AddAttribute("Mode", "DRB Mode: 0 for PER DEST, 1 for PER FLOW",
	    	          UintegerValue(1),
	    	          MakeUintegerAccessor(&Ipv4DrbRouting::m_mode),
	    	          MakeUintegerChecker<uint32_t>());

	return tid;
}

bool Ipv4DrbRouting::AddPath(uint32_t path) {
	return this->AddPath(1, path);
}

bool Ipv4DrbRouting::AddPath(uint32_t weight, uint32_t path) {
	if (weight != 1 && m_mode != PER_FLOW) {
		NS_LOG_ERROR(
			"You must use PER_FLOW when `weight` is different from 1");
		return false;
	}

    // push back `weight` instances of path.
	for (uint32_t i = 0; i < weight; i++) {
		m_allPaths.push_back(path);
	}
	return true;
}


bool Ipv4DrbRouting::AddWeightedPathToDsts(
	uint32_t weight, uint32_t path,
    const std::set<Ipv4Address>& dstIPs) {
    // First we add path to the list of all paths.
    Ipv4DrbRouting::AddPath(weight, path);

    // Add rules to all other tables
    std::set<Ipv4Address>::iterator itr = dstIPs.begin();
    for (; itr != dstIPs.end(); itr++) {
    	// We start from an empty list of paths, if we don't currently know
    	// of any paths to the destination we will just add the current path.
    	std::vector<uint32_t> paths;
    	auto dstPathPair = m_dstPaths.find(*itr);

    	// If we already have a list of paths for this destination, start
    	// from there.
    	if (dstPathPair != m_dstPaths.end()) {
    		paths = dstPathPair->second; 
    	}

    	// Now we add the path to the list.
    	for (uint32_t i = 0; i < weight; i++) {
    		paths.push_back(path);
    	}
    	m_dstPaths[*itr] = paths;
    }
    return true;
}

bool Ipv4DrbRouting::AddWeightedPathToDst(
 	Ipv4Address dstAddr, uint32_t weight, uint32_t path) {
 	// First we add path to the list of all paths.
 	Ipv4DrbRouting::AddPath(weight, path);

 	// Now add rule to the destination paths. We start from an empty list of
 	// paths, if we don't currently know of any paths to the destination then
 	// we will just add the current path.
 	std::vector<uint32_t> paths;
 	auto dstPathPair = m_dstPaths.find(dstAddr);

 	// If we already have a list of paths for this destination, start
 	// from there.
 	if (dstPathPair != m_dstPaths.end()) {
 		paths = dstPathPair->second;
 	}

 	// Now we add the path to the list.
 	for (uint32_t i = 0; i < weight; i++) {
 		paths.push_back(path);
 	}
 	m_dstPaths[dstAddr] = paths;

 	return true;
}

// In DRB, the RouteOutput method will not actually route the packets out
// but will instead assign the path ID. DRB relies on list routing and static
// routing to do the actual forwarding.
Ptr<Ipv4Route> Ipv4DrbRouting::RouteOutput(
 	Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif,
 	Socket::SocketErrno& sockerr) {
 	if (p == 0) {
 		return 0;
 	}

 	uint32_t flowIdentity = 0;
 	if (m_mode == PER_FLOW) {
 		FlowIdTag flowIdTag;
 		bool found = p->PeekPacketTag(flowIdTag);
 		if (!found) {
 			sockerr = Socket::ERROR_NOROUTETOHOST;
 			return 0;
 		}
 		flowIdentity = flowIdTag.GetFlowId();
 		NS_LOG_LOGIC("For flow with flow id: " << flowIdentity);
 	}
 	// m_mode == PER_DEST
 	else {
 		// header.GetDestination() returns a Ipv4Address. Calling Get() on the
 		// result gives a uint32_t.
 		flowIdentity = header.GetDestination().Get();
 		NS_LOG_LOGIC("For flow with dest: " << flowIdentity);
 	}

 	std::vector<uint32_t> paths;
 	auto dstItr = m_dstPaths.find(header.GetDestination());
 	if (dstItr == m_dstPaths.end()) {
 		// If we can't find the destination, just pick a random path.
 		paths = m_allPaths;
 	} else {
 		// Otherwise use the set of paths to the destination.
 		paths = dstItr->second;
 	}

 	uint32_t pathIdx;
 	auto flowItr = m_flowPathMap.find(flowIdentity);
 	if (flowItr != m_flowPathMap.end()) {
 		// If we already have a path for the flow, send it there.
 		pathIdx = flowItr->second;
 	} else {
 		// Otherwise, pick a random path.
 		pathIdx = rand() % paths.size();
 	}

 	// DRB sends the packets of a flow (or to a destination) in a round robin
 	// fashion. So we send the packet down the current path and increment the
 	// path index by 1 for the next packet.
 	uint32_t path = paths[pathIdx];
 	m_flowPathMap[flowIdentity] = (pathIdx + 1) % paths.size();

    // DRB doesn't actually do the sending, just tags the packet with the
    // path.
 	Ipv4XPathTag ipv4XPathTag;
 	ipv4XPathTag.SetPathId(path);
 	p->AddPacketTag(ipv4XPathTag);

 	NS_LOG_LOGIC("DRB Routing has assigned path: " << path);

 	sockerr = Socket::ERROR_NOTERROR;

 	return 0;
}

bool Ipv4DrbRouting::RouteInput(
 	Ptr<const Packet> p, const Ipv4Header& header, Ptr<const NetDevice> idev,
 	UnicastForwardCallback usb, MulticastForwardCallback mcb,
 	LocalDeliverCallback lcb, ErrorCallback ecb) {
 	NS_LOG_ERROR("DRB can only support end host routing");
 	ecb(p, header, Socket::ERROR_NOROUTETOHOST);
 	return false;
}

void Ipv4DrbRouting::NotifyInterfaceUp(uint32_t interface) {}

void Ipv4DrbRouting::NotifyInterfaceDown(uint32_t interface) {}

void Ipv4DrbRouting::NotifyAddAddress(
 	uint32_t interface, Ipv4InterfaceAddress address) {}

void Ipv4DrbRouting::NotifyRemoveAddress(
	uint32_t interface, Ipv4InterfaceAddress address) {}

void Ipv4DrbRouting::SetIpv4(Ptr<Ipv4> ipv4) {
	NS_LOG_LOGIC(this << " Setting up Ipv4: " << ipv4);
 	NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
 	m_ipv4 = ipv4;
}

void Ipv4DrbRouting::PrintRoutingTable(
	Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {}

void Ipv4DrbRouting::DoDispose() {}

}  // namespace ns3
