# Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
# Advanced Remanufacturing and Technology Centre
# A*STAR Research Entities (Co. Registration No. 199702110H)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Run common ament linters for VDA5050
#

function(vda5050_ament_lint_common)
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()

  find_package(ament_cmake_cpplint REQUIRED)
  set(cpplint_filters
    "-whitespace/newline"
    "-build/header_guard"
  )
  ament_cpplint(FILTERS "${cpplint_filters}")

  # Only run ament_clang_format for 16.0.0 and higher
  find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
  if(CLANG_FORMAT_EXECUTABLE)
    execute_process(
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Parse the version number from the output
    # The output typically looks something like: "<platform> clang-format version 17.0.6-<platform-version>"
    string(REGEX REPLACE ".*version \([0-9.]\+\).*" "\\1" CLANG_FORMAT_VERSION "${CLANG_FORMAT_VERSION_OUTPUT}")

    message(STATUS "clang-format version: ${CLANG_FORMAT_VERSION}")
    if(CLANG_FORMAT_VERSION VERSION_GREATER_EQUAL "16.0.0")
      find_package(ament_cmake_clang_format REQUIRED)
      ament_clang_format(
        CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format"
      )
    endif()
  endif()

  # Disable copyright check for older ament_copyright versions
  find_package(ament_cmake_copyright 0.13.3 QUIET)
  if(ament_cmake_copyright_FOUND)
    set(copyright_excludes install)
    ament_copyright(include src test)
  endif()
endfunction()
