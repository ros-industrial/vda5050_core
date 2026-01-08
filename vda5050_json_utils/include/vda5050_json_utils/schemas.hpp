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

#ifndef VDA5050_JSON_UTILS__SCHEMAS_HPP_
#define VDA5050_JSON_UTILS__SCHEMAS_HPP_

#include <string>

/// \brief Schema of the VDA5050 Connection Object
inline constexpr auto connection_schema = R"(
    {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "title": "connection",
        "description": "The last will message of the AGV. Has to be sent with retain flag.\nOnce the AGV comes online, it has to send this message on its connect topic, with the connectionState enum set to \"ONLINE\".\n The last will message is to be configured with the connection state set to \"CONNECTIONBROKEN\".\nThus, if the AGV disconnects from the broker, master control gets notified via the topic \"connection\".\nIf the AGV is disconnecting in an orderly fashion (e.g. shutting down, sleeping), the AGV is to publish a message on this topic with the connectionState set to \"DISCONNECTED\".",
        "subtopic": "/connection",
        "type": "object",
        "required": [
            "headerId",
            "timestamp",
            "version",
            "manufacturer",
            "serialNumber",
            "connectionState"
        ],
        "properties": {
            "headerId": {
                "type": "integer",
                "description": "Header ID of the message. The headerId is defined per topic and incremented by 1 with each sent (but not necessarily received) message."
            },
            "timestamp": {
                "type": "string",
                "format": "date-time",
                "description": "Timestamp in ISO8601 format (YYYY-MM-DDTHH:mm:ss.ssZ).",
                "examples": [
                    "1991-03-11T11:40:03.12Z"
                ]
            },
            "version": {
                "type": "string",
                "description": "Version of the protocol [Major].[Minor].[Patch]",
                "examples": [
                    "1.3.2"
                ]
            },
            "manufacturer": {
                "type": "string",
                "description": "Manufacturer of the AGV."
            },
            "serialNumber": {
                "type": "string",
                "description": "Serial number of the AGV."
            },
            "connectionState": {
                "type": "string",
                "enum": [
                    "ONLINE",
                    "OFFLINE",
                    "CONNECTIONBROKEN"
                ],
                "description": "ONLINE: connection between AGV and broker is active. OFFLINE: connection between AGV and broker has gone offline in a coordinated way. CONNECTIONBROKEN: The connection between AGV and broker has unexpectedly ended."
            }
        }
    }
)";

#endif  // VDA5050_JSON_UTILS__SCHEMAS_HPP_
