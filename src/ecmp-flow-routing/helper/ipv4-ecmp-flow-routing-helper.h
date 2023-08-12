#ifndef ECMP_FLOW_ROUTING_HELPER_H
#define ECMP_FLOW_ROUTING_HELPER_H

#include "ns3/ipv4-ecmp-flow-routing.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"

namespace ns3
{

/**
 * \ingroup ipv4EcmpFlowHelper
 * 
 * \brief Helper class that adds ns3::Ipv4EcmpFlowRouting objects.
 */
class Ipv4EcmpFlowRoutingHelper : public Ipv4RoutingHelper {
public:
	/**
	 * \brief Construct a Ipv4EcmpFlowRoutingHelper to more easily manage ECMP
	 * flow-level routing.
	 */
	Ipv4EcmpFlowRoutingHelper();

	/**
	 * \brief Construct a Ipv4EcmpFlowRoutingHelper from another previously
	 * initialized instance (Copy Constructor).
	 * 
	 * \param o Ipv4EcmpFlowRoutingHelper object to be copied.
	 */
	Ipv4EcmpFlowRoutingHelper(const Ipv4EcmpFlowRoutingHelper& o);

	// Delete assignment operator to avoid misuse.
	Ipv4EcmpFlowRoutingHelper& operator=(
		const Ipv4EcmpFlowRoutingHelper&) = delete;

	/**
	 * \return pointer to close of this Ipv4EcmpFlowRoutingHelper.
	 */
	Ipv4EcmpFlowRoutingHelper* Copy() const override;

	/**
	 * \brief This method is called by ns3::InternetStackHelper::Install.
	 * 
	 * \param node The node on which the routing protocol will run.
	 * 
	 * \returns a newly-created routing protocol.
	 */
	Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

	/**
	 * \brief Build a routing database and initialize the routing tables of
	 * the nodes in the simulation. Makes all nodes in the simulation into
	 * routers.
	 */
	static void PopulateRoutingTables();

	/**
	 * \brief Remove all routes that were previously installed in a prior call
	 * to either PopulateRoutingTables() or RecomputeRoutingTables(), and add
	 * a new set of routes.
	 */
	static void RecomputeRoutingTables();

	/**
	 * \brief Retrieve the Ipv4EcmpFlowRouting protocol attached to the
	 * helper.
	 * 
	 * \param ipv4 The Ptr<Ipv4> used to search for the routing protocol.
	 * 
	 * \returns Ipv4EcmpFlowRouting pointer or nullptr if not found.
	 */
	Ptr<Ipv4EcmpFlowRouting> GetEcmpFlowRouting(Ptr<Ipv4> ipv4) const;
};

}  // namespace ns3

#endif  // ECMP_FLOW_ROUTING_HELPER_H
