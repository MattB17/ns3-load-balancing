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
#include "ns3/ipv4-ecmp-flow-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
// numNodesInCenter controls the number of nodes in the middle.
// The default is 3 as shown below.
//
//                 n2
//                /  \
//               /    \
//              /      \
//             /        \
//    n0 - - n1 - -n3- - n5 - - n6
//             \        /
//              \      /
//               \    /
//                \  /
//                 n4

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FlowLevelEcmpRouting");

int main(int argc, char* argv[]) {
    // turning on explicit debugging.
    LogComponentEnable("FlowLevelEcmpRouting", LOG_LEVEL_INFO);

    // Packets have size 210 and the data rate is 500kb/s.
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

    // Setup Random ECMP routing in the case that we fall back to the global
    // router.
    Config::SetDefault(
        "ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

    uint32_t numNodesInCenter = 3;
    size_t numSmallFlows = 15;
    double flowStartTime = 1.0;
    double flowEndTime = 10.0;
    bool verbose = false;
    bool tracing = true;

    CommandLine cmd;
    cmd.AddValue("numNodesInCenter",
                 "Number of nodes in each level of the topology",
                 numNodesInCenter);
    cmd.AddValue("numSmallFlows",
                 "number of small flows to start in the simulation",
                 numSmallFlows);
    cmd.AddValue("flowStartTime",
                 "The earliest time at which a flow can be sent",
                 flowStartTime);
    cmd.AddValue("flowEndTime",
                 "The time at which all flows must end",
                 flowEndTime);
    cmd.AddValue("verbose",
                 "Controls whether logging is enabled",
                 verbose);
    cmd.AddValue("tracing",
                 "Controls whether tracing is enabled",
                 tracing);
    cmd.Parse(argc, argv);

    if (verbose) {
        LogComponentEnable("Ipv4EcmpFlowRouting", LOG_LEVEL_LOGIC);
    }

    // Create the nodes and links them together.
    NodeContainer n;
    // Number of nodes in center plus two nodes on each side.
    n.Create(numNodesInCenter + 4);

    Ipv4EcmpFlowRoutingHelper ecmpFlowRouting;
    InternetStackHelper internet;
    internet.SetRoutingHelper(ecmpFlowRouting);
    internet.Install(n);

    Ipv4AddressHelper ipv4L;
    ipv4L.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4AddressHelper ipv4R;
    ipv4R.SetBase("10.1.3.0", "255.255.255.0");

    int internalLinkRate = 5;
    int edgeLinkRate = internalLinkRate * numNodesInCenter;

    // We think of every node other than the first and last as being part of the
    // core network infrastructure.
    PointToPointHelper p2pInternal;
    p2pInternal.SetDeviceAttribute("DataRate", StringValue(
      std::to_string(internalLinkRate) + "Mbps"));
    p2pInternal.SetChannelAttribute("Delay", StringValue("50us"));

    NodeContainer nc;
    NetDeviceContainer ndc;
    Ipv4InterfaceContainer iic;

    // Add the necessary IP addresses and point to point links.
    // We add the links from n0 to all of n1 to nk where k = numNodesInCenter
    // and all the links from n1 to nk to n_k+1.
    for (int node_idx = 2; node_idx <= numNodesInCenter + 1; node_idx++) {
        nc = NodeContainer(n.Get(1), n.Get(node_idx));
        ndc = p2pInternal.Install(nc);
        iic = ipv4L.Assign(ndc);

        nc = NodeContainer(n.Get(node_idx), n.Get(numNodesInCenter + 2));
        ndc = p2pInternal.Install(nc);
        iic = ipv4R.Assign(ndc);
    }

    // Add the last link and IP address for the nodes n_k+1 to n_k+2 where
    // k = numNodesInCenter.
    // We think of the last node as being an edge node.
    PointToPointHelper p2pEdge;
    p2pEdge.SetDeviceAttribute("DataRate", StringValue(
      std::to_string(edgeLinkRate) + "Mbps"));
    p2pEdge.SetChannelAttribute("Delay", StringValue("50us"));
    NodeContainer nL = NodeContainer(n.Get(0), n.Get(1));
    NodeContainer nR = NodeContainer(n.Get(numNodesInCenter+2), n.Get(numNodesInCenter+3));
    NetDeviceContainer ndL = p2pEdge.Install(nL);
    NetDeviceContainer ndR = p2pEdge.Install(nR);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iiL = ipv4.Assign(ndL);
    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iiR = ipv4.Assign(ndR);

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4EcmpFlowRoutingHelper::PopulateRoutingTables();

    std::vector<std::string> data_rates = {"400kb/s", "500kb/s", "600kb/s"};
    OnOffPairsHelper pairsHelper(
        1000, n.Get(0), n.Get(numNodesInCenter + 3),
        iiR.GetAddress(1), data_rates);
    double flowLaunchEndTime = (
      (flowEndTime - flowStartTime) / 2) + flowStartTime;
    pairsHelper.InstallFlows(
      numSmallFlows, flowStartTime, flowLaunchEndTime, flowEndTime);

    if (tracing) {
        AsciiTraceHelper ascii;
        p2pInternal.EnableAsciiAll(ascii.CreateFileStream(
            "outputs/flow-level-ecmp-example/trace.tr"));
        p2pInternal.EnablePcapAll("outputs/flow-level-ecmp-example/switch");
    }

    Simulator::Stop(Seconds(flowEndTime + 1.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;

}
