
add_subdirectory(main)

add_executable(tests
    net/ip_addr.cpp
    net/ip_addr_port.cpp
    $<TARGET_OBJECTS:tests_main>
)

target_include_directories(tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tests
    PRIVATE
        doctest::doctest
        flags_options
        tests_options
)

target_precompile_headers(tests
    PRIVATE precompiled.hpp
)

add_test(
    NAME frklib_tests
    COMMAND tests --force-colors
)
