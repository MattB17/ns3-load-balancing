build_example(
  NAME small-scale-lb
  SOURCE_FILES small-scale-lb.cc
               cdf.cc
  HEADER_FILES cdf.h
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libinternet}
    ${libapplications}
)

build_example(
  NAME flow-level-ecmp
  SOURCE_FILES flow-level-ecmp.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libinternet}
    ${libapplications}
    ${libflow-monitor}
)