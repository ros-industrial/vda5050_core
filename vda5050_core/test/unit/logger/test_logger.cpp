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

#include <gtest/gtest.h>

#include <vector>

#include "vda5050_core/logger/logger.hpp"

struct LogEntry
{
  vda5050_core::logger::LogLevel level;
  std::string message;
};

class MockLogger : public vda5050_core::logger::LogHandler
{
public:
  void log(
    vda5050_core::logger::LogLevel level, const std::string& message) override
  {
    logs.push_back({level, message});
  }

  std::vector<LogEntry> logs;
};

TEST(LoggerTest, BasicLogging)
{
  auto mock = std::unique_ptr<MockLogger>(new MockLogger());
  MockLogger* mock_ptr = mock.get();

  vda5050_core::logger::set_handler(std::move(mock));

  vda5050_core::logger::set_log_level(vda5050_core::logger::LogLevel::DEBUG);

  VDA5050_DEBUG("Example debug message ...");
  VDA5050_INFO("Important information.");
  VDA5050_WARN("Something is going wrong!");
  VDA5050_ERROR("Everything has gone wrong!!");
  VDA5050_FATAL("All is lost ...");

  VDA5050_DEBUG_STREAM("Debug stream with code: " << 452);
  VDA5050_INFO_STREAM("Essential info with code: " << 323);
  VDA5050_WARN_STREAM("Look here before proceeding: " << 893);
  VDA5050_ERROR_STREAM("Danger! Check code: " << 456);
  VDA5050_FATAL_STREAM("Fatality!! With code: " << 539);

  ASSERT_EQ(mock_ptr->logs.size(), 10);

  EXPECT_EQ(mock_ptr->logs[0].level, vda5050_core::logger::LogLevel::DEBUG);
  EXPECT_EQ(mock_ptr->logs[0].message, "Example debug message ...");

  EXPECT_EQ(mock_ptr->logs[1].level, vda5050_core::logger::LogLevel::INFO);
  EXPECT_EQ(mock_ptr->logs[1].message, "Important information.");

  EXPECT_EQ(mock_ptr->logs[2].level, vda5050_core::logger::LogLevel::WARN);
  EXPECT_EQ(mock_ptr->logs[2].message, "Something is going wrong!");

  EXPECT_EQ(mock_ptr->logs[3].level, vda5050_core::logger::LogLevel::ERROR);
  EXPECT_EQ(mock_ptr->logs[3].message, "Everything has gone wrong!!");

  EXPECT_EQ(mock_ptr->logs[4].level, vda5050_core::logger::LogLevel::FATAL);
  EXPECT_EQ(mock_ptr->logs[4].message, "All is lost ...");

  EXPECT_EQ(mock_ptr->logs[5].level, vda5050_core::logger::LogLevel::DEBUG);
  EXPECT_EQ(mock_ptr->logs[5].message, "Debug stream with code: 452");

  EXPECT_EQ(mock_ptr->logs[6].level, vda5050_core::logger::LogLevel::INFO);
  EXPECT_EQ(mock_ptr->logs[6].message, "Essential info with code: 323");

  EXPECT_EQ(mock_ptr->logs[7].level, vda5050_core::logger::LogLevel::WARN);
  EXPECT_EQ(mock_ptr->logs[7].message, "Look here before proceeding: 893");

  EXPECT_EQ(mock_ptr->logs[8].level, vda5050_core::logger::LogLevel::ERROR);
  EXPECT_EQ(mock_ptr->logs[8].message, "Danger! Check code: 456");

  EXPECT_EQ(mock_ptr->logs[9].level, vda5050_core::logger::LogLevel::FATAL);
  EXPECT_EQ(mock_ptr->logs[9].message, "Fatality!! With code: 539");

  vda5050_core::logger::release_handler();
}

TEST(LoggerTest, LogLevelFiltering)
{
  auto mock = std::unique_ptr<MockLogger>(new MockLogger());
  MockLogger* mock_ptr = mock.get();

  vda5050_core::logger::set_handler(std::move(mock));

  vda5050_core::logger::set_log_level(vda5050_core::logger::LogLevel::WARN);

  VDA5050_DEBUG("Should be ignored");
  VDA5050_INFO("Should be ignored");
  VDA5050_WARN("Should appear first");
  VDA5050_ERROR("Should appear second");
  VDA5050_FATAL("Should appear third");

  ASSERT_EQ(mock_ptr->logs.size(), 3);

  EXPECT_EQ(mock_ptr->logs[0].level, vda5050_core::logger::LogLevel::WARN);
  EXPECT_EQ(mock_ptr->logs[0].message, "Should appear first");

  EXPECT_EQ(mock_ptr->logs[1].level, vda5050_core::logger::LogLevel::ERROR);
  EXPECT_EQ(mock_ptr->logs[1].message, "Should appear second");

  EXPECT_EQ(mock_ptr->logs[2].level, vda5050_core::logger::LogLevel::FATAL);
  EXPECT_EQ(mock_ptr->logs[2].message, "Should appear third");

  vda5050_core::logger::release_handler();
}
