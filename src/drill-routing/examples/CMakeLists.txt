build_lib_example(
    NAME ipv4-drill-routing-example
    SOURCE_FILES ipv4-drill-routing-example.cc
    LIBRARIES_TO_LINK
      ${libapplications}
      ${libcore}
      ${libdrill-routing}
      ${libflow-monitor}
      ${libinternet}
      ${libpoint-to-point}
)

