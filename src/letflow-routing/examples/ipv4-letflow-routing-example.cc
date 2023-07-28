#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
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

    uint16_t sendRate = 6000;

    double endTime = 10.0;

    CommandLine cmd;
    
    cmd.AddValue("tracing", "Whether or not tracing is enabled", tracing);
    cmd.AddValue(
        "sendRate",
        "The rate at which the application sends data in bps",
        sendRate);
    cmd.AddValue(
        "endTime", "The time when the application stops sending", endTime);

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

    // Create the peer-to-peer links with data rates 5Mbps and delay of 2ms.
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer dAdB = p2p.Install(nAnB);
    NetDeviceContainer dBdC = p2p.Install(nBnC);
    NetDeviceContainer dBdD = p2p.Install(nBnD);
    NetDeviceContainer dEdC = p2p.Install(nEnC);
    NetDeviceContainer dEdD = p2p.Install(nEnD);
    NetDeviceContainer dFdE = p2p.Install(nFnE);

    // Add IP addresses to the devices.
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

    // Initialize routing database and set up the routing tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create an OnOff application to send TCP datagrams at a data rate of
    // `sendRate` bps
    uint16_t port = 22;
    // Send to the interface of F on link EF at port 22.
    OnOffHelper onoff(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(iFiE.GetAddress(0), port)));
    onoff.SetConstantRate(DataRate(sendRate));
    // Send from node A.
    ApplicationContainer apps = onoff.Install(nA);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(endTime));

    // Create a packet sink to receive these packets. It will accept a
    // connection from any address on port 22.
    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    apps = sink.Install(nF);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(endTime));

    if (tracing) {
        AsciiTraceHelper ascii;
        p2p.EnableAsciiAll(ascii.CreateFileStream("outputs/letflow-example/trace.tr"));
        p2p.EnablePcapAll("outputs/letflow-example/switch");

        FlowMonitorHelper flowmonHelper;
        flowmonHelper.InstallAll();

        Simulator::Run();

        flowmonHelper.SerializeToXmlFile(
            "outputs/letflow-example/monitoring.flowmon", true, true);
    } else {
        Simulator::Run();
    }

    Simulator::Destroy();

    return 0;
}
