// Maintains the set of flow statistics relevant to comparing load balancing
// schemes.
#ifndef IPV4_LB_UTILS_H
#define IPV4_LB_UTILS_H

#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"

namespace ns3 {

typedef uint32_t FlowId;

struct Ipv4LbFlowStats {
  // The source address from which the flow originated.
  Ipv4Address sourceAddress;

  // The destination address for the flow.
  Ipv4Address destinationAddress;

  // Contains the sum of all end-to-end delays for all received packets of the
  // flow
  Time delaySum;

  // Contains the sum of all end-to-end delay jitter (delay variation) values
  // for all received packets of the flow.
  Time jitterSum;

  // Total number of transmitted bytes for the flow.
  uint64_t txBytes;

  // Total number of received bytes for the flow.
  uint64_t rxBytes;

  // Total number of transmitted packets for the flow.
  uint32_t txPackets;

  // Total number of received packets for the flow.
  uint32_t rxPackets;
};

void SerializeIpv4LbFlowStatsToCsvStream(std::ostream& os,
                                         FlowId flowId,
                                         Ipv4LbFlowStats& ipv4LbFlowStats);

}  // namespace ns3

#endif  // IPV4_LB_UTILS_H
