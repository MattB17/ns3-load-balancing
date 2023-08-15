#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-letflow-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nstime.h"

#include <iostream>

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

NS_LOG_COMPONENT_DEFINE("LetFlowRoutingExample");

int main(int argc, char* argv[])
{
    LogComponentEnable("LetFlowRoutingExample", LOG_LEVEL_INFO);

    // Packets have size 210 and the data rate is 500kb/s.
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

    // Setup Random ECMP routing in the case that we fall back to the global
    // router.
    Config::SetDefault(
        "ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

    uint32_t numNodesInCenter = 3;
    uint16_t flowletTimeoutUs = 100;
    size_t numSmallFlows = 15;
    bool verbose = false;
    bool tracing = true;

    CommandLine cmd;
    cmd.AddValue("numNodesInCenter", 
                 "Number of nodes in each level of the topology",
                 numNodesInCenter);
    cmd.AddValue("numSmallFlows",
                 "number of small flows to start in the simulation",
                 numSmallFlows);
    cmd.AddValue("verbose",
                 "Controls whether logging is enabled",
                 verbose);
    cmd.AddValue("tracing",
                 "Controls whether tracing is enabled",
                 tracing);
    cmd.AddValue("flowletTimeoutUs",
                 "The flowlet timeout in microseconds",
                 flowletTimeoutUs);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::Ipv4LetFlowRouting::FlowletTimeout",
                       TimeValue(MicroSeconds(flowletTimeoutUs)));

    if (verbose) {
        LogComponentEnable("Ipv4LetFlowRouting", LOG_LEVEL_LOGIC);
    }

    // Create the nodes and links them together.
    NodeContainer n;
    // Number of nodes in center plus two nodes on each side.
    n.Create(numNodesInCenter + 4);

    Ipv4LetFlowRoutingHelper letFlowRouting;
    InternetStackHelper internet;
    internet.SetRoutingHelper(letFlowRouting);
    internet.Install(n);

    Ipv4AddressHelper ipv4L;
    ipv4L.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4AddressHelper ipv4R;
    ipv4R.SetBase("10.1.3.0", "255.255.255.0");

    // We think of every node other than the last as being part of the core
    // network infrastructure.
    PointToPointHelper p2pInternal;
    p2pInternal.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
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
    p2pEdge.SetDeviceAttribute("DataRate", StringValue("15Mbps"));
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
    Ipv4LetFlowRoutingHelper::PopulateRoutingTables();

    std::vector<std::string> data_rates = {"400kb/s", "500kb/s", "600kb/s"};
    OnOffPairsHelper pairsHelper(
        1000, n.Get(0), n.Get(numNodesInCenter + 3),
        iiR.GetAddress(1), data_rates);
    pairsHelper.InstallFlows(numSmallFlows, 1.0, 5.0, 10.0);

    if (tracing) {
        AsciiTraceHelper ascii;
        p2pInternal.EnableAsciiAll(ascii.CreateFileStream(
            "outputs/letflow-example/trace.tr"));
        p2pInternal.EnablePcapAll("outputs/letflow-example/switch");
    }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
