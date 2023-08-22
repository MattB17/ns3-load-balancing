#include "ipv4-drill-routing-helper.h"

#include "ns3/global-router-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DrillRoutingHelper");

Ipv4DrillRoutingHelper::Ipv4DrillRoutingHelper() {}

Ipv4DrillRoutingHelper::Ipv4DrillRoutingHelper(
		const Ipv4DrillRoutingHelper& o) {
}

Ipv4DrillRoutingHelper* Ipv4DrillRoutingHelper::Copy() const {
	return new Ipv4DrillRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol> Ipv4DrillRoutingHelper::Create(Ptr<Node> node) const {
	// Add the global router interface because this is what is used to run
	// the routing algorithm and construct routes.
	NS_LOG_LOGIC("Adding GlobalRouter interface to node " << node->GetId());
	Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
	node->AggregateObject(globalRouter);

	NS_LOG_LOGIC("Adding GlobalRouting interface " << node->GetId());
	Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting>();
	globalRouter->SetRoutingProtocol(globalRouting);

	Ptr<Ipv4DrillRouting> drillRouting = CreateObject<Ipv4DrillRouting>(
			globalRouting);
	return drillRouting;
}

void Ipv4DrillRoutingHelper::PopulateRoutingTables() {
	GlobalRouteManager::BuildGlobalRoutingDatabase();
	GlobalRouteManager::InitializeRoutes();
}

void Ipv4DrillRoutingHelper::RecomputeRoutingTables() {
	GlobalRouteManager::DeleteGlobalRoutes();
	GlobalRouteManager::BuildGlobalRoutingDatabase();
	GlobalRouteManager::InitializeRoutes();
}

Ptr<Ipv4DrillRouting>
Ipv4DrillRoutingHelper::GetDrillRouting(Ptr<Ipv4> ipv4) const {
	Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol();
	// If the routing protocol can be cast to Ipv4DrillRouting then return
	// the cast.
    if (DynamicCast<Ipv4DrillRouting>(ipv4rp)) {
		return DynamicCast<Ipv4DrillRouting>(ipv4rp);
	}
	// If the routing protocol can be cast to Ipv4ListRouting then perform the
	// cast and iterate through the list searching for an Ipv4DrillRouting
	// object.
	if (DynamicCast<Ipv4ListRouting>(ipv4rp)) {
		Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting>(ipv4rp);
		int16_t priority;
		for (uint32_t route_idx = 0;
			 route_idx < lrp->GetNRoutingProtocols();
			 route_idx++) {
			Ptr<Ipv4RoutingProtocol> temp = lrp->GetRoutingProtocol(
					route_idx, priority);
			if (DynamicCast<Ipv4DrillRouting>(temp)) {
				return DynamicCast<Ipv4DrillRouting>(temp);
			}
		}
	}
	return nullptr;
}

}  // namespace ns3
