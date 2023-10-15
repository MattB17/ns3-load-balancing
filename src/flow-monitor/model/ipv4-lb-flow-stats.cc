#include "ipv4-lb-flow-stats.h"

namespace ns3 {

void SerializeIpv4LbFlowStatsToCsvStream(std::ostream& os,
                                         FlowId flowId,
                                         Ipv4LbFlowStats& ipv4LbFlowStats) {
  os << flowId << "," << ipv4LbFlowStats.sourceAddress << ","
     << ipv4LbFlowStats.destinationAddress << ","
     << ipv4LbFlowStats.delaySum << "," << ipv4LbFlowStats.jitterSum << ","
     << ipv4LbFlowStats.txBytes << "," << ipv4LbFlowStats.rxBytes << ","
     << ipv4LbFlowStats.txPackets << "," << ipv4LbFlowStats.rxPackets;
}

}  // namespace ns3
