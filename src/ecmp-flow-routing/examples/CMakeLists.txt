build_lib_example(
    NAME ipv4-ecmp-flow-routing-example
    SOURCE_FILES ipv4-ecmp-flow-routing-example.cc
    LIBRARIES_TO_LINK
      ${libapplications}
      ${libcore}
      ${libecmp-flow-routing}
      ${libflow-monitor}
      ${libinternet}
      ${libpoint-to-point}
)

