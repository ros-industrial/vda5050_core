cmake_minimum_required(VERSION 3.8)
project(vda5050_core)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake QUIET)
find_package(PahoMqttCpp REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

include(GNUInstallDirs)

# Types
add_library(types INTERFACE)
add_library(${PROJECT_NAME}::types ALIAS types)
target_include_directories(types
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# JSON Utils
option(ENABLE_ROS2 "Enable ROS 2 message support in JSON utilities" OFF)
if(ament_cmake_FOUND AND ENABLE_ROS2)
  message(STATUS "Building vda5050_json_utils with ROS 2 support")
  find_package(vda5050_interfaces REQUIRED)
else()
  message(STATUS "Building vda5050_json_utils without ROS 2 support")
endif()

add_library(json_utils INTERFACE)
add_library(${PROJECT_NAME}::json_utils ALIAS json_utils)
target_link_libraries(json_utils
  INTERFACE
    types
    nlohmann_json::nlohmann_json
)
target_include_directories(json_utils
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(ament_cmake_FOUND AND ENABLE_ROS2)
  target_link_libraries(json_utils
    INTERFACE
      ${vda5050_interfaces_LIBRARIES}
  )
  target_include_directories(json_utils
    INTERFACE
      ${vda5050_interfaces_INCLUDE_DIRS}
  )
  target_compile_definitions(json_utils INTERFACE ENABLE_ROS2)
endif()

# Logger
add_library(logger SHARED src/logger/logger.cpp)
add_library(${PROJECT_NAME}::logger ALIAS logger)
target_link_libraries(logger PUBLIC fmt::fmt)
target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Errors
add_library(errors INTERFACE)
add_library(${PROJECT_NAME}::errors ALIAS errors)
target_include_directories(errors
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Order Utils
add_library(order_utils SHARED src/order_utils/order_graph_validator.cpp)
add_library(${PROJECT_NAME}::order_utils ALIAS order_utils)
target_link_libraries(order_utils PUBLIC types errors)
target_include_directories(order_utils
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Transport
add_library(transport src/transport/paho_mqtt_client.cpp)
add_library(${PROJECT_NAME}::transport ALIAS transport)
target_link_libraries(transport
  PUBLIC
    PahoMqttCpp::paho-mqttpp3
    logger
)
target_include_directories(transport
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Execution
file(GLOB_RECURSE execution_srcs "src/execution/*.cpp")
add_library(execution ${execution_srcs})
add_library(${PROJECT_NAME}::execution ALIAS execution)
target_link_libraries(execution
  PUBLIC
    types
    json_utils
    transport
)
target_include_directories(transport
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Master
file(GLOB_RECURSE master_srcs "src/master/*.cpp")
add_library(master ${master_srcs})
add_library(${PROJECT_NAME}::master ALIAS master)
target_link_libraries(master
  PUBLIC
    types
    json_utils
    transport
    execution
)
target_include_directories(transport
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


set(lib_targets
  types
  json_utils
  logger
  errors
  order_utils
  transport
  execution
  master
)

install(
  DIRECTORY include/${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
  TARGETS ${lib_targets}
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT export_${PROJECT_NAME}
  DESTINATION share/${PROJECT_NAME}/cmake
  NAMESPACE ${PROJECT_NAME}::
)

if(ament_cmake_FOUND)
  ament_export_targets(export_vda5050_core HAS_LIBRARY_TARGET)
  ament_export_dependencies(PahoMqttCpp fmt nlohmann_json)

  if(ENABLE_ROS2)
    ament_export_dependencies(vda5050_interfaces)
  endif()
endif()

option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
  if(ament_cmake_FOUND)
    include(cmake/vda5050_ament_lint_common.cmake)
    vda5050_ament_lint_common()

    find_package(ament_cmake_gmock REQUIRED)

    set(json_tests test/json_utils/test_json_utils.cpp)
    if(ENABLE_ROS2)
      list(APPEND json_tests test/json_utils/test_json_utils_ros2.cpp)
    endif()
    ament_add_gmock(test_json_utils ${json_tests})
    target_link_libraries(test_json_utils vda5050_core::json_utils)

    ament_add_gmock(test_order_utils
      test/order_utils/test_unit_order_graph_validator.cpp)
    target_link_libraries(test_order_utils vda5050_core::order_utils)

    ament_add_gmock(test_logger
      test/logger/test_unit_default_logger.cpp
      test/logger/test_unit_logger.cpp
    )
    target_link_libraries(test_logger vda5050_core::logger)

    ament_add_gmock(test_transport
      test/transport/test_unit_mqtt_client_interface.cpp
      test/transport/test_integration_paho_mqtt_client.cpp
    )
    target_link_libraries(test_transport vda5050_core::transport)

    ament_add_gmock(test_execution
      test/execution/test_engine.cpp
      test/execution/test_event_queue.cpp
      test/execution/test_handler.cpp
      test/execution/test_protocol_adapter.cpp
      test/execution/test_provider.cpp
    )
    target_link_libraries(test_execution vda5050_core::execution)

    ament_add_gmock(test_master
      test/master/test_integration_heartbeat.cpp
      test/master/test_integration_master_logic.cpp
      test/master/test_integration_master_mqtt.cpp
      test/master/test_integration_master_teardown.cpp
      test/master/test_unit_agv.cpp
    )
    target_link_libraries(test_master vda5050_core::master)
  else()
    enable_testing()

    include(cmake/vda5050_lint_common.cmake)
    vda5050_lint_common()

    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test_order_utils test/order_utils/test_unit_order_graph_validator.cpp)
    target_link_libraries(test_order_utils
      GTest::gtest_main
      ${PROJECT_NAME}::order_utils
    )

    add_executable(test_json_utils test/json_utils/test_json_utils.cpp)
    target_link_libraries(test_json_utils
      GTest::gtest_main
      ${PROJECT_NAME}::json_utils
    )

    add_executable(test_logger
      test/logger/test_unit_logger.cpp
      test/logger/test_unit_default_logger.cpp
    )
    target_link_libraries(test_logger
      GTest::gtest_main
      ${PROJECT_NAME}::logger
    )

    add_executable(test_transport
      test/transport/test_unit_mqtt_client_interface.cpp
      test/transport/test_integration_paho_mqtt_client.cpp
    )
    target_link_libraries(test_transport
      GTest::gmock_main
      ${PROJECT_NAME}::transport
    )

    add_executable(test_execution
      test/execution/test_engine.cpp
      test/execution/test_event_queue.cpp
      test/execution/test_handler.cpp
      test/execution/test_protocol_adapter.cpp
      test/execution/test_provider.cpp
    )
    target_link_libraries(test_execution
      GTest::gmock_main
      ${PROJECT_NAME}::execution
    )

    add_executable(test_master
      test/master/test_integration_heartbeat.cpp
      test/master/test_integration_master_logic.cpp
      test/master/test_integration_master_mqtt.cpp
      test/master/test_integration_master_teardown.cpp
      test/master/test_unit_agv.cpp
    )
    target_link_libraries(test_master
      GTest::gmock_main
      ${PROJECT_NAME}::master
    )

    set(test_execs
      test_order_utils
      test_json_utils
      test_logger
      test_transport
      test_execution
      test_master
    )
    foreach(exec ${test_execs})
      gtest_discover_tests(${exec})
    endforeach()
  endif()
endif()

if(ament_cmake_FOUND)
  ament_package()
endif()
