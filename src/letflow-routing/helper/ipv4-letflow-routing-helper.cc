/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ipv4-letflow-routing-helper.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4LetFlowRoutingHelper");

Ipv4LetFlowRoutingHelper::Ipv4LetFlowRoutingHelper() {}

Ipv4LetFlowRoutingHelper::Ipv4LetFlowRoutingHelper(
	const Ipv4LetFlowRoutingHelper&) {
}

Ipv4LetFlowRoutingHelper* Ipv4LetFlowRoutingHelper::Copy(void) const {
	return new Ipv4LetFlowRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
Ipv4LetFlowRoutingHelper::Create(Ptr<Node> node) const {
	Ptr<Ipv4LetFlowRouting> letFlowRouting = CreateObject<Ipv4LetFlowRouting>();
	return letFlowRouting;
}

Ptr<Ipv4LetFlowRouting>
Ipv4LetFlowRoutingHelper::GetLetFlowRouting(Ptr<Ipv4> ipv4) const {
	Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol();
	// If the routing protocol can be cast to Ipv4LetFlowRouting then return
	// the cast.
	if (DynamicCast<Ipv4LetFlowRouting>(ipv4rp)) {
		return DynamicCast<Ipv4LetFlowRouting>(ipv4rp);
	}
	// If the routing protocol can be cast to Ipv4ListRouting then perform the
	// cast and iterate through the list searching for a Ipv4LetFlowRouting.
	if (DynamicCast<Ipv4ListRouting>(ipv4rp)) {
		Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting>(ipv4rp);
		int16_t priority;
		for (uint32_t route_idx = 0;
			 route_idx < lrp->GetNRoutingProtocols();
			 route_idx++) {
			Ptr<Ipv4RoutingProtocol> temp = lrp->GetRoutingProtocol(
				route_idx, priority);
		    if (DynamicCast<Ipv4LetFlowRouting>(temp)) {
		    	return DynamicCast<Ipv4LetFlowRouting>(temp);
		    }
		}
	}
	return 0;
}

}  // namespace ns3