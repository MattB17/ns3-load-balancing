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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SmallLoadBalanceExample");