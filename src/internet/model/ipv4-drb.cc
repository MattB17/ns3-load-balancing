#include "ns3/log.h"
#include "ipv4-drb.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4Drb");

NS_OBJECT_ENSURE_REGISTERED(Ipv4Drb);

Ipv4Drb::Ipv4Drb() {
	NS_LOG_FUNCTION(this);
}

Ipv4Drb::~Ipv4Drb() {
	NS_LOG_FUNCTION(this);
}

TypeId Ipv4Drb::GetTypeId() {
	static TypeId tid = TypeId("ns3::Ipv4Drb")
	  .SetParent<Object>()
	  .SetGroupName("Internet")
	  .AddConstructor<Ipv4Drb>();

	return tid;
}

Ipv4Address Ipv4Drb::GetCoreSwitchAddress(uint32_t flowId) {
	NS_LOG_FUNCTION(this);

	if (m_coreSwitchAddressList.size() == 0) {
		return Ipv4Address();
	}

	uint32_t pathIdx;
	auto itr = m_flowPathMap.find(flowId);
	
	// If we already have a path entry for the flow, use that path.
	if (itr != m_flowPathMap.end()) {
		pathIdx = itr->second;
	}
	// Otherwise, just pick a path at random.
	else {
		pathIdx = rand() % m_coreSwitchAddressList.size();
	}

	Ipv4Address addr = m_coreSwitchAddressList[pathIdx];

	// For a given destination or flow, DRB sends packets in a round robin
	// fashion to the core switches. So once we pick the path for this packet
	// the next packet will go to the next core switch, so we increment the
	// path index by 1 and store it in the list for the next time.
	m_flowPathMap[flowId] = ((pathIdx + 1) % m_coreSwitchAddressList.size());

	NS_LOG_DEBUG(
		this << " The index for flow: " << flowId << " is: " << pathIdx);
	return addr;
}

void Ipv4Drb::AddCoreSwitchAddress(Ipv4Address addr) {
	NS_LOG_FUNCTION(this << addr);
	m_coreSwitchAddressList.push_back(addr);
}

void Ipv4Drb::AddCoreSwitchAddress(uint32_t weight, Ipv4Address addr) {
	// Store `weight` copies of `addr`.
	for (uint32_t i = 0; i < weight; i++) {
		Ipv4Drb::AddCoreSwitchAddress(addr);
	}
}
	
}  // namespace ns3