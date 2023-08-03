check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/ipv4-ecmp-flow-routing-examples-test-suite.cc
        )
endif()

set(source_files
  model/ipv4-ecmp-flow-routing.cc
  helper/ipv4-ecmp-flow-routing-helper.cc
)

set (header_files
  model/ipv4-ecmp-flow-routing.h
  helper/ipv4-ecmp-flow-routing-helper.h
)

build_lib(
    LIBNAME ecmp-flow-routing
    SOURCE_FILES ${source_files}
    HEADER_FILES ${header_files}
    LIBRARIES_TO_LINK ${libcore}
    TEST_SOURCES test/ipv4-ecmp-flow-routing-test-suite.cc
                 ${examples_as_tests_sources}
)

