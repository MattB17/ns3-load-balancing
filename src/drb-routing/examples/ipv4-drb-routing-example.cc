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
 *       nE   nF
 *      /|\ //|\
 *     / |/\/ | \
 *    /  | /\ |  \
 *   / / |/  \|  \\
 *  nC   nD   nG  nH
 *  |\  /|    |\  /|
 *  | \/ |    | \/ |
 *  | /\ |    | /\ |
 *  |/  \|    |/  \|
 *  nA   nB   nI   nJ
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
    Ptr<Node> nG = CreateObject<Node>();
    Ptr<Node> nH = CreateObject<Node>();
    Ptr<Node> nI = CreateObject<Node>();
    Ptr<Node> nJ = CreateObject<Node>();
    Ptr<Node> nK = CreateObject<Node>();
    Ptr<Node> nL = CreateObject<Node>();
    Ptr<Node> nM = CreateObject<Node>();
    Ptr<Node> nN = CreateObject<Node>();

    InternetStackHelper internet;
    internet.Install(nA);
    internet.Install(nB);
    internet.Install(nE);
    internet.Install(nF);
    internet.Install(nI);
    internet.Install(nJ);
    internet.Install(nK);
    internet.Install(nL);
    internet.Install(nM);
    internet.Install(nN);

    // Install DRB capability on nodes B and E which are the ones sending
    // traffic to the spine.
    internet.SetDrb();
    internet.Install(nC);
    internet.Install(nD);
    internet.Install(nG);
    internet.Install(nH);

    // Point-to-point links
    NodeContainer nAnC = NodeContainer(nA, nC);
    NodeContainer nAnD = NodeContainer(nA, nD);
    NodeContainer nBnC = NodeContainer(nB, nC);
    NodeContainer nBnD = NodeContainer(nB, nD);
    NodeContainer nCnE = NodeContainer(nC, nE);
    NodeContainer nCnF = NodeContainer(nC, nF);
    NodeContainer nDnE = NodeContainer(nD, nE);
    NodeContainer nDnF = NodeContainer(nD, nF);
    NodeContainer nGnE = NodeContainer(nG, nE);
    NodeContainer nGnF = NodeContainer(nG, nF);
    NodeContainer nHnE = NodeContainer(nH, nE);
    NodeContainer nHnF = NodeContainer(nH, nF);
    NodeContainer nInG = NodeContainer(nI, nG);
    NodeContainer nInH = NodeContainer(nI, nH);
    NodeContainer nJnG = NodeContainer(nJ, nG);
    NodeContainer nJnH = NodeContainer(nJ, nH);

    NodeContainer nKnA = NodeContainer(nK, nA);
    NodeContainer nLnB = NodeContainer(nL, nB);
    NodeContainer nMnI = NodeContainer(nM, nI);
    NodeContainer nNnJ = NodeContainer(nN, nJ);

    // Create the channels without any IP addressing information.
    // The links all have a bit rate of 5Mbps and a delay of 2ms.
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer dAdC = p2p.Install(nAnC);
    NetDeviceContainer dAdD = p2p.Install(nAnD);
    NetDeviceContainer dBdC = p2p.Install(nBnC);
    NetDeviceContainer dBdD = p2p.Install(nBnD);
    NetDeviceContainer dCdE = p2p.Install(nCnE);
    NetDeviceContainer dCdF = p2p.Install(nCnF);
    NetDeviceContainer dDdE = p2p.Install(nDnE);
    NetDeviceContainer dDdF = p2p.Install(nDnF);
    NetDeviceContainer dGdE = p2p.Install(nGnE);
    NetDeviceContainer dGdF = p2p.Install(nGnF);
    NetDeviceContainer dHdE = p2p.Install(nHnE);
    NetDeviceContainer dHdF = p2p.Install(nHnF);
    NetDeviceContainer dIdG = p2p.Install(nInG);
    NetDeviceContainer dIdH = p2p.Install(nInH);
    NetDeviceContainer dJdG = p2p.Install(nJnG);
    NetDeviceContainer dJdH = p2p.Install(nJnH);
    NetDeviceContainer dKdA = p2p.Install(nKnA);
    NetDeviceContainer dLdB = p2p.Install(nLnB);
    NetDeviceContainer dMdI = p2p.Install(nMnI);
    NetDeviceContainer dNdJ = p2p.Install(nNnJ);

    // Assign IP addresses to all the interfaces.
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iAiC = ipv4.Assign(dAdC);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iAiD = ipv4.Assign(dAdD);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iBiC = ipv4.Assign(dBdC);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iBiD = ipv4.Assign(dBdD);

    ipv4.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer iCiE = ipv4.Assign(dCdE);

    ipv4.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer iCiF = ipv4.Assign(dCdF);

    ipv4.SetBase("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer iDiE = ipv4.Assign(dDdE);

    ipv4.SetBase("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer iDiF = ipv4.Assign(dDdF);

    ipv4.SetBase("10.1.9.0", "255.255.255.0");
    Ipv4InterfaceContainer iGiE = ipv4.Assign(dGdE);

    ipv4.SetBase("10.1.10.0", "255.255.255.0");
    Ipv4InterfaceContainer iGiF = ipv4.Assign(dGdF);

    ipv4.SetBase("10.1.11.0", "255.255.255.0");
    Ipv4InterfaceContainer iHiE = ipv4.Assign(dHdE);

    ipv4.SetBase("10.1.12.0", "255.255.255.0");
    Ipv4InterfaceContainer iHiF = ipv4.Assign(dHdF);

    ipv4.SetBase("10.1.13.0", "255.255.255.0");
    Ipv4InterfaceContainer iIiG = ipv4.Assign(dIdG);

    ipv4.SetBase("10.1.14.0", "255.255.255.0");
    Ipv4InterfaceContainer iIiH = ipv4.Assign(dIdH);

    ipv4.SetBase("10.1.15.0", "255.255.255.0");
    Ipv4InterfaceContainer iJiG = ipv4.Assign(dJdG);

    ipv4.SetBase("10.1.16.0", "255.255.255.0");
    Ipv4InterfaceContainer iJiH = ipv4.Assign(dJdH);

    ipv4.SetBase("10.1.17.0", "255.255.255.0");
    Ipv4InterfaceContainer iKiA = ipv4.Assign(dKdA);

    ipv4.SetBase("10.1.18.0", "255.255.255.0");
    Ipv4InterfaceContainer iLiB = ipv4.Assign(dLdB);

    ipv4.SetBase("10.1.19.0", "255.255.255.0");
    Ipv4InterfaceContainer iMiI = ipv4.Assign(dMdI);

    ipv4.SetBase("10.1.20.0", "255.255.255.0");
    Ipv4InterfaceContainer iNiJ = ipv4.Assign(dNdJ);

    // DRB bouncing switch configuration. Nodes E and F are the spine
    // switches so we program C, D, G, and H to bounce off them.
    Ipv4DrbHelper drb;
    Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbC = drb.GetIpv4Drb(ipv4C);
    // The interface of node E on the link CE.
    ipv4DrbC->AddCoreSwitchAddress(iCiE.GetAddress(1));
    // The interface of node F on the link CF.
    ipv4DrbC->AddCoreSwitchAddress(iCiF.GetAddress(1));

    Ptr<Ipv4> ipv4D = nD->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbD = drb.GetIpv4Drb(ipv4D);
    // The interface of node E on the DE link.
    ipv4DrbD->AddCoreSwitchAddress(iDiE.GetAddress(1));
    // The interface of node F on the DF link.
    ipv4DrbD->AddCoreSwitchAddress(iDiF.GetAddress(1));

    Ptr<Ipv4> ipv4G = nG->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbG = drb.GetIpv4Drb(ipv4G);
    // The interface of node E on the link GE.
    ipv4DrbG->AddCoreSwitchAddress(iGiE.GetAddress(1));
    // The interface of node F on the link GF.
    ipv4DrbG->AddCoreSwitchAddress(iGiF.GetAddress(1));

    Ptr<Ipv4> ipv4H = nH->GetObject<Ipv4>();
    Ptr<Ipv4Drb> ipv4DrbH = drb.GetIpv4Drb(ipv4H);
    // The interface of node E on the DE link.
    ipv4DrbH->AddCoreSwitchAddress(iHiE.GetAddress(1));
    // The interface of node F on the DF link.
    ipv4DrbH->AddCoreSwitchAddress(iHiF.GetAddress(1));

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create the OnOff application to send TCP datagrams of size
    // 210 bytes at a rate of 2.5Mbps
    uint16_t port = 22;
    // Send to node N on port 22.
    OnOffHelper onOffKN(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iNiJ.GetAddress(0), port)));
    onOffKN.SetConstantRate(DataRate(2500));
    // Send from node K.
    ApplicationContainer apps = onOffKN.Install(nK);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    // Create a packet sink to receive the packets on node N port 22.
    PacketSinkHelper sinkN(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sinkN.Install(nN);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Create another onoff application that sends from node L to node M
    // on port 22.
    OnOffHelper onOffLM(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iMiI.GetAddress(0), port)));
    onOffLM.SetConstantRate(DataRate(2500));
    // Send from node L.
    apps = onOffLM.Install(nL);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    // Create a packet sink to receive the packets on node M on port 22.
    PacketSinkHelper sinkM(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sinkM.Install(nM);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    port = 44;
    // Create another onoff application that sends from node M to node K
    // on port 44.
    OnOffHelper onOffMA(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iKiA.GetAddress(0), port)));
    onOffMA.SetConstantRate(DataRate(2500));
    // Send from node M.
    apps = onOffMA.Install(nM);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    // Create a packet sink to receive the packets on node K on port 44.
    PacketSinkHelper sinkK(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sinkK.Install(nK);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Create another onoff application that sends from node N to node L
    // on port 22.
    OnOffHelper onOffNL(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iLiB.GetAddress(0), port)));
    onOffNL.SetConstantRate(DataRate(2500));
    // Send from node N.
    apps = onOffNL.Install(nN);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    // Create a packet sink to receive the packets on node L on port 44.
    PacketSinkHelper sinkL(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sinkL.Install(nL);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
