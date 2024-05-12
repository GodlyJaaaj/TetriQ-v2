// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "network/IPacket.hpp"
#include "network/PacketId.hpp"

#include <cstdint>

namespace tetriq {
    struct TestPacket : public IPacket {
        public:
            PacketId getId() const override;
            
            NetworkStream &operator>>(NetworkStream &ns) const override;
            NetworkStream &operator<<(NetworkStream &ns) override;
            size_t getSize() const override;
        private:
            uint64_t _magic = 0x737819;
    };
}