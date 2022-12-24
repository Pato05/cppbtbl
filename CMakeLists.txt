cmake_minimum_required(VERSION 3.24)

option(NO_CLANG "Don't prefer clang for compilation" OFF)

if (NOT NO_CLANG)
  # Prefer clang++ for compilation if available
  find_program(CLANG clang)
  find_program(CLANGXX clang++)

  if (CLANG)
    set(CMAKE_C_COMPILER ${CLANG})
  endif()

  if (CLANGXX)
    set(CMAKE_CXX_COMPILER ${CLANGXX})
  endif()
endif()

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined)
  target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
endif()

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
