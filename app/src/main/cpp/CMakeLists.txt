cmake_minimum_required(VERSION 3.10.2)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义库
add_library(
        native-lib
        SHARED
        native-lib.cpp
        utils.cpp
)

# 包含Android日志库
find_library(
        log-lib
        log
)

# 链接库
target_link_libraries(
        native-lib
        android
        ${log-lib}
)
