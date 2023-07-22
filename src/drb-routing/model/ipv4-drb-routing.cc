#include "ipv4-drb-routing.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DrbRouting");

Ipv4DrbRouting::Ipv4DrbRouting() : m_mode(PER_FLOW) {
	NS_LOG_FUNCTION(this);
}

Ipv4DrbRouting::~Ipv4DrbRouting() {
	NS_LOG_FUNCTION(this);
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
		m_paths.push_back(path);
	}
	return true;
}

}  // namespace ns3
