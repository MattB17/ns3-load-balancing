build_example(
  NAME fat-tree-2-tier
  SOURCE_FILES cdf.cc
               fat-tree-2-tier.cc
  HEADER_FILES cdf.h
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libinternet}
    ${libapplications}
    ${libflow-monitor}
)

build_example(
  NAME simple-parallel-paths
  SOURCE_FILES cdf.cc
               simple-parallel-paths.cc
  HEADER_FILES cdf.h
               load-balancing-scheme.h
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libdrill-routing}
    ${libflow-monitor}
    ${libinternet}
    ${libpoint-to-point}
)
