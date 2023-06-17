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
    Config::SetDefault("ns3::Ipv4GlobalRouting::PerflowEcmpRouting", BooleanValue(true));

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

    Ipv4AddressHelper ivp4L;
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
    for (int node_idx = 1; node_idx <= numNodesInCenter; node_idx++) {
        nc = NodeContainer(n.Get(0), n.Get(node_idx));
        ndc = p2p.Install(nc);
        iic = ipv4L.Install(ndc);

        nc = NodeContainer(n.Get(node_idx), n.Get(numNodesInCenter + 1));
        ndc = p2p.Install(nc);
        iic = Ipv4R.Install(ndc);
    }

    Simulator::Stop(Seconds(11));
    Simulator::Run();
    Simulator::Destroy();

}