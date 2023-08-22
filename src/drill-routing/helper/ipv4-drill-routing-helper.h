#ifndef IPV4_DRILL_ROUTING_HELPER_H
#define IPV4_DRILL_ROUTING_HELPER_H

#include "ns3/ipv4-drill-routing.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"

namespace ns3
{

class Ipv4DrillRoutingHelper : public Ipv4RoutingHelper {
	/**
	 * \brief Construct a DrillRoutingHelper to more easily manage DRILL
	 * routing.
	 */
	Ipv4DrillRoutingHelper();

	/**
	 * \brief Construct a DrillRoutingHelper from another previously
	 * initialized instance (Copy Constructor).
	 *
	 * \param o Object to be copied.
	 */
	Ipv4DrillRoutingHelper(const Ipv4DrillRoutingHelper& o);

	// Delete assignment operator to avoid misuse.
	Ipv4DrillRoutingHelper& operator=(const Ipv4DrillRoutingHelper&) = delete;

	/**
	 * \return pointer to close of this Ipv4DrillRoutingHelper.
	 */
	Ipv4DrillRoutingHelper* Copy() const override;

	/**
	 * \brief This method is called by ns3::InternetStackHelper::Install.
	 *
	 * \param node The node on which the routing protocol will run.
	 *
	 * \returns a newly-created routing protocol.
	 */
	virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

	/**
	 * \brief Build a routing database and initialize the routing tables of the
	 * nodes in the simulation. Makes all nodes in the simulation into routers.
	 */
	static void PopulateRoutingTables();

	/**
	 * \brief Remove all routes that were previously installed in a prior call
	 * to either PopulateRoutingTables() or RecomputeRoutingTables(), and add
	 * a new set of routes.
	 */
	static void RecomputeRoutingTables();

	/**
	 * \brief Retrieve the Ipv4DrillRouting protocol attached to the helper.
	 *
	 * \param ipv4 The Ptr<Ipv4> to search for the DRILL routing protocol.
	 *
	 * \returns Ipv4DrillRouting pointer or nullptr if not found.
	 */
	Ptr<Ipv4DrillRouting> GetDrillRouting(Ptr<Ipv4> ipv4) const;
};

}  // namespace ns3

#endif  // IPV4_DRILL_ROUTING_HELPER_H
