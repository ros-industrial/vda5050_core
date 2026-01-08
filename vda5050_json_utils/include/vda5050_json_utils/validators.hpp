/*
 * Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
 * Advanced Remanufacturing and Technology Centre
 * A*STAR Research Entities (Co. Registration No. 199702110H)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VDA5050_JSON_UTILS__VALIDATORS_HPP_
#define VDA5050_JSON_UTILS__VALIDATORS_HPP_

#include <iostream>
#include <string>
#include <limits>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include "vda5050_json_utils/schemas.hpp"

constexpr const char* ISO8601_FORMAT = "%Y-%m-%dT%H:%M:%S";

/// \brief Utility function to check that a given string is in ISO8601 format
///
/// \param value The string to be checked
///
/// \return True if the given string follows the format
bool is_in_ISO8601_format(const std::string& value)
{
  std::tm t = {};
  char sep;
  int millisec = 0;

  std::istringstream ss(value);

  ss >> std::get_time(&t, ISO8601_FORMAT);
  if (ss.fail())
  {
    return false;
  }

  ss >> sep;
  if (ss.fail() || sep != '.')
  {
    return false;
  }

  ss >> millisec;
  if (ss.fail())
  {
    return false;
  }
  if (!ss.eof())
  {
    ss.ignore(std::numeric_limits<std::streamsize>::max(), 'Z');
  }
  else
  {
    return false;
  }
  return true;
}

/// \brief Format checker for a date-time field
///
/// \param format Name of the field whose format is to be checked
/// \param value Value associated with the given field
///
/// \throw std::invalid_argument if the value in the date-time field does not
/// follow ISO8601 format. \throw std::logic_error if the format field is not
/// "date-time".
static void date_time_format_checker(
  const std::string& format, const std::string& value)
{
  if (format == "date-time")
  {
    if (!is_in_ISO8601_format(value))
    {
      throw std::invalid_argument("Value is not in valid ISO8601 format");
    }
  }
  else
  {
    throw std::logic_error("Don't know how to validate " + format);
  }
}

/// \brief Checks that a JSON object is following a given schema
///
/// \param schema The schema to validate against, as an nlohmann::json object
/// \param j Reference to the nlohmann::json object to be validated
///
/// \return true if schema is valid, false otherwise
bool is_valid_schema(nlohmann::json schema, nlohmann::json& j)
{
  nlohmann::json_schema::json_validator validator(
    nullptr, date_time_format_checker);

  try
  {
    validator.set_root_schema(schema);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Validation of schema failed: " << e.what() << "\n";
    return false;
  }

  try
  {
    validator.validate(j);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

#endif  // VDA5050_JSON_UTILS__VALIDATORS_HPP_
