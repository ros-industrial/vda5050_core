cmake_minimum_required(VERSION 3.20)
project(vda5050_json_utils)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(vda5050_types REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(vda5050_msgs REQUIRED)

include(GNUInstallDirs)

option(ENABLE_ROS2 "Enable ROS 2 message support in JSON utilities" OFF)

if(ENABLE_ROS2)
  message(STATUS "Building vda5050_json_utils with ROS 2 support")
  find_package(vda5050_msgs REQUIRED)
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
      ${vda5050_msgs_LIBRARIES}
  )

  target_include_directories(vda5050_json_utils
    INTERFACE
      ${vda5050_msgs_INCLUDE_DIRS}
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
  vda5050_msgs
)

if(ENABLE_ROS2)
  ament_export_dependencies(vda5050_msgs)
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

  find_package(ament_cmake_clang_format REQUIRED)
  ament_clang_format(
    CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format"
  )

  find_package(ament_cmake_copyright REQUIRED)
  ament_copyright()

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
