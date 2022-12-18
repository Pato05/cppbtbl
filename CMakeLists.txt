cmake_minimum_required(VERSION 3.24)
project(cppbtbl)

find_package(sdbus-c++ REQUIRED)

add_executable(cppbtbl cppbtbl.cpp)
target_link_libraries(cppbtbl PRIVATE SDBusCpp::sdbus-c++)

install(
    TARGETS cppbtbl
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
