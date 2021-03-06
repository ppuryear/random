# Copyright 2014 Philip Puryear
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
project(random C)

add_subdirectory(simple-strconv EXCLUDE_FROM_ALL)
include_directories(simple-strconv/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
# By default, CMake passes -rdynamic to the linker on Linux. This silently
# breaks LTO when linking with a static library, so remove the flag.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
add_executable(random random.c)
target_link_libraries(random simple_strconv gmp)

add_custom_command(
    OUTPUT random.1
    COMMAND a2x -D . -f manpage ${CMAKE_CURRENT_SOURCE_DIR}/manual.asciidoc
    DEPENDS manual.asciidoc
    COMMENT "Building man page random(1)"
    VERBATIM
)
add_custom_target(man ALL DEPENDS random.1)

install(TARGETS random DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/random.1 DESTINATION share/man/man1)
