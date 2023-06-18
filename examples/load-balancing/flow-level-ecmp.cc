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

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NodeContainer nc;
    NetDeviceContainer ndc;
    Ipv4InterfaceContainer iic;

    // Add the necessary IP addresses and point to point links.
    // We add the links from n0 to all of n1 to nk where k = numNodesInCenter
    // and all the links from n1 to nk to n_k+1.
    for (int node_idx = 1; node_idx <= numNodesInCenter; node_idx++) {
        nc = NodeContainer(n.Get(0), n.Get(node_idx));
        ndc = p2p.Install(nc);
        iic = ipv4L.Assign(ndc);

        nc = NodeContainer(n.Get(node_idx), n.Get(numNodesInCenter + 1));
        ndc = p2p.Install(nc);
        iic = ipv4R.Assign(ndc);
    }

    // Add the last link and IP address for the nodes n_k+1 to n_k+2 where
    // k = numNodesInCenter.
    p2p.SetDeviceAttribute("DataRate", StringValue("15Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    nc = NodeContainer(n.Get(numNodesInCenter+1), n.Get(numNodesInCenter+2));
    ndc = p2p.Install(nc);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    iic = ipv4.Assign(ndc);

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create the OnOff application to send UDP datagrams of size 210 bytes at
    // a rate of 448 Kb/s.
    uint16_t port = 9;  // Discard port (RFC 863).
    // Destined to the last node.
    OnOffHelper onoff("ns3::TcpSocketFactory", Address(
        InetSocketAddress(iic.GetAddress(1), port)));
    onoff.SetConstantRate(DataRate("448kb/s"));
    // Install application on node 0.
    ApplicationContainer apps = onoff.Install(n.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Create a packet sink to receive the packets.
    // Accepts a connection from any IP address on that port.
    PacketSinkHelper sink("ns3::TcpSocketFactory", Address(
        InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sink.Install(n.Get(numNodesInCenter + 2));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // A second application sending at a rate of 667 Kb/s.
    uint16_t port2 =  18;
    OnOffHelper onoff2("ns3::TcpSocketFactory", Address(
        InetSocketAddress(iic.GetAddress(1), port2)));
    onoff2.SetConstantRate(DataRate("667kb/s"));
    apps = onoff2.Install(n.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    PacketSinkHelper sink2("ns3::TcpSocketFactory", Address(
        InetSocketAddress(Ipv4Address::GetAny(), port2)));
    apps = sink2.Install(n.Get(numNodesInCenter + 2));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("flow-level-ecmp.tr"));
    p2p.EnablePcapAll("flow-level-ecmp");

    FlowMonitorHelper flowmonHelper;
    flowmonHelper.InstallAll();

    flowmonHelper.SerializeToXmlFile("flow-level-ecmp.flowmon", false, false);

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

}