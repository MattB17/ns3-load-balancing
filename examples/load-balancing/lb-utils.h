// A set of utility functions for configuring and managing the load balancing
// scheme used.
#ifndef LB_UTILS_H
#define LB_UTILS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"

#include "load-balancing-scheme.h"

using namespace ns3;

void SetLogging(LbScheme lbScheme, LogLevel level);

InternetStackHelper ConfigureLoadBalancing(LbScheme lbScheme,
                                           uint32_t drillSampleSize,
                                           uint16_t flowletTimeoutUs);

void PopulateLbRoutingTables(LbScheme lbScheme);

#endif  // LB_UTILS_H
