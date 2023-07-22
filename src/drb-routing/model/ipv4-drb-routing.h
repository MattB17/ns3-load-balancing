#ifndef IPV4_DRB_ROUTING_H
#define IPV4_DRB_ROUTING_H

#include "ns3/object.h"

namespace ns3
{

enum DrbRoutingMode {
    PER_DEST = 0,
    PER_FLOW = 1
};

/**
 * \ingroup drb-routing
 * \brief DRB Load Balancing Protocol
 * 
 * Implements the DRB per-packet load balancing scheme from the paper
 * "Per-packet Load-balanced, Low-Latency Routing for Clos-based Data Center
 * Networks"
 */
class Ipv4DrbRouting {
public:
    Ipv4DrbRouting();
    ~Ipv4DrbRouting();

    bool AddPath(uint32_t path);
    bool AddPath(uint32_t weight, uint32_t path);

private:
    std::vector<uint32_t> m_paths;
    enum DrbRoutingMode m_mode;
};


}  // namespace ns3

#endif  // IPV4_DRB_ROUTING_H
