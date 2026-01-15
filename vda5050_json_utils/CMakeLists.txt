cmake_minimum_required(VERSION 3.20)
project(vda5050_json_utils)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(vda5050_types REQUIRED)
find_package(nlohmann_json REQUIRED)

include(GNUInstallDirs)

option(ENABLE_ROS2 "Enable ROS 2 message support in JSON utilities" OFF)

if(ENABLE_ROS2)
  message(STATUS "Building vda5050_json_utils with ROS 2 support")
  find_package(vda5050_interfaces REQUIRED)
else()
  message(STATUS "Building vda5050_json_utils without ROS 2 support")
endif()

add_library(vda5050_json_utils INTERFACE)

target_link_libraries(vda5050_json_utils
  INTERFACE
    vda5050_types::vda5050_types
    nlohmann_json::nlohmann_json
)

target_include_directories(vda5050_json_utils
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(ENABLE_ROS2)
  target_link_libraries(vda5050_json_utils
    INTERFACE
      ${vda5050_interfaces_LIBRARIES}
  )

  target_include_directories(vda5050_json_utils
    INTERFACE
      ${vda5050_interfaces_INCLUDE_DIRS}
  )

  target_compile_definitions(vda5050_json_utils INTERFACE ENABLE_ROS2)
endif()

install(
  DIRECTORY include/vda5050_json_utils
  DESTINATION include
)

install(
  TARGETS vda5050_json_utils
  EXPORT export_vda5050_json_utils
)

ament_export_include_directories(include)
ament_export_targets(export_vda5050_json_utils HAS_LIBRARY_TARGET)
ament_export_dependencies(
  nlohmann_json
  vda5050_types
)

if(ENABLE_ROS2)
  ament_export_dependencies(vda5050_interfaces)
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()

  find_package(ament_cmake_cpplint REQUIRED)

  set(cpplint_filters
    "-whitespace/newline"
  )

  ament_cpplint(
    FILTERS "${cpplint_filters}"
  )

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

  find_package(ament_cmake_gtest REQUIRED)

  set(tests test/test_json_utils.cpp)
  if(ENABLE_ROS2)
    list(APPEND tests test/test_json_utils_ros2.cpp)
  endif()

  ament_add_gtest(json_utils_struct_test ${tests})

  target_include_directories(json_utils_struct_test
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(json_utils_struct_test
      vda5050_json_utils
  )
endif()

ament_package()
