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
 */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "cdf.h"

#include <iostream>

// Two-tier leaf spine topology.
// S stands for spine, l stands for leaf, n represents server nodes
//
//       s0           s1
//       | \        / |
//       |  \      /  |
//       |   \    /   |
//       |    \  /    |
//       |     \/     |
//       |     /\     |
//       |    /  \    |
//       |   /    \   |
//       |  /      \  |
//       | /        \ |
//       |/          \|
//      l0            l1
//    /   \          /  \
//   /     \        /    \
//  n0 ...  n31   n32 ... n63 
//

#define LINK_CAPACITY_BASE    1000000000          // 1Gbps
#define PACKET_SIZE           1400

// The flow port range, each flow will be assigned a random port number within
// this range.
static uint16_t PORT = 1000;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SmallLoadBalanceExample");

double PoissonGenInterval(double avgRate) {
    if (avgRate > 0) {
        return -logf(1.0 - ((double)rand() / RAND_MAX)) / avgRate;
    } else {
        return 0;
    }
}

template<typename T>
T RandRange(T min, T max) {
    return min + (((double)max - min) * (rand() / RAND_MAX));
}

void InstallApplications(
    int srcLeafId, NodeContainer servers, double requestRate,
    struct CdfTable* cdfTable, long& flowCount, long& totalFlowSize,
    int serverCount, int leafCount, double startTime, double endTime,
    double flowLaunchEndTime) {
    for (int serverNum = 0; serverNum < serverCount; serverNum++) {
        int srcServerIdx = (srcLeafId * serverCount) + serverNum;

        // Flow inter-arrival times are poisson.
        double flowStartTime = startTime + PoissonGenInterval(requestRate);
        while (flowStartTime < flowLaunchEndTime) {
            flowCount++;
            uint16_t port = PORT++;

            // Pick a random server attached to the other link.
            int dstServerIdx = (
                (1 - srcLeafId) * serverCount) + RandRange(0, serverCount);

            Ptr<Node> dstServer = servers.Get(dstServerIdx);
            Ptr<Ipv4> ipv4 = dstServer->GetObject<Ipv4>();
            Ipv4InterfaceAddress dstInterface = ipv4->GetAddress(1, 0);
            Ipv4Address dstAddress = dstInterface.GetLocal();

            BulkSendHelper src("ns3::TcpSocketFactory",
                InetSocketAddress(dstAddress, port));
            uint32_t flowSize = GenRandomCdfValue(cdfTable);

            totalFlowSize += flowSize;

            src.SetAttribute("SendSize", UintegerValue(PACKET_SIZE));
            src.SetAttribute("MaxBytes", UintegerValue(flowSize));

            // Install applications
            ApplicationContainer srcApp = src.Install(
                servers.Get(srcServerIdx));
            srcApp.Start(Seconds(flowStartTime));
            srcApp.Stop(Seconds(endTime));

            // Install packet sinks
            // Can accept tcp connection from any address on the given port.
            PacketSinkHelper sink("ns3::TcpSocketFactory",
                InetSocketAddress(Ipv4Address::GetAny(), port));
            ApplicationContainer sinkApp = sink.Install(
                servers.Get(dstServerIdx));
            sinkApp.Start(Seconds(startTime));
            sinkApp.Stop(Seconds(endTime));

            flowStartTime += PoissonGenInterval(requestRate);
        }
    }
}

