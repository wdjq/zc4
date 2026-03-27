cmake_minimum_required(VERSION 3.10.2)
project(license_core)

add_library(license_core SHARED src/license_core.c)

find_library(log-lib log)

target_link_libraries(license_core
    ${log-lib}
    -static-libgcc
)

target_compile_options(license_core PRIVATE
    -O3
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-stack-protector
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -fno-ident
    -fomit-frame-pointer
    -ffunction-sections
    -fdata-sections
    -flto
    -Wall
)

target_link_options(license_core PRIVATE
    -Wl,--strip-all
    -Wl,--exclude-libs,ALL
    -Wl,--gc-sections
    -s
)

set_target_properties(license_core PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    POSITION_INDEPENDENT_CODE YES
)
