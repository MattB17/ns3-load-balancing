/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_LETFLOW_ROUTING_HELPER_H
#define IPV4_LETFLOW_ROUTING_HELPER_H

#include "ns3/ipv4-letflow-routing.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"

namespace ns3 {

class Ipv4LetFlowRoutingHelper : public Ipv4RoutingHelper {
public:
	/**
	 * \brief Construct a LetFlowRoutingHelper to more easily manage
	 * LetFlow routing.
	 */
	Ipv4LetFlowRoutingHelper();

	/**
	 * \brief Construct a LetFlowRoutingHelper from another previously
	 * initialized instance (Copy Constructor).
	 * 
	 * \param o object to be copied.
	 */
	Ipv4LetFlowRoutingHelper(const Ipv4LetFlowRoutingHelper& o);

	// Delete assignment operator to avoid misuse.
	Ipv4LetFlowRoutingHelper& operator=(
		const Ipv4LetFlowRoutingHelper&) = delete;

    /**
     * \returns pointer to clone of this Ipv4LetFlowRoutingHelper.
     */
	Ipv4LetFlowRoutingHelper* Copy(void) const override;

    /**
     * \brief This method is called by ns3::InternetStackHelper::Install.
     * 
     * \param node The node on which the routing protocol will run.
     * 
     * \returns a newly-created routing protocol.
     */
	virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

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
     * \brief Retrieve the Ipv4LetFlowRouting protocol attached to the helper.
     * 
     * \param ipv4 The Ptr<Ipv4> to search for the LetFlow routing protocol.
     * 
     * \returns Ipv4LetFlowRouting pointer or nullptr if not found.
     */
	Ptr<Ipv4LetFlowRouting> GetLetFlowRouting(Ptr<Ipv4> ipv4) const;
};

}  // namespace ns3

#endif  // IPV4_LETFLOW_ROUTING_HELPER_H