# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("engine")

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRC_BASIC_NETWORK "src/basicnetwork/*.cpp" "src/basicnetwork/*.h")
file(GLOB_RECURSE SRC_COMMON "src/common/*.cpp" "src/common/*.h")
file(GLOB_RECURSE SRC_CONFIG "src/config/*.cpp" "src/config/*.h")
file(GLOB_RECURSE SRC_FASTDB "src/fastdb/*.cpp" "src/fastdb/*.h")
file(GLOB_RECURSE SRC_GAME_SERVER "src/gameserver/*.cpp" "src/gameserver/*.h")
file(GLOB_RECURSE SRC_INCLUDE "src/include/*.cpp" "src/include/*.h")
file(GLOB_RECURSE SRC_LOG "src/log/*.cpp" "src/log/*.h")
file(GLOB_RECURSE SRC_NETWORK "src/network/*.cpp" "src/network/*.h")
file(GLOB_RECURSE SRC_RMI "src/rmi/*.cpp" "src/rmi/*.h")
file(GLOB_RECURSE SRC_UTILITY "src/utility/*.cpp" "src/utility/*.h")

# 包含子项目。
add_subdirectory ("src/basicnetwork")
add_subdirectory ("src/common")
add_subdirectory ("src/config")
add_subdirectory ("src/fastdb")
add_subdirectory ("src/gameserver")
add_subdirectory ("src/log")
add_subdirectory ("src/network")
add_subdirectory ("src/rmi")
add_subdirectory ("src/utility")
