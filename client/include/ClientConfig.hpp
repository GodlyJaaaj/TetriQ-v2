// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "AConfig.hpp"

#include <cstdint>
#include <string>

namespace tetriq {
    class ClientConfig : AConfig {
    public:
        ClientConfig(const std::string &config_name = "client.toml");

        uint32_t max_incoming_bandwidth = 0;
        uint32_t max_outgoing_bandwidth = 0;
        uint32_t server_timeout = 1000;
    };
}
