# The listed source files here are only used for verification of the compilation
# of some of the library headers. They verify if the library header includes
# all of the needed headers to be compiled in isolation.
add_library(par OBJECT
    src/cfreaks_rcu.cpp
    src/dpdk_rcu.cpp
    src/vyukov_mpmc_queue.cpp
)

target_include_directories(par
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(par
    PRIVATE flags_options
)
