// An enum storing the different load balancing schemes available.
#ifndef LOAD_BALANCING_SCHEME_H
#define LOAD_BALANCING_SCHEME_H

#include <algorithm>
#include <string>

enum class LbScheme {
  PACKET_SPRAY = 0,
  ECMP = 1,
  DRILL = 2,
  LETFLOW = 3,
  UNKNOWN = 4
};

static std::unordered_map<std::string, LbScheme> const lbSchemesMap = {
  {"packet_spray", LbScheme::PACKET_SPRAY},
  {"ecmp", LbScheme::ECMP},
  {"drill", LbScheme::DRILL},
  {"letflow", LbScheme::LETFLOW}
};

static std::string LbSchemeToString(LbScheme scheme) {
  switch (scheme) {
    case LbScheme::PACKET_SPRAY:
      return "packet_spray";
    case LbScheme::ECMP:
      return "ecmp";
    case LbScheme::DRILL:
      return "drill";
    case LbScheme::LETFLOW:
      return "letflow";
    case LbScheme::UNKNOWN:
      return "unknown";
  }
}

static LbScheme StringToLbScheme(std::string lb_scheme) {
  std::transform(lb_scheme.begin(), lb_scheme.end(), lb_scheme.begin(),
                 [](unsigned char c){ return std::tolower(c); });
  auto itr = lbSchemesMap.find(lb_scheme);
  if (itr != lbSchemesMap.end()) {
    return itr->second;
  }
  return LbScheme::UNKNOWN;
}

#endif  // LOAD_BALANCING_SCHEME_H
