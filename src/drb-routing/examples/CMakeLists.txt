build_lib_example(
    NAME ipv4-drb-routing-example
    SOURCE_FILES ipv4-drb-routing-example.cc
    LIBRARIES_TO_LINK
      ${libapplications}
      ${libcore}
      ${libdrb-routing}
      ${libflow-monitor}
      ${libinternet}
      ${libpoint-to-point}
)

