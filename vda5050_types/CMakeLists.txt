cmake_minimum_required(VERSION 3.20)
project(vda5050_types)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

include(GNUInstallDirs)

add_library(vda5050_types INTERFACE)

target_include_directories(vda5050_types
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  DIRECTORY include/vda5050_types
  DESTINATION include
)

install(
  TARGETS vda5050_types
  EXPORT export_vda5050_types
)

ament_export_include_directories(include)
ament_export_targets(export_vda5050_types HAS_LIBRARY_TARGET)

if(BUILD_TESTING)
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()

  find_package(ament_cmake_cpplint REQUIRED)
  ament_cpplint()

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
    ament_copyright()
  endif()
endif()

ament_package()
