// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "IDisplay.hpp"
#include "ITetris.hpp"

#include <SFML/Graphics.hpp>
#include <cstdint>

namespace tetriq {
    class SFMLDisplay: public IDisplay {
        public:
            SFMLDisplay();
            ~SFMLDisplay() override;

            bool loadGame(const ITetris &game, uint64_t player_count) override;
            bool draw(const ITetris &game, ITetrisIter otherGamesStart, ITetrisIter otherGamesEnd) override;
            bool handleEvents(ITetris &game) override;
        private:
            void drawGame(const ITetris &game, Position position, uint64_t block_size);
            void drawBlock(sf::Vector2u pos, BlockType block, uint64_t block_size);
            void drawTetromino(const Tetromino &tetromino, Position position, uint64_t block_size);
            void drawCurrentTetromino(const ITetris &game);
            void drawNextTetromino(const ITetris &game);

            const uint64_t BLOCK_SIZE = 16;
            const uint64_t SIDEBAR_SIZE = 6;

            sf::RenderWindow _window;
            sf::Event _event;
    };
}
