# Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
# Advanced Remanufacturing and Technology Centre
# A*STAR Research Entities (Co. Registration No. 199702110H)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Configure internal and export alias for library and interface targets
#
# Create an alias of NAMESPACE::EXPORT_NAME both for internal and export
#
# :param TARGET: library or interface target name
# :type TARGET: string
# :param NAMESPACE: new namespace for the internal target
# :type NAMESPACE: string
# :param EXPORT_NAME: alias for the target name
# :type EXPORT_NAME: string

macro(vda5050_configure_target_alias)
  set(_one_value_args TARGET NAMESPACE EXPORT_NAME)
  cmake_parse_arguments(_ARG "" "${_one_value_args}" "" ${ARGN})

  if(NOT DEFINED _ARG_TARGET)
    message(FATAL_ERROR
      "vda5050_configure_target_alias() must be called with TARGET defined!")
  endif()

  # Allow optionally overriding default namespace and alias name
  set(_VDA5050_TARGET_NAMESPACE ${_ARG_NAMESPACE})
  if(NOT DEFINED _VDA5050_TARGET_NAMESPACE)
    set(_VDA5050_TARGET_NAMESPACE "${PROJECT_NAME}::")
  endif()

  set(_VDA5050_TARGET_EXPORT_NAME ${_ARG_EXPORT_NAME})
  if(NOT DEFINED _VDA5050_TARGET_EXPORT_NAME)
    set(_VDA5050_TARGET_EXPORT_NAME "${_ARG_TARGET}")
  endif()

  # Set internal alias
  add_library(
    "${_VDA5050_TARGET_NAMESPACE}${_VDA5050_TARGET_EXPORT_NAME}"
    ALIAS ${_ARG_TARGET}
  )

  # set custom export name for target
  set_property(TARGET
    ${_ARG_TARGET}
    PROPERTY EXPORT_NAME ${_VDA5050_TARGET_EXPORT_NAME}
  )
endmacro()
