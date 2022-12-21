cmake_minimum_required(VERSION 3.24)
project(cppbtbl)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_COLOR_DIAGNOSTICS ON)

find_package(sdbus-c++ REQUIRED)

include_directories(${SOURCE_DIR})

file(GLOB COMPILE_FILES ${SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_NAME} ${COMPILE_FILES})

target_link_libraries(cppbtbl PRIVATE SDBusCpp::sdbus-c++)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Wno-c++98-compat -Wfloat-conversion -Wno-unused-parameter -Wimplicit-fallthrough -Wconversion)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
