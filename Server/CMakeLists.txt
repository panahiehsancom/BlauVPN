project(ServerBlauVPN)

cmake_minimum_required (VERSION 2.8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT ../lib/boost/ CACHE PATH "Installation prefix of the boost library")
endif (NOT DEFINED BOOST_ROOT)

find_package(Boost COMPONENTS system thread REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_executable(${CMAKE_PROJECT_NAME} main.cpp tcpserver.h CommonFunction.h CommonFunction.cpp tcpserver.cpp IPeripheralCommunication.h tundevicecommunication.h tundevicecommunication.cpp )

target_link_libraries(${CMAKE_PROJECT_NAME} ${Boost_LIBRARIES})
