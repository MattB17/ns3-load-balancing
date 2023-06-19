/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
// numNodesInCenter controls the number of nodes in the middle.
// The default is 3 as shown below.
//
//                 n1
//                /  \
//               /    \
//              /      \
//             /        \
//           n0 - -n2- - n4 - - n5
//             \        /
//              \      /
//               \    /
//                \  /
//                 n3

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FlowLevelEcmpRouting");

template<typename T>
T RandRange(T min, T max) {
    return min + (((double)max - min) * (rand() / RAND_MAX));
}

void InstallSourceAndSink(uint16_t port_num, Ptr<Node> src_node,
                          Ptr<Node> dst_node, Ipv4Address dst_addr,
                          double start_time, double end_time,
                          std::string data_rate) {
    OnOffHelper onoff("ns3::TcpSocketFactory", Address(InetSocketAddress(
        dst_addr, port_num)));
    onoff.SetConstantRate(DataRate(data_rate));
    ApplicationContainer src_app = onoff.Install(src_node);
    src_app.Start(Seconds(start_time));
    src_app.Stop(Seconds(end_time));

    // Create a packet sink to receive the packets.
    // Accepts a connection from any IP address on that port.
    PacketSinkHelper sink("ns3::TcpSocketFactory", Address(InetSocketAddress(
        Ipv4Address::GetAny(), port_num)));
    ApplicationContainer dst_app = sink.Install(dst_node);
    dst_app.Start(Seconds(start_time));
    dst_app.Stop(Seconds(end_time));
}

void InstallApplications(uint16_t start_port, size_t num_flows,
                         Ptr<Node> src_node, Ptr<Node> dst_node, 
                         Ipv4Address dst_addr, double start_time,
                         double flow_launch_end_time, double end_time,
                         std::vector<std::string> data_rates) {
    uint16_t curr_port = start_port;
    double curr_flow_launch_time;
    std::string curr_data_rate;
    for (size_t flow_idx = 0; flow_idx < num_flows; flow_idx++) {
        curr_flow_launch_time = RandRange(start_time, flow_launch_end_time);
        curr_data_rate = data_rates[RandRange((size_t)0, data_rates.size() - 1)];
        InstallSourceAndSink(curr_port, src_node, dst_node, dst_addr,
                             curr_flow_launch_time, end_time, curr_data_rate);
        curr_port++;
    }
}

int main(int argc, char* argv[]) {
    // turning on explicit debugging.
#if 1
    LogComponentEnable("FlowLevelEcmpRouting", LOG_LEVEL_INFO);
#endif

    // Packets have size 210 and the data rate is 500kb/s.
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

    // Setup ECMP routing.
    Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

    uint32_t numNodesInCenter = 3;
    size_t numSmallFlows = 15;

    CommandLine cmd;
    cmd.AddValue("numNodesInCenter", 
                 "Number of nodes in each level of the topology",
                 numNodesInCenter);
    cmd.AddValue("numSmallFlows",
                 "number of small flows to start in the simulation",
                 numSmallFlows);
    cmd.Parse(argc, argv);

    // Create the nodes and link them together.
    NodeContainer n;
    n.Create(numNodesInCenter + 3);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv4AddressHelper ipv4L;
    ipv4L.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4AddressHelper ipv4R;
    ipv4R.SetBase("10.1.2.0", "255.255.255.0");

    // We think of every node other than the last as being part of the core
    // network infrastructure.
    PointToPointHelper p2pInternal;
    p2pInternal.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2pInternal.SetChannelAttribute("Delay", StringValue("2ms"));

    NodeContainer nc;
    NetDeviceContainer ndc;
    Ipv4InterfaceContainer iic;

    // Add the necessary IP addresses and point to point links.
    // We add the links from n0 to all of n1 to nk where k = numNodesInCenter
    // and all the links from n1 to nk to n_k+1.
    for (int node_idx = 1; node_idx <= numNodesInCenter; node_idx++) {
        nc = NodeContainer(n.Get(0), n.Get(node_idx));
        ndc = p2pInternal.Install(nc);
        iic = ipv4L.Assign(ndc);

        nc = NodeContainer(n.Get(node_idx), n.Get(numNodesInCenter + 1));
        ndc = p2pInternal.Install(nc);
        iic = ipv4R.Assign(ndc);
    }

    // Add the last link and IP address for the nodes n_k+1 to n_k+2 where
    // k = numNodesInCenter.
    // We think of the last node as being an edge node.
    PointToPointHelper p2pEdge;
    p2pEdge.SetDeviceAttribute("DataRate", StringValue("15Mbps"));
    p2pEdge.SetChannelAttribute("Delay", StringValue("10ms"));
    nc = NodeContainer(n.Get(numNodesInCenter+1), n.Get(numNodesInCenter+2));
    ndc = p2pEdge.Install(nc);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    iic = ipv4.Assign(ndc);

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    std::vector<std::string> data_rates = {"400kb/s", "500kb/s", "600kb/s"};
    InstallApplications(1000, numSmallFlows, n.Get(0),
                        n.Get(numNodesInCenter + 2), iic.GetAddress(1),
                        1.0, 5.0, 10.0, data_rates);

    InstallSourceAndSink(1000 + numSmallFlows, n.Get(0),
                         n.Get(numNodesInCenter + 2), iic.GetAddress(1),
                         6.0, 10.0, "3Mbps");

    AsciiTraceHelper ascii;
    p2pInternal.EnableAsciiAll(ascii.CreateFileStream("flow-level-ecmp.tr"));
    p2pInternal.EnablePcapAll("flow-level-ecmp");

    FlowMonitorHelper flowmonHelper;
    flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();

    // Needs to be after the run command to pick up the flows.
    flowmonHelper.SerializeToXmlFile("flow-level-ecmp.flowmon", true, false);

    Simulator::Destroy();

}