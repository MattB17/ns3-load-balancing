build_example(
  NAME fat-tree-2-tier
  SOURCE_FILES fat-tree-2-tier.cc
               cdf.cc
  HEADER_FILES cdf.h
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libinternet}
    ${libapplications}
    ${libflow-monitor}
)