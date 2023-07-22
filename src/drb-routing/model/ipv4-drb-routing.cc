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

}  // namespace ns3
