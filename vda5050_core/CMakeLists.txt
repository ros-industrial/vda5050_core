cmake_minimum_required(VERSION 3.8)
project(vda5050_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

include(GNUInstallDirs)

add_library(order_graph_element src/vda5050_core/order_execution/OrderGraphElement.cpp)
target_include_directories(order_graph_element
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(edge src/vda5050_core/order_execution/Edge.cpp)
target_include_directories(edge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge order_graph_element)

add_library(node src/vda5050_core/order_execution/Node.cpp)
target_include_directories(node
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(node order_graph_element)

add_library(order_graph_validator src/vda5050_core/order_execution/OrderGraphValidator.cpp)
target_include_directories(order_graph_validator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(order_graph_validator edge node)

add_library(order src/vda5050_core/order_execution/Order.cpp)
target_include_directories(order
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(order edge node)

add_library(order_manager src/vda5050_core/order_execution/OrderManager.cpp)
target_include_directories(order_manager
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(order_manager order order_graph_validator)

install(
    DIRECTORY include/vda5050_core
    DESTINATION include
)

install(
    TARGETS
        order_graph_element
        edge
        node
        order_graph_validator
        order
        order_manager
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

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(
    ${PROJECT_NAME}_order_graph_validator_test test/unit/test_OrderGraphValidator.cpp
    src/vda5050_core/order_execution/Edge.cpp
    src/vda5050_core/order_execution/Node.cpp
    src/vda5050_core/order_execution/OrderGraphValidator.cpp
    src/vda5050_core/order_execution/OrderGraphElement.cpp
    )
  target_include_directories(${PROJECT_NAME}_order_graph_validator_test
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  find_package(ament_cmake_gmock REQUIRED)
  ament_add_gmock(${PROJECT_NAME}_test_OrderManager
    test/unit/test_OrderManager.cpp
  )
  target_include_directories(${PROJECT_NAME}_test_OrderManager
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_link_libraries(${PROJECT_NAME}_test_OrderManager
    order_graph_validator
    edge
    node
    order_graph_element
    order_manager
  )
  
endif()

ament_package()
