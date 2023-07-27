build_lib_example(
    NAME ipv4-letflow-routing-example
    SOURCE_FILES ipv4-letflow-routing-example.cc
    LIBRARIES_TO_LINK
      ${libcore}
      ${libinternet}
      ${libletflow-routing}
      ${libpoint-to-point}
)

