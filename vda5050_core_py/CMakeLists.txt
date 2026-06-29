cmake_minimum_required(VERSION 3.8)
project(vda5050_core_py)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
find_package(vda5050_core REQUIRED)

# Build the compiled extension. The module name `_core` must match
# PYBIND11_MODULE(_core, m) in src/bindings.cpp — the Python package's
# __init__.py imports from `._core`.
pybind11_add_module(_core
  SHARED
    src/bindings.cpp
)

target_link_libraries(_core
  PRIVATE
    pybind11::pybind11
    vda5050_core::adapter
    vda5050_core::execution
    vda5050_core::transport
    vda5050_core::types
)

# Install the Python source package (vda5050_core_py/__init__.py, py.typed).
ament_python_install_package(${PROJECT_NAME})

# Install the compiled extension into the same Python package directory so
# `from ._core import ...` inside __init__.py resolves at runtime.
install(TARGETS _core
  LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}/${PROJECT_NAME}"
)

install(DIRECTORY autoxing-l300
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
  PATTERN "__pycache__" EXCLUDE
)

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(test_adapter_smoke test/test_adapter_smoke.py
    TIMEOUT 60
  )
endif()

ament_package()
