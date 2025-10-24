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

  find_package(ament_cmake_clang_format REQUIRED)
  ament_clang_format(
    CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format"
  )

  find_package(ament_cmake_copyright REQUIRED)
  ament_copyright()
endif()

ament_package()
