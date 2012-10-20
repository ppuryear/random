# Copyright 2012 Philip Puryear
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

cmake_minimum_required(VERSION 2.6)
project(rand C)

add_subdirectory(simple-strconv)
include_directories(simple-strconv/include)
find_library(LIBGMP gmp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
add_executable(rand rand.c)
target_link_libraries(rand simple_strconv gmp)

install(TARGETS rand DESTINATION bin)
