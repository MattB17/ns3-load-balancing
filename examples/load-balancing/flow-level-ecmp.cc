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

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

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

void InstallSourceAndSink(uint16_t port_num, Ptr<Node> src_node,
                          Ptr<Node> dst_node, Ipv4Address dst_addr,
                          std::string data_rate, double start_time,
                          double end_time) {
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

int main(int argc, char* argv[]) {
    // turning on explicit debugging.
#if 1
    LogComponentEnable("FlowLevelEcmpRouting", LOG_LEVEL_INFO);
#endif

    // Packets have size 210 and the data rate is 448kb/s.
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("448kb/s"));

    // Setup ECMP routing.
    Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

    uint32_t numNodesInCenter = 3;

    CommandLine cmd;
    cmd.AddValue("numNodesInCenter", 
                 "Number of nodes in each level of the topology",
                 numNodesInCenter);
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

    // Create the OnOff application to send UDP datagrams of size 210 bytes at
    // a rate of 448 Kb/s.
    uint16_t port = 9;  // Discard port (RFC 863).
    InstallSourceAndSink(port, n.Get(0), n.Get(numNodesInCenter + 2),
                         iic.GetAddress(1), "448kb/s", 1.0, 10.0);

    // A second application sending at a rate of 667 Kb/s.
    uint16_t port2 =  18;
    InstallSourceAndSink(port2, n.Get(0), n.Get(numNodesInCenter + 2),
                         iic.GetAddress(1), "667kb/s", 1.5, 10.0);

    // A third application sending at a rate of 448 Kb/s.
    uint16_t port3 = 27;
    InstallSourceAndSink(port3, n.Get(0), n.Get(numNodesInCenter + 2),
                         iic.GetAddress(1), "448kb/s", 2.0, 10.0);

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