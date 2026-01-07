cmake_minimum_required(VERSION 3.20)
project(vda5050_execution)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(vda5050_core REQUIRED)
find_package(vda5050_types REQUIRED)
find_package(vda5050_json_utils REQUIRED)

include(GNUInstallDirs)

file(GLOB_RECURSE srcs "src/vda5050_execution/*.cpp")
add_library(vda5050_execution ${srcs})

target_link_libraries(vda5050_execution
  PUBLIC
    vda5050_core::vda5050_core
    vda5050_types::vda5050_types
    vda5050_json_utils::vda5050_json_utils
)

target_include_directories(vda5050_execution
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  DIRECTORY include/vda5050_execution
  DESTINATION include
)

install(
  TARGETS vda5050_execution
  EXPORT export_vda5050_execution
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT export_vda5050_execution
  DESTINATION share/${PROJECT_NAME}/cmake
  NAMESPACE vda5050_execution::
)

ament_export_targets(export_vda5050_execution HAS_LIBRARY_TARGET)
ament_export_dependencies(
  vda5050_core
  vda5050_types
  vda5050_json_utils
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
