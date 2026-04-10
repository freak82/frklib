
add_library(flags_options INTERFACE)

target_compile_options(flags_options
    INTERFACE
        -std=c++2c
        -fno-rtti
        -march=native
        -Wall 
        -Wextra 
        -Wpedantic 
        -Werror
        -Winit-self
        -Wuseless-cast
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Warith-conversion
)

#target_link_options(flags_options
#    INTERFACE
#        -static-libstdc++ -static-libgcc
#)
