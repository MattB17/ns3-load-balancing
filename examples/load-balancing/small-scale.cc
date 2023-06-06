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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SmallLoadBalanceExample");

int main(int argc, char* argv[]) {
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    double load = 0.5;

    int SPINE_COUNT = 2;
    int LEAF_COUNT = 2;
    int SERVER_COUNT = 32;
    int LINK_COUNT = 2;

    uint32_t linkLatency = 10;
    uint64_t spineToLeafCapacity = 40;
    uint64_t leafToServerCapacity = 10;

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
    Ipv4GlobalRoutingHelper(globalRoutingHelper);

    internet.Install(servers);
    internet.Install(spines);
    internet.install(leaves);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.0.0", "255.255.255.0");

    PointToPointHelper p2pServerToLeaf;
    p2pServerToLeaf.SetDeviceAttribute(
        "DataRate", DataRateValue(DataRate(LEAF_SERVER_CAPACITY)));
    p2pServerToLeaf.SetChannelAttribute("Delay", TimeValue(LINK_LATENCY));
    p2pServerToLeaf.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(100));

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
    p2pLeafToSpine.SetQueue("Ns3::DropTailQueue", "MaxPackets", UintegerValue(100));

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
}