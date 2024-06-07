cmake_minimum_required(VERSION 3.21)
project(gtest_gmock)

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)

# 设置库文件目录
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 添加源文件
add_executable(gtest_gmock src/main.cpp src/parse.cpp src/area.cpp src/brain.cpp src/parserBrain.cpp)

# 链接 GoogleTest 库和 pthread 库
target_link_libraries(gtest_gmock libgtest.a libgtest_main.a pthread)