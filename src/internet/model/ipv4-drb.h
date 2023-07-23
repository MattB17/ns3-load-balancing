#ifndef IPV4_DRB_H
#define IPV4_DRB_H

#include "ns3/object.h"
#include "ns3/ipv4-address.h"

#include <map>
#include <vector>

namespace ns3 {

class Ipv4Drb : public Object {
public:
	Ipv4Drb();
	~Ipv4Drb();

	static TypeId GetTypeId();

	Ipv4Address GetCoreSwitchAddress(uint32_t flowId);
	void AddCoreSwitchAddress(Ipv4Address addr);
	void AddCoreSwitchAddress(uint32_t weight, Ipv4Address addr);

private:
	// The addresses of the core switches. The idea behind DRB is to ping
	// traffic of a core switch before routing it to its almost destination.
	std::vector<Ipv4Address> m_coreSwitchAddressList;
	// Maintains a map of flow IDs to paths. So if a flow already has an
	// associated path we can just reuse that path.
	std::map<uint32_t, uint32_t> m_flowPathMap;
};

}  // namespace ns3

#endif  // IPV4_DRB_H