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
# Run common linters for VDA5050
#

function(vda5050_lint_common)
  find_program(cppcheck_exe cppcheck)
  if(cppcheck_exe)
    add_test(
      NAME cppcheck
      COMMAND ${cppcheck_exe}
                --enable=all
                --error-exitcode=1
                --verbose
                --std=c++17
                -I ${CMAKE_CURRENT_SOURCE_DIR}/include
                --suppress=missingIncludeSystem
                --suppress=unusedFunction
                --suppress=useStlAlgorithm
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/test
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endif()

  find_program(clang_format_exe clang-format)
  if(clang_format_exe)
    file(GLOB_RECURSE srcs
      ${CMAKE_SOURCE_DIR}/src/*.cpp
      ${CMAKE_SOURCE_DIR}/include/*.hpp
      ${CMAKE_SOURCE_DIR}/test.*.cpp
    )

    add_test(
      NAME clang_format
      COMMAND ${clang_format_exe}
              --dry-run
              --Werror
              ${srcs}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()
endfunction()
