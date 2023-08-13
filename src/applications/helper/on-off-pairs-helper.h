#ifndef ON_OFF_PAIRS_HELPER_H
#define ON_OFF_PAIRS_HELPER_H

#include <string>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/network-module.h"
#include "ns3/ptr.h"

namespace ns3 {

/**
 * \ingroup onoff
 * 
 * \brief A helper class to install a series of OnOffApplication objects and
 * the corresponding PacketSink objects between a source and destination node.
 * 
 * This can be used to simulate multiple senders and multiple destinations,
 * without having to create the corresponding nodes.
 */
class OnOffPairsHelper {
public:
	/**
	 * \brief constructor for OnOffPairsHelper.
	 * 
	 * \params start_port the port from which to start sending flows. Flows
	 *         are then sent on consecutive ports: start_port, start_port + 1,
	 *         start_port + 2, ...
	 * \params src_node the node from which the flows originate.
	 * \params dst_node the destination node for the flows.
	 * \params dst_addr the address of the destination.
	 * \params data_rates a vector of strings denoting possible data rates
	 *         for flows.
	 */
	OnOffPairsHelper(uint16_t start_port,
		             Ptr<Node> src_node,
		             Ptr<Node> dst_node,
		             Ipv4Address dst_addr,
		             std::vector<std::string> data_rates);

    /**
     * \brief installs all pairs of flows between the source and destination.
     * 
     * \param num_flows The number of flows to install.
     * \param start_time The earliest time at which flows can start.
     * \param flow_launch_end_time The latest time at which flows can start.
     * \param end_time The time at which all flows finish.
     */
	void InstallFlows(size_t num_flows, double start_time,
		              double flow_launch_end_time, double end_time);
private:
	/**
	 * \brief chooses a data rate for the current flow.
	 * 
	 * \returns string the data rate for the current flow.
	 */
    std::string ChooseDataRate() const;

    /**
     * \brief chooses a launch time for the current flow.
     * 
     * \param flow_launch_start the earliest time at which a flow can start.
     * \param flow_launch_end the latest time at which a flow can start.
     * 
     * \returns double the time at which the current flow will start. 
     */
    double ChooseFlowLaunchTime(double flow_launch_start,
    	                        double flow_launch_end) const;

    /**
     * \brief Installs a flow between source and destination.
     * 
     * The flow is installed based on the current port.
     * 
     * \param flow_start the time at which the flow will start.
     * \param flow_end the time at which the flow will stop sending.
     * \param data_rate the data rate for the flow.
     */
    void InstallSourceAndSink(double flow_start,
    	                      double flow_end,
    	                      std::string data_rate);

	// The port on which to install the current pair.
	uint16_t m_currPort;
    // The source node from which the flows will be sent.
    Ptr<Node> m_srcNode;
    // The destination node for the flows.
    Ptr<Node> m_dstNode;
    // The address of the destination.
    Ipv4Address m_dstAddr;
	// A vector containing a set of data rates for flows from which the
	// data rate for a current flow is sampled.
	std::vector<std::string> m_dataRates;
};

}  // namespace ns3

#endif  // ON_OFF_PAIRS_HELPER_H