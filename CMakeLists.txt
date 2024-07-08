# 指定最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(Main LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 为不同构建类型设置库文件的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pdb)

# 设置可执行文件的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 添加头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/inc)

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/inc/*.cpp")

# 设置可执行文件的名称和源文件列表
add_executable(Main test17.cpp ${SOURCES})

# 可选：如果需要链接库，可以使用 target_link_libraries
# target_link_libraries(Main PRIVATE some_library)

