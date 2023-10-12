#include "lb-utils.h"

#include "ns3/ipv4-drill-routing-helper.h"
#include "ns3/ipv4-ecmp-flow-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-letflow-routing-helper.h"
#include "ns3/nstime.h"

using namespace ns3;

void SetLogging(LbScheme lbScheme, LogLevel level) {
  switch(lbScheme) {
    case LbScheme::ECMP:
      LogComponentEnable("Ipv4EcmpFlowRouting", level);
      break;
    case LbScheme::DRILL:
      LogComponentEnable("Ipv4DrillRouting", level);
      break;
    case LbScheme::LETFLOW:
      LogComponentEnable("Ipv4LetFlowRouting", level);
      break;
    default:
      LogComponentEnable("Ipv4GlobalRouting", level);
      break;
  }
}

InternetStackHelper ConfigureLoadBalancing(LbScheme lbScheme,
                                           uint32_t drillSampleSize,
                                           uint16_t flowletTimeoutUs) {
  InternetStackHelper internet;
  switch (lbScheme) {
    case LbScheme::ECMP:
      {
        Ipv4EcmpFlowRoutingHelper ecmpFlowRouting;
        internet.SetRoutingHelper(ecmpFlowRouting);
        break;
      }
    case LbScheme::DRILL:
      {
        Config::SetDefault("ns3::Ipv4DrillRouting::d",
                           UintegerValue(drillSampleSize));
        Ipv4DrillRoutingHelper drillRouting;
        internet.SetRoutingHelper(drillRouting);
        break;
      }
    case LbScheme::LETFLOW:
      {
        Config::SetDefault("ns3::Ipv4LetFlowRouting::FlowletTimeout",
                           TimeValue(MicroSeconds(flowletTimeoutUs)));
        Ipv4LetFlowRoutingHelper letflowRouting;
        internet.SetRoutingHelper(letflowRouting);
        break;
      }
    default:
      {
        Ipv4GlobalRoutingHelper globalRouting;
        internet.SetRoutingHelper(globalRouting);
        break;
      }
  }
  return internet;
}

void PopulateLbRoutingTables(LbScheme lbScheme) {
  switch (lbScheme) {
    case LbScheme::ECMP:
      Ipv4EcmpFlowRoutingHelper::PopulateRoutingTables();
      break;
    case LbScheme::DRILL:
      Ipv4DrillRoutingHelper::PopulateRoutingTables();
      break;
    case LbScheme::LETFLOW:
      Ipv4LetFlowRoutingHelper::PopulateRoutingTables();
      break;
    default:
      Ipv4GlobalRoutingHelper::PopulateRoutingTables();
      break;
  }
}
