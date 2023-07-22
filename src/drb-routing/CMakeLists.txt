check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/drb-routing-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME drb-routing
    SOURCE_FILES model/drb-routing.cc
                 helper/drb-routing-helper.cc
    HEADER_FILES model/drb-routing.h
                 helper/drb-routing-helper.h
    LIBRARIES_TO_LINK ${libcore}
    TEST_SOURCES test/drb-routing-test-suite.cc
                 ${examples_as_tests_sources}
)

