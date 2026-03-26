cmake_minimum_required(VERSION 3.10.2)
project("license-core")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden -s -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fmerge-all-constants -fno-ident -fno-stack-protector")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/license.ld -Wl,--strip-all -Wl,--exclude-libs,ALL -Wl,--build-id=none")

add_library(license-core SHARED license_core.c)
find_library(log-lib log)
target_link_libraries(license-core ${log-lib})
