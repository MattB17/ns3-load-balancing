#include "ipv4-ecmp-flow-routing-helper.h"

#include "ns3/global-router-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4EcmpFlowRoutingHelper");

Ipv4EcmpFlowRoutingHelper::Ipv4EcmpFlowRoutingHelper() {}

Ipv4EcmpFlowRoutingHelper::Ipv4EcmpFlowRoutingHelper(
	const Ipv4EcmpFlowRoutingHelper& o) {

}

Ipv4EcmpFlowRoutingHelper* Ipv4EcmpFlowRoutingHelper::Copy() const {
	return new Ipv4EcmpFlowRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
Ipv4EcmpFlowRoutingHelper::Create(Ptr<Node> node) const {
	NS_LOG_LOGIC("Adding GlobalRouter interface to node " << node->GetId());
	Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
	node->AggregateObject(globalRouter);

	NS_LOG_LOGIC("Adding GlobalRouting interface " << node->GetId());
	Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting>();
	globalRouter->SetRoutingProtocol(globalRouting);

	Ptr<Ipv4EcmpFlowRouting> ecmpFlowRouting =
	    CreateObject<Ipv4EcmpFlowRouting>(globalRouting);
	return ecmpFlowRouting;
}

void Ipv4EcmpFlowRoutingHelper::PopulateRoutingTables() {
	GlobalRouteManager::BuildGlobalRoutingDatabase();
	GlobalRouteManager::InitializeRoutes();
}

void Ipv4EcmpFlowRoutingHelper::RecomputeRoutingTables() {
	GlobalRouteManager::DeleteGlobalRoutes();
	GlobalRouteManager::BuildGlobalRoutingDatabase();
	GlobalRouteManager::InitializeRoutes();
}

Ptr<Ipv4EcmpFlowRouting>
Ipv4EcmpFlowRoutingHelper::GetEcmpFlowRouting(Ptr<Ipv4> ipv4) const {
	Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol();
	// If the routing protocol can be cast to Ipv4EcmpFlowRouting then return
	// the cast.
	if (DynamicCast<Ipv4EcmpFlowRouting>(ipv4rp)) {
		return DynamicCast<Ipv4EcmpFlowRouting>(ipv4rp);
	}
	// If the routing protocol can be cast to Ipv4ListRouting then perform the
	// cast and iterate through the list searching for an Ipv4EcmpFlowRouting
	// object.
	if (DynamicCast<Ipv4ListRouting>(ipv4rp)) {
		Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting>(ipv4rp);
		int16_t priority;
		for (uint32_t route_idx = 0;
			route_idx < lrp->GetNRoutingProtocols();
			route_idx++) {
			Ptr<Ipv4RoutingProtocol> temp = lrp->GetRoutingProtocol(route_idx, priority);
			if (DynamicCast<Ipv4EcmpFlowRouting>(temp)) {
				return DynamicCast<Ipv4EcmpFlowRouting>(temp);
			}
		}
	}
	return nullptr;
}

}  // namespace ns3
