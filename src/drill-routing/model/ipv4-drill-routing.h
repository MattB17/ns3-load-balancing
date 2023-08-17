#ifndef IPV4_DRILL_ROUTING_H
#define IPV4_DRILL_ROUTING_H

/**
 * \defgroup drill-routing Load balancing with DRILL.
 * 
 * This section documents the API of the DRILL load balancing module. This
 * module uses DRILL to balance flows by making routing choices on a per
 * packet basis. It does so by keeping track of the least congested links.
 * Then at each decision point it randomly samples two links and takes the
 * previously least congested link. Among the three, whichever has the
 * smallest queue is chosen as the gateway to send the packet.
 */

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup drill-routing directive

/* ... */

}  // namespace ns3

#endif  // IPV4_DRILL_ROUTING_H
