// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "GameConfig.hpp"
#include "Tetris.hpp"
#include "network/APacket.hpp"
#include "network/PacketHandler.hpp"
#include "network/packets/FullGameRequestPacket.hpp"
#include "network/packets/GameActionPacket.hpp"
#include <enet/enet.h>
#include <cstdint>

namespace tetriq {
    class Channel;

    class Player : public PacketHandler {
        public:
            Player(uint64_t network_id, ENetPeer *peer, Channel &channel);
            ~Player();

            void startGame(const GameConfig &config);
            void tickGame();
            void applyPackets();
            bool isGameOver() const;
            void setGameOver(bool game_over);
            Tetris &getGame();

            uint64_t getNetworkId() const;

            void sendPacket(const APacket &packet);

            bool handle(GameActionPacket &packet) override;
            bool handle(FullGameRequestPacket &packet) override;
            bool doPuSwitchField(BlockType power_up, Player &target);
            bool handle(PowerUpPacket &packet) override;

            Channel &getChannel();
            bool disconnect();

            void sendInitGamePacket(const GameConfig &config);

        private:
            const uint64_t _network_id;
            ENetPeer *const _peer;

            Channel &_channel;
            Tetris _game;

            uint64_t _applied_actions{0};
    };
}
