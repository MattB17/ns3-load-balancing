#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-drill-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/on-off-pairs-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nstime.h"

#include <string>

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

NS_LOG_COMPONENT_DEFINE("DrillRoutingExample");

int
main(int argc, char* argv[])
{
	LogComponentEnable("DrillRoutingExample", LOG_LEVEL_INFO);

    // Setup Random ECMP routing in the case that we fall back to the global
    // router.
    Config::SetDefault(
        "ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

    uint32_t numNodesInCenter = 3;
    uint32_t drillSampleSize = 2;
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
	               "The time by which all flows must end",
							   flowEndTime);
    cmd.AddValue("verbose",
                 "Controls whether logging is enabled",
                 verbose);
    cmd.AddValue("tracing",
                 "Controls whether tracing is enabled",
                 tracing);
    cmd.AddValue(
    	"drillSampleSize",
    	"The number of ports DRILL samples when making per packet choices",
		  drillSampleSize);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::Ipv4DrillRouting::d",
    		           UintegerValue(drillSampleSize));

    if (verbose) {
    	LogComponentEnable("Ipv4DrillRouting", LOG_LEVEL_LOGIC);
    }

    // Create the nodes and links them together.
    NodeContainer n;
    // Number of nodes in center plus two nodes on each side.
    n.Create(numNodesInCenter + 4);

    Ipv4DrillRoutingHelper drillRouting;
    InternetStackHelper internet;
    internet.SetRoutingHelper(drillRouting);
    internet.Install(n);

    Ipv4AddressHelper ipv4L;
    ipv4L.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4AddressHelper ipv4R;
    ipv4R.SetBase("10.1.3.0", "255.255.255.0");

		int internalLinkRate = 5;
		int edgeLinkRate = numNodesInCenter * internalLinkRate;

    // We think of every node other than the first and last as being part of
    // the core network infrastructure.
    PointToPointHelper p2pInternal;
    p2pInternal.SetDeviceAttribute("DataRate", StringValue(
			std::to_string(internalLinkRate) + "Mbps"));
    p2pInternal.SetChannelAttribute("Delay", StringValue("50us"));

    NodeContainer nc;
    NetDeviceContainer ndc;
    Ipv4InterfaceContainer iic;

    // Add the necessary IP addresses and point to point links.
    // We add the links from n1 to all of n2 to n_k+1 where k = numNodesInCenter
    // and all the links from n2 to n_k+1 to n_k+2.
    for (int node_idx = 2; node_idx <= numNodesInCenter + 1; node_idx++) {
        nc = NodeContainer(n.Get(1), n.Get(node_idx));
        ndc = p2pInternal.Install(nc);
        iic = ipv4L.Assign(ndc);

        nc = NodeContainer(n.Get(node_idx), n.Get(numNodesInCenter + 2));
        ndc = p2pInternal.Install(nc);
        iic = ipv4R.Assign(ndc);
    }

    // Add the first link and IP address for the link from n0 to n1 as well as
    // the last link and IP address for the link from n_k+2 to n_k+3 where
    // k = numNodesInCenter.
    // We think of the first and last node as being edge nodes.
    PointToPointHelper p2pEdge;
    p2pEdge.SetDeviceAttribute("DataRate", StringValue(
			std::to_string(edgeLinkRate) + "Mbps"));
    p2pEdge.SetChannelAttribute("Delay", StringValue("50us"));
    NodeContainer nL = NodeContainer(n.Get(0), n.Get(1));
    NodeContainer nR = NodeContainer(
    	n.Get(numNodesInCenter+2), n.Get(numNodesInCenter+3));
    NetDeviceContainer ndL = p2pEdge.Install(nL);
    NetDeviceContainer ndR = p2pEdge.Install(nR);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iiL = ipv4.Assign(ndL);
    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iiR = ipv4.Assign(ndR);

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4DrillRoutingHelper::PopulateRoutingTables();

    std::vector<std::string> data_rates = {"400kb/s", "500kb/s", "600kb/s"};
    // iiR.GetAddress(1) retrieves the 2nd interface on the last link from
    // node n_k+2 to n_k+3.
    OnOffPairsHelper pairsHelper = OnOffPairsHelper(
        1000, n.Get(0), n.Get(numNodesInCenter + 3),
        iiR.GetAddress(1), data_rates);
		double flowLaunchEndTime = (
			(flowEndTime - flowStartTime) / 2) + flowStartTime;
    pairsHelper.InstallFlows(
			numSmallFlows, flowStartTime, flowLaunchEndTime, flowEndTime);

    if (tracing) {
        AsciiTraceHelper ascii;
        p2pInternal.EnableAsciiAll(ascii.CreateFileStream(
            "outputs/drill-example/trace.tr"));
        p2pInternal.EnablePcapAll("outputs/drill-example/switch");
    }

    Simulator::Stop(Seconds(flowEndTime + 1.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
