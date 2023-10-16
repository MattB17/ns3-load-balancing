#include "ns3/address.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/data-rate.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nstime.h"

#include "lb-utils.h"
#include "load-balancing-scheme.h"

#include <iomanip>
#include <sstream>
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

#define PACKET_SIZE 1400

static uint16_t START_PORT = 1000;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleParallelPathsExample");

void InstallFlows(uint16_t min_port, Ptr<Node> src_node, Ptr<Node> dst_node,
                  Ipv4Address dst_addr, std::string data_rate, size_t num_flows,
                  double flow_start, double flow_end) {
  uint16_t curr_port = min_port;
  for (size_t flow_idx = 0; flow_idx < num_flows; flow_idx++) {
    // Install the on/off sender on the src node to the dst node.
    OnOffHelper onOff("ns3::TcpSocketFactory",
                      Address(InetSocketAddress(dst_addr, curr_port)));
    onOff.SetConstantRate(DataRate(data_rate), PACKET_SIZE);
    ApplicationContainer src_app = onOff.Install(src_node);
    src_app.Start(Seconds(flow_start));
    src_app.Stop(Seconds(flow_end));

    // Create a packet sink to receive the packets.
    // Accepts a connection from any IP address sending on the `curr_port`.
    PacketSinkHelper sink(
      "ns3::TcpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), curr_port)));
    ApplicationContainer dst_app = sink.Install(dst_node);
    dst_app.Start(Seconds(flow_start));
    dst_app.Stop(Seconds(flow_end));

    // Increment port, next application will send from next port.
    curr_port++;
  }
}

int main(int argc, char* argv[]) {
  LogComponentEnable("SimpleParallelPathsExample", LOG_LEVEL_INFO);

  // Setup Random ECMP routing in the case that we fall back to the global
  // router.
  Config::SetDefault(
      "ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

  // Experiment configuration variables. These variables apply to the experiment
  // regardless of the load balancing scheme used.
  uint32_t numNodesInCenter = 3;
  double load = 0.5;
	double flowStartTime = 1.0;
	double flowEndTime = 10.0;
  bool verbose = false;
  bool tracing = true;

  // Load balancing specific variables.
  std::string loadBalancingScheme = "drill";
  uint32_t drillSampleSize = 2;  // Used for DRILL.
  uint16_t flowletTimeoutUs = 100; // Used for flowlet based schemes.

  CommandLine cmd;
  // Experiment configuration variables exposed on the command line.
  cmd.AddValue("numNodesInCenter",
               "Number of nodes in each level of the topology",
               numNodesInCenter);
  cmd.AddValue("load", "Load on the network in the range [0.0, 1.0]", load);
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

  // Variables used for the specific load balancing scheme.
  cmd.AddValue("loadBalancingScheme",
               "The load balancing scheme used in the experiment",
               loadBalancingScheme);
  cmd.AddValue(
    "drillSampleSize",
    "The number of ports DRILL samples when making per packet choices",
    drillSampleSize);
  cmd.AddValue("flowletTimeoutUs",
               "The flowlet timeout in microseconds",
               flowletTimeoutUs);

  cmd.Parse(argc, argv);

  LbScheme lbScheme = StringToLbScheme(loadBalancingScheme);
  // converting it back to string to standardize naming.
  loadBalancingScheme = LbSchemeToString(lbScheme);

  if (lbScheme == LbScheme::UNKNOWN) {
    NS_LOG_ERROR("Must specify a valid load balancing scheme");
    return -1;
  }

  if (verbose) {
    SetLogging(lbScheme, LOG_LEVEL_LOGIC);
  }

  // Create the nodes and links them together.
  NodeContainer n;
  // Number of nodes in center plus two nodes on each side.
  n.Create(numNodesInCenter + 4);

  InternetStackHelper internet = ConfigureLoadBalancing(
    lbScheme, drillSampleSize, flowletTimeoutUs);
  internet.Install(n);

  Ipv4AddressHelper ipv4L;
  ipv4L.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4AddressHelper ipv4R;
  ipv4R.SetBase("10.1.3.0", "255.255.255.0");

	int internalLinkRate = 10;
	int edgeLinkRate = numNodesInCenter * internalLinkRate;

  // We think of every node other than the first and last as being part of
  // the core network infrastructure.
  PointToPointHelper p2pInternal;
  p2pInternal.SetDeviceAttribute("DataRate", StringValue(
    std::to_string(internalLinkRate) + "Mbps"));
  p2pInternal.SetChannelAttribute("Delay", StringValue("10us"));

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
  p2pEdge.SetChannelAttribute("Delay", StringValue("10us"));
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
  PopulateLbRoutingTables(lbScheme);

  // We send `load * edgeLinkRate` number of small flows (at a rate of 1Mbps).
  // Note that `edgeLinkRate` is the network capacity in Mbps so this saturates
  // the network to exactly load.
  int numSmallFlows = (edgeLinkRate * load);
  InstallFlows(START_PORT, n.Get(0), n.Get(numNodesInCenter + 3),
               iiR.GetAddress(1), "1Mbps", numSmallFlows, flowStartTime,
               flowEndTime);
  // Note that in perfect load balancing each link will be loaded to exactly
  // `load` percent. In particular, all the internal links will be saturated to
  // `load * internalLinkRate` Mbps. Then we start up a large flow at halfway
  // through the simulation with rate `((1 - load) * internalLinkRate) + 1` Mbps
  // so that one of the links would be saturated at internalLinkRate + 1 Mbps
  // under perfect load balancing.
  double largeFlowStartTime = (
    (flowEndTime - flowStartTime) / 2) + flowStartTime;
  int largeFlowSendSize = (internalLinkRate * load) + 1;
  std::string largeFlowSendRate = std::to_string(largeFlowSendSize) + "Mbps";
  InstallFlows(START_PORT + numSmallFlows, n.Get(0),
               n.Get(numNodesInCenter + 3), iiR.GetAddress(1),
               largeFlowSendRate, 1, largeFlowStartTime,
               flowEndTime);

  FlowMonitorHelper flowmonHelper;
  std::stringstream ss;
  ss << "outputs/simple-parallel-paths/";
  ss << std::fixed << std::setprecision(1) << load;
  ss << "/" << loadBalancingScheme << "/";
  std::string lbDir = ss.str();

  if (tracing) {
    AsciiTraceHelper ascii;
    p2pInternal.EnableAsciiAll(ascii.CreateFileStream(
      lbDir + "trace.tr"));
    p2pInternal.EnablePcapAll(lbDir + "switch");
    flowmonHelper.InstallAll();
  }

  Simulator::Stop(Seconds(flowEndTime + 1.0));
  Simulator::Run();

  if (tracing) {
    flowmonHelper.SerializeToXmlFile(lbDir + "flows.flowmon", true, true);
    flowmonHelper.LbPerformanceMetricsToFile(
      lbDir + "lb-metrics.csv", Time::NS);
  }

  Simulator::Destroy();

  return 0;
}
