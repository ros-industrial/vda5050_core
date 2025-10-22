cmake_minimum_required(VERSION 3.8)
project(vda5050_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(fmt REQUIRED)

include(GNUInstallDirs)

add_library(mqtt_client src/vda5050_core/mqtt_client/paho_mqtt_client.cpp)
target_link_libraries(mqtt_client PRIVATE PahoMqttCpp::paho-mqttpp3)
target_include_directories(mqtt_client
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(logger src/vda5050_core/logger/logger.cpp)
target_link_libraries(logger PRIVATE fmt::fmt)
target_include_directories(logger
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(state_manager src/vda5050_core/state_manager/state_manager.cpp)
target_include_directories(state_manager
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(vda5050_core INTERFACE)
target_link_libraries(vda5050_core INTERFACE mqtt_client logger state_manager)
target_include_directories(vda5050_core
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(order_graph_element src/vda5050_core/order_execution/order_graph_element.cpp)
target_include_directories(order_graph_element
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(edge src/vda5050_core/order_execution/edge.cpp)
target_include_directories(edge
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(edge order_graph_element)

add_library(node src/vda5050_core/order_execution/node.cpp)
target_include_directories(node
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(node order_graph_element)

add_library(order_graph_validator src/vda5050_core/order_execution/order_graph_validator.cpp)
target_include_directories(order_graph_validator
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(order_graph_validator edge node)

add_library(order src/vda5050_core/order_execution/order.cpp)
target_include_directories(order
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(order edge node)

add_library(order_manager src/vda5050_core/order_execution/order_manager.cpp)
target_include_directories(order_manager
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(order_manager order order_graph_validator)

target_link_libraries(vda5050_core
  INTERFACE order_graph_element edge node order_graph_validator order order_manager)

install(DIRECTORY include/vda5050_core DESTINATION include)

install(TARGETS
  mqtt_client
  logger
  state_manager
  order_graph_element
  edge
  node
  order_graph_validator
  order
  order_manager
  vda5050_core
  EXPORT export_vda5050_core
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT export_vda5050_core
  DESTINATION share/${PROJECT_NAME}/cmake
  NAMESPACE vda5050_core::)

ament_export_targets(export_vda5050_core HAS_LIBRARY_TARGET)
ament_export_dependencies(PahoMqttCpp)

if(BUILD_TESTING)
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()
  find_package(ament_cmake_cpplint REQUIRED)
  ament_cpplint()
  find_package(ament_cmake_clang_format REQUIRED)
  ament_clang_format(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format")
  find_package(ament_cmake_copyright REQUIRED)
  ament_copyright()

  find_package(ament_cmake_gmock REQUIRED)
  ament_add_gmock(test_mqtt_client
    test/unit/mqtt_client/test_mqtt_client_interface.cpp
    test/unit/logger/test_logger.cpp
    test/unit/logger/test_default_logger.cpp
    test/unit/state_manager/test_state_manager.cpp
    test/integration/mqtt_client/test_paho_mqtt_client.cpp)
  target_link_libraries(test_mqtt_client mqtt_client logger state_manager)

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(${PROJECT_NAME}_order_graph_validator_test
    test/unit/order_manager/test_order_graph_validator.cpp
    src/vda5050_core/order_execution/edge.cpp
    src/vda5050_core/order_execution/node.cpp
    src/vda5050_core/order_execution/order_graph_validator.cpp
    src/vda5050_core/order_execution/order_graph_element.cpp)
  target_include_directories(${PROJECT_NAME}_order_graph_validator_test
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  ament_add_gmock(${PROJECT_NAME}_test_OrderManager
    test/unit/order_manager/test_order_manager.cpp)
  target_include_directories(${PROJECT_NAME}_test_OrderManager
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  target_link_libraries(${PROJECT_NAME}_test_OrderManager
    order_graph_validator edge node order_graph_element order_manager)
endif()

ament_package()
