#include "ipv4-drb-routing-helper.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DrbRoutingHelper");

Ipv4DrbRoutingHelper::Ipv4DrbRoutingHelper() {}

Ipv4DrbRoutingHelper::Ipv4DrbRoutingHelper(const Ipv4DrbRoutingHelper&) {}

Ipv4DrbRoutingHelper* Ipv4DrbRoutingHelper::Copy() const {
	return new Ipv4DrbRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol> Ipv4DrbRoutingHelper::Create(Ptr<Node> node) const {
	Ptr<Ipv4DrbRouting> drbRouting = CreateObject<Ipv4DrbRouting>();
	return drbRouting;
}

Ptr<Ipv4DrbRouting>
Ipv4DrbRoutingHelper::GetDrbRouting(Ptr<Ipv4> ipv4) const {
	Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol();
	// If the routing protocol is a vanilla Ipv4DrbRouting object then just
	// return it.
	if (DynamicCast<Ipv4DrbRouting>(ipv4rp)) {
		return DynamicCast<Ipv4DrbRouting>(ipv4rp);
	}
	// If instead the routing protocol is a list routing protocol then parse
	// out the Ipv4DrbRouting protocol (if it's in the list) and return it.
	if (DynamicCast<Ipv4ListRouting>(ipv4rp)) {
		Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting>(ipv4rp);
		int16_t priority;
		// Loop through all the routing protocols, getting the protocol and
		// its corresponding priority.
		for (uint32_t i = 0; i < lrp->GetNRoutingProtocols(); i++) {
			Ptr<Ipv4RoutingProtocol> temp = lrp->GetRoutingProtocol(i, priority);
			if (DynamicCast<Ipv4DrbRouting>(temp)) {
				return DynamicCast<Ipv4DrbRouting>(temp);
			}
		}
	}
	// Otherwise, the routing protocol does not have a Ipv4DrbRouting protocol
	// so just return a null ptr.
	return 0;
}

}  // namespace ns3
