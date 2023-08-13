#include "on-off-pairs-helper.h"
#include "on-off-helper.h"
#include "packet-sink-helper.h"

#include "ns3/address.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OnOffPairsHelper");

template<typename T>
T RandRange(T min, T max) {
    return min + (((double)max - min) * (rand() / RAND_MAX));
}

OnOffPairsHelper::OnOffPairsHelper(uint16_t start_port,
		                           Ptr<Node> src_node,
		                           Ptr<Node> dst_node,
		                           Ipv4Address dst_addr,
		                           std::vector<std::string> data_rates)
	: m_currPort(start_port),
	  m_srcNode(src_node),
	  m_dstNode(dst_node),
	  m_dstAddr(dst_addr),
	  m_dataRates(data_rates)
{}

std::string OnOffPairsHelper::ChooseDataRate() const {
	return m_dataRates[RandRange((size_t)0, m_dataRates.size() - 1)];
}

double OnOffPairsHelper::ChooseFlowLaunchTime(double flow_launch_start,
	                                          double flow_launch_end) const {
	return RandRange(flow_launch_start, flow_launch_end);
}

void OnOffPairsHelper::InstallSourceAndSink(double flow_start,
	                                        double flow_end,
	                                        std::string data_rate) {
	// Install the on off sender on the src node to the destination.
	OnOffHelper onOff("ns3::TcpSocketFactory",
		              Address(InetSocketAddress(m_dstAddr, m_currPort)));
	onOff.SetConstantRate(DataRate(data_rate));
	ApplicationContainer src_app = onOff.Install(m_srcNode);
	src_app.Start(Seconds(flow_start));
	src_app.Stop(Seconds(flow_end));

	// Create a packet sink to receive the packets.
	// Accepts a connection form any IP address on that port.
	PacketSinkHelper sink("ns3::TcpSocketFactory", Address(InetSocketAddress(
		Ipv4Address::GetAny(), m_currPort)));
	ApplicationContainer dst_app = sink.Install(m_dstNode);
	dst_app.Start(Seconds(flow_start));
	dst_app.Stop(Seconds(flow_end));

    // Increment port for next application.
	m_currPort++;
}

void OnOffPairsHelper::InstallFlows(size_t num_flows,
	                                double start_time,
	                                double flow_launch_end_time,
	                                double end_time) {
	double flow_start;
	std::string flow_rate;
	for (size_t flow_idx = 0; flow_idx < num_flows; flow_idx++) {
		flow_start = ChooseFlowLaunchTime(start_time, flow_launch_end_time);
		flow_rate = ChooseDataRate();
		InstallSourceAndSink(flow_start, end_time, flow_rate);
	}
}

}  // namespace ns3