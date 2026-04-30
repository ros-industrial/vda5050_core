cmake_minimum_required(VERSION 3.8)
project(vda5050_master)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(vda5050_core REQUIRED)
find_package(vda5050_execution REQUIRED)
find_package(vda5050_types REQUIRED)
find_package(vda5050_json_utils REQUIRED)
find_package(fmt REQUIRED)

add_library(vda5050_master
  src/agv/agv.cpp
  src/vda5050_master/master.cpp
  src/vda5050_master/communication/heartbeat.cpp
)

ament_target_dependencies(vda5050_master
  vda5050_core
  vda5050_execution
  vda5050_types
  vda5050_json_utils
  fmt
  rclcpp
  std_msgs
)

target_include_directories(vda5050_master PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_features(vda5050_master PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17


install(TARGETS vda5050_master
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  include(../cmake/vda5050_lint_common.cmake)
  vda5050_lint_common()

  # find_package(ament_cmake_gtest REQUIRED)
  # ament_add_gtest(test_mqtt_client
  #   test/integration/heartbeat.cpp
  #   test/integration/mqtt_communications.cpp
  # )

  find_package(ament_cmake_gmock REQUIRED)

  # Heartbeat unit tests
  ament_add_gmock(test_heartbeat
    test/integration/heartbeat.cpp
  )
  target_link_libraries(test_heartbeat vda5050_master)

  # AGV unit tests
  ament_add_gmock(test_agv
    test/agv/agv_connection_state_test.cpp
    test/agv/agv_operational_state_test.cpp
    test/agv/agv_lifecycle_test.cpp
  )
  target_link_libraries(test_agv vda5050_master)

  # Master unit tests (uses real MQTT broker, skipped if unavailable)
  ament_add_gmock(test_master
    test/master/master_logic_test.cpp
    test/master/master_mqtt_test.cpp
  )
  target_link_libraries(test_master vda5050_master)

  # Master teardown integration test (uses gmock MqttClient, no broker)
  ament_add_gmock(test_master_teardown
    test/master/teardown_test.cpp
  )
  target_link_libraries(test_master_teardown vda5050_master)

endif()

ament_package()
