#include "ipv4-lb-flow-stats.h"

namespace ns3 {

void SerializeIpv4LbFlowStatsToCsvStream(std::ostream& os,
                                         FlowId flowId,
                                         Ipv4LbFlowStats& ipv4LbFlowStats,
                                         Time::Unit timeUnit) {
  Time duration = (
    ipv4LbFlowStats.timeLastRxPacket - ipv4LbFlowStats.timeFirstTxPacket);
  // `rxBytes * 8` gives the received bits. Then we divide by the duration in
  // seconds to get bps.
  double effectiveRate = ((ipv4LbFlowStats.rxBytes * 8) /
                           duration.GetSeconds());
  os << flowId << "," << ipv4LbFlowStats.sourceAddress << ","
     << ipv4LbFlowStats.destinationAddress << ","
     << ipv4LbFlowStats.timeFirstTxPacket.As(timeUnit) << ","
     << ipv4LbFlowStats.timeLastTxPacket.As(timeUnit) << ","
     << ipv4LbFlowStats.timeFirstRxPacket.As(timeUnit) << ","
     << ipv4LbFlowStats.timeLastRxPacket.As(timeUnit) << ","
     << ipv4LbFlowStats.delaySum.As(timeUnit) << ","
     << ipv4LbFlowStats.jitterSum.As(timeUnit) << ","
     << ipv4LbFlowStats.txBytes << "," << ipv4LbFlowStats.rxBytes << ","
     << ipv4LbFlowStats.txPackets << "," << ipv4LbFlowStats.rxPackets << ","
     << duration.As(timeUnit) << "," << effectiveRate;
}

}  // namespace ns3
