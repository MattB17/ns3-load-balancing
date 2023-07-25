#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-drb-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

/**
 * \file ipv4-drb-routing-example
 *
 * An example of DRB routing
 *
 *  nC    nD
 *   |\  /|
 *   | \/ |
 *   | /\ |
 *   |/  \|
 *  nB    nE
 *   |    |
 *   |    |
 *  nA    nF
 * 
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DrbRouterExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    Ptr<Node> nA = CreateObject<Node>();
    Ptr<Node> nB = CreateObject<Node>();
    Ptr<Node> nC = CreateObject<Node>();
    Ptr<Node> nD = CreateObject<Node>();
    Ptr<Node> nE = CreateObject<Node>();
    Ptr<Node> nF = CreateObject<Node>();

    InternetStackHelper internet;
    internet.Install(nA);
    internet.Install(nC);
    internet.Install(nD);
    internet.Install(nF);

    // Install DRB capability on nodes B and E which are the ones sending
    // traffic to the spine.
    internet.SetDrb();
    internet.Install(nB);
    internet.Install(nE);

    // Point-to-point links
    NodeContainer nAnB = NodeContainer(nA, nB);
    NodeContainer nBnC = NodeContainer(nB, nC);
    NodeContainer nBnD = NodeContainer(nB, nD);
    NodeContainer nEnC = NodeContainer(nE, nC);
    NodeContainer nEnD = NodeContainer(nE, nD);
    NodeContainer nFnE = NodeContainer(nF, nE);

    // Create the channels without any IP addressing information.
    // The links all have a bit rate of 5Mbps and a delay of 2ms.
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer dAdB = p2p.Install(nAnB);
    NetDeviceContainer dBdC = p2p.Install(nBnC);
    NetDeviceContainer dBdD = p2p.Install(nBnD);
    NetDeviceContainer dEdC = p2p.Install(nEnC);
    NetDeviceContainer dEdD = p2p.Install(nEnD);
    NetDeviceContainer dFdE = p2p.Install(nFnE);

    // Assign IP addresses to all the interfaces.
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iAiB = ipv4.Assign(dAdB);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iBiC = ipv4.Assign(dBdC);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iBiD = ipv4.Assign(dBdD);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iEiC = ipv4.Assign(dEdC);

    ipv4.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer iEiD = ipv4.Assign(dEdD);

    ipv4.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer iFiE = ipv4.Assign(dFdE);

    // DRB bouncing switch configuration. Nodes C and D are the core
    // switches so we program B and D to bounce off them.
    Ipv4DrbHelper drb;
    Ptr<Ipv4> ipv4B = nB->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbB = drb.GetIpv4Drb(ipv4B);
    // The interface of node C on the link BC.
    ipv4DrbB->AddCoreSwitchAddress(iBiC.GetAddress(1));
    // The interface of node D on the link BD.
    ipv4DrbB->AddCoreSwitchAddress(iBiD.GetAddress(1));

    Ptr<Ipv4> ipv4E = nE->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbE = drb.GetIpv4Drb(ipv4E);
    // The interface of node C on the EC link.
    ipv4DrbE->AddCoreSwitchAddress(iEiC.GetAddress(1));
    // The interface of node D on the ED link.
    ipv4DrbE->AddCoreSwitchAddress(iEiD.GetAddress(1));

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create the OnOff application to send TCP datagrams of size
    // 210 bytes at a rate of 4000Kbs
    uint16_t port = 22;
    // Send to node F on port 22.
    OnOffHelper onOff(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iFiE.GetAddress(0), port)));
    onOff.SetConstantRate(DataRate(4000));
    // Send from node A
    ApplicationContainer apps = onOff.Install(nA);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Create a packet sink to receive the packets on node F port 22.
    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sink.Install(nF);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
