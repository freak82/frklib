add_library(tests_main OBJECT main.cpp)

target_link_libraries(tests_main
    PUBLIC doctest::doctest flags_options tests_options
)
