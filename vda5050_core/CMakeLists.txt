cmake_minimum_required(VERSION 3.8)
project(vda5050_core)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(fmt REQUIRED)
find_package(vda5050_types REQUIRED)

include(GNUInstallDirs)

add_library(mqtt_client src/vda5050_core/mqtt_client/paho_mqtt_client.cpp)
target_link_libraries(mqtt_client
  PRIVATE
    PahoMqttCpp::paho-mqttpp3
)
target_include_directories(mqtt_client
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(logger src/vda5050_core/logger/logger.cpp)
target_link_libraries(logger
  PRIVATE
    fmt::fmt
)
target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(client src/vda5050_core/state_manager/state_manager.cpp)
target_link_libraries(client
  vda5050_types::vda5050_types)
target_include_directories(client
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(vda5050_core INTERFACE)
target_link_libraries(vda5050_core
  INTERFACE
    mqtt_client
    logger
    client
)
target_include_directories(vda5050_core
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  DIRECTORY include/vda5050_core
  DESTINATION include
)

install(
  TARGETS
    mqtt_client
    logger
    client
    vda5050_core
  EXPORT export_vda5050_core
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT export_vda5050_core
  DESTINATION share/${PROJECT_NAME}/cmake
  NAMESPACE vda5050_core::
)

ament_export_targets(export_vda5050_core HAS_LIBRARY_TARGET)
ament_export_dependencies(PahoMqttCpp fmt)

if(BUILD_TESTING)
  include(../cmake/vda5050_lint_common.cmake)
  vda5050_lint_common()

  find_package(ament_cmake_gmock REQUIRED)
  ament_add_gmock(test_mqtt_client
    test/unit/mqtt_client/test_mqtt_client_interface.cpp
    test/unit/logger/test_logger.cpp
    test/unit/logger/test_default_logger.cpp
    test/unit/state_manager/test_state_manager.cpp
    test/integration/mqtt_client/test_paho_mqtt_client.cpp
  )
  target_link_libraries(test_mqtt_client mqtt_client logger client)
endif()

ament_package()
