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
find_package(vda5050_msgs REQUIRED)
find_package(fmt REQUIRED)

add_library(vda5050_master
  src/vda5050_master/master.cpp
  src/vda5050_master/communication/mqtt.cpp
  src/vda5050_master/communication/ros2.cpp
)

ament_target_dependencies(vda5050_master
  vda5050_core
  vda5050_msgs
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

  # find_package(ament_cmake_gtest REQUIRED)
  # ament_add_gtest(test_mqtt_client
  #   test/integration/heartbeat.cpp
  #   test/integration/mqtt_communications.cpp
  # )

  find_package(ament_cmake_gmock REQUIRED)
  ament_add_gmock(test_communications
    test/communication/mqtt/communications.cpp
    test/communication/ros2/communications.cpp
    test/communication/vda5050/vda5050_mqtt.cpp
    test/integration/heartbeat.cpp
  )

  target_link_libraries(test_communications vda5050_master)

endif()

ament_package()
