#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-letflow-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

/**
 * \file ipv4-let-flow-routing-example
 *
 * An example of LetFlow Routing.
 * 
 * The topology is given by the following diagram with node A sending
 * traffic to node F:
 * 
 *  nC   nD
 *  |\  /|
 *  | \/ |
 *  | /\ |
 *  |/  \|
 *  nB   nE
 *  |    |
 *  |    |
 *  |    |
 *  nA   nF
 */

using namespace ns3;

int
main(int argc, char* argv[])
{
    bool tracing = false;

    CommandLine cmd;
    
    cmd.AddValue("tracing", "Whether or not tracing is enabled", tracing);

    cmd.Parse(argc, argv);

    Ptr<Node> nA = CreateObject<Node>();
    Ptr<Node> nB = CreateObject<Node>();
    Ptr<Node> nC = CreateObject<Node>();
    Ptr<Node> nD = CreateObject<Node>();
    Ptr<Node> nE = CreateObject<Node>();
    Ptr<Node> nF = CreateObject<Node>();

    NodeContainer nAnB(nA, nB);
    NodeContainer nBnC(nB, nC);
    NodeContainer nBnD(nB, nD);
    NodeContainer nEnC(nE, nC);
    NodeContainer nEnD(nE, nD);
    NodeContainer nFnE(nF, nE);

    NodeContainer allNodes(nA, nB, nC, nD, nE, nF);

    Ipv4LetFlowRoutingHelper letflowRouting;

    InternetStackHelper internet;
    internet.SetRoutingHelper(letflowRouting);
    internet.Install(allNodes);

    // Create the channels first without any IP addressing information.
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer dAdB = p2p.Install(nAnB);
    NetDeviceContainer dBdC = p2p.Install(nBnC);
    NetDeviceContainer dBdD = p2p.Install(nBnD);
    NetDeviceContainer dEdC = p2p.Install(nEnC);
    NetDeviceContainer dEdD = p2p.Install(nEnD);
    NetDeviceContainer dFdE = p2p.Install(nFnE);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