int main(int argc, char* argv[]) {
    double START_TIME = 0.0;
    double END_TIME = 0.5;
    double FLOW_LAUNCH_END_TIME = 0.2;

    std::string cdfFileName = "examples/load-balancing/DCTCP_CDF.txt";
    unsigned randomSeed = 0;
    double load = 0.0;

    int SPINE_COUNT = 2;
    int LEAF_COUNT = 2;
    int SERVER_COUNT = 32;
    int LINK_COUNT = 2;

    uint32_t linkLatency = 10;
    uint64_t spineToLeafCapacity = 40;
    uint64_t leafToServerCapacity = 10;

    CommandLine cmd;
    cmd.AddValue("startTime", "Start time of the simulation", START_TIME);
    cmd.AddValue("endTime", "End time of the simulation", END_TIME);

    cmd.AddValue("cdfFileName", "File name for flow distribution", cdfFileName);
    cmd.AddValue("randomSeed", "Random seed, 0 for randomly generated", randomSeed);
    cmd.AddValue("load", "Load of the network [0.0, 1.0]", load);

    cmd.AddValue("serverCount", "The number of servers per leaf", SERVER_COUNT);
    cmd.AddValue("spineCount", "The number of spines in the topology", SPINE_COUNT);
    cmd.AddValue("leafCount", "The number of leaf nodes", LEAF_COUNT);
    cmd.AddValue("linkCount", "The number of parallel links between each leaf and spine pair", LEAF_COUNT);

    cmd.AddValue("linkLatency", "Link latency, should be in microseconds", linkLatency);
    cmd.AddValue("spineToLeafCapacity", "Spine <-> Lead capacity in Gbps", spineToLeafCapacity);
    cmd.AddValue("leafToServerCapacity", "Leaf <-> Server capacity in Gbps", leafToServerCapacity);

    cmd.Parse(argc, argv);

    uint64_t SPINE_LEAF_CAPACITY = spineToLeafCapacity * LINK_CAPACITY_BASE;
    uint64_t LEAF_SERVER_CAPACITY = leafToServerCapacity * LINK_CAPACITY_BASE;
    Time LINK_LATENCY = MicroSeconds(linkLatency);

    NodeContainer spines;
    spines.Create(SPINE_COUNT);

    NodeContainer leaves;
    leaves.Create(LEAF_COUNT);

    NodeContainer servers;
    servers.Create(SERVER_COUNT * LEAF_COUNT);

    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRoutingHelper;
    internet.SetRoutingHelper(globalRoutingHelper);

    internet.Install(servers);
    internet.Install(spines);
    internet.Install(leaves);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.0.0", "255.255.255.0");

    PointToPointHelper p2pServerToLeaf;
    p2pServerToLeaf.SetDeviceAttribute(
        "DataRate", DataRateValue(DataRate(LEAF_SERVER_CAPACITY)));
    p2pServerToLeaf.SetChannelAttribute("Delay", TimeValue(LINK_LATENCY));

        for (int leaf_idx = 0; leaf_idx < LEAF_COUNT; leaf_idx++) {
        ipv4.NewNetwork();
        for (int server_num = 0; server_num < SERVER_COUNT; server_num++) {
            int server_idx = (leaf_idx * SERVER_COUNT) + server_num;
            NodeContainer serverLeafContainer = NodeContainer(
                servers.Get(server_idx), leaves.Get(leaf_idx));
            NetDeviceContainer serverLeafDeviceContainer =
                p2pServerToLeaf.Install(serverLeafContainer);
            Ipv4InterfaceContainer serverLeafInterfaceContainer =
                ipv4.Assign(serverLeafDeviceContainer);

        }
    }

    PointToPointHelper p2pLeafToSpine;
    p2pLeafToSpine.SetDeviceAttribute(
        "DataRate", DataRateValue(DataRate(SPINE_LEAF_CAPACITY)));
    p2pLeafToSpine.SetChannelAttribute("Delay", TimeValue(LINK_LATENCY));

    for (int leaf_idx = 0; leaf_idx < LEAF_COUNT; leaf_idx++) {
        for (int spine_idx = 0; spine_idx < SPINE_COUNT; spine_idx++) {
            for (int link_idx = 0; link_idx < LINK_COUNT; link_idx++) {
                ipv4.NewNetwork();

                NodeContainer leafSpineContainer = NodeContainer(
                    leaves.Get(leaf_idx), spines.Get(spine_idx));
                NetDeviceContainer leafSpineDeviceContainer =
                    p2pLeafToSpine.Install(leafSpineContainer);
                Ipv4InterfaceContainer leafSpineInterfaceContainer =
                    ipv4.Assign(leafSpineDeviceContainer);
            }
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Oversubscription ratio: ratio of total capacity of server to leaf links
    // (max volume of traffic that can enter network) to total capacity of
    // leaf to spine links (max volume that can circulate in network core).
    double oversubRatio = static_cast<double>(
        (SERVER_COUNT * LEAF_SERVER_CAPACITY) /
        (SPINE_LEAF_CAPACITY * SPINE_COUNT * LINK_COUNT));
    NS_LOG_INFO("Over-subscription Ratio: " << oversubRatio);

    struct CdfTable* cdfTable = new CdfTable();
    InitCdf(cdfTable);
    LoadCdf(cdfTable, cdfFileName.c_str());

    double cdfAvg = AvgCdf(cdfTable);
    double requestRate = load * LEAF_SERVER_CAPACITY * SERVER_COUNT / oversubRatio / (8 * cdfAvg) / SERVER_COUNT;
    NS_LOG_INFO("CDF average: " << cdfAvg << ", average request rate: " << requestRate << " per second");

    if (randomSeed == 0) {
        srand((unsigned) time(NULL));
    } else {
        srand(randomSeed);
    }

    long flowCount = 0;
    long totalFlowSize = 0;

    for (int srcLeafId = 0; srcLeafId < LEAF_COUNT; srcLeafId++) {
        InstallApplications(
            srcLeafId, servers, requestRate, cdfTable, flowCount,
            totalFlowSize, SERVER_COUNT, LEAF_COUNT, START_TIME, END_TIME,
            FLOW_LAUNCH_END_TIME);
    }

    Simulator::Stop(Seconds(END_TIME));
    Simulator::Run();
    Simulator::Destroy();

    FreeCdf(cdfTable);

    std::cout << "Program ran" << std::endl;
    return 0;
}