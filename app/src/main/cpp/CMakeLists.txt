cmake_minimum_required(VERSION 3.22.1)

project("license_guard")

add_library(
        license_guard
        SHARED
        native_license.cpp
)

find_library(
        log-lib
        log
)

target_link_libraries(
        license_guard
        android
        ${log-lib}
)
