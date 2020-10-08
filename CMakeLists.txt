# Copyright 2016 Alex Silva Torres
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8)
project(shpp C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(cmake/SettiUtils.cmake)
include(cmake/sanitize-helpers.cmake)
include(CheckCXXCompilerFlag)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_14)

include(cmake/CompilerWarnings.cmake)

if(MSVC)
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

SHPP_OPTION(BUILD_UNIT_TESTS "Build unit tests" OFF)
SHPP_OPTION(COV_TESTS "Coverage tests" OFF)

if (COV_TESTS)
  # put on debug
  SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
  set(CMAKE_BUILD_TYPE "Debug")
endif()
#
# Boost support
#
find_package(Boost COMPONENTS system filesystem locale REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
find_package(Threads REQUIRED)

if (BUILD_UNIT_TESTS)
  enable_testing()
  include(googletest)
  include_directories(${googletest_INCLUDE_DIRS}
                      ${googlemock_INCLUDE_DIRS})
endif ()

include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/glob-cpp/include)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
add_subdirectory(shell)
