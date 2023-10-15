#include "ipv4-lb-flow-stats.h"

namespace ns3 {

void SerializeIpv4LbFlowStatsToCsvStream(std::ostream& os,
                                         FlowId flowId,
                                         Ipv4LbFlowStats& ipv4LbFlowStats,
                                         Time::Unit timeUnit) {
  os << flowId << "," << ipv4LbFlowStats.sourceAddress << ","
     << ipv4LbFlowStats.destinationAddress << ","
     << ipv4LbFlowStats.delaySum.As(timeUnit) << ","
     << ipv4LbFlowStats.jitterSum.As(timeUnit) << ","
     << ipv4LbFlowStats.txBytes << "," << ipv4LbFlowStats.rxBytes << ","
     << ipv4LbFlowStats.txPackets << "," << ipv4LbFlowStats.rxPackets;
}

}  // namespace ns3
