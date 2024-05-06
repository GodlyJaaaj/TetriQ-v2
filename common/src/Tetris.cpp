/*
* This file is part of TetriQ.
* Copyright © 2024 Sébastien Lucas <sebastien.lucas@epitech.eu>
*
* TetriQ is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* TetriQ is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with TetriQ. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Tetris.hpp"

tetriq::Block::Block(Tetris &tetris, BlockType type)
: _tetris(tetris), _type(type), _isDestructible(false)
{}

tetriq::BlockType tetriq::Block::getType() const
{
    return _type;
}

tetriq::StandardBlock::StandardBlock(Tetris &_tetris, BlockType type)
: Block(_tetris, type)
{}

tetriq::StandardBlock::~StandardBlock()
= default;

bool tetriq::StandardBlock::isDestructible()
{
    return true;
}

void tetriq::StandardBlock::onDestruction()
{}

tetriq::IndestructibleBlock::IndestructibleBlock(Tetris &_tetris)
: Block(_tetris, BlockType::INDESTRUCTIBLE)
{}

tetriq::IndestructibleBlock::~IndestructibleBlock()
= default;

bool tetriq::IndestructibleBlock::isDestructible()
{
    return false;
}

void tetriq::IndestructibleBlock::onDestruction()
{}

tetriq::SpecialBlock::SpecialBlock(Tetris &_tetris, BlockType type) : Block(
    _tetris, type)
{}

tetriq::SpecialBlock::~SpecialBlock()
= default;

bool tetriq::SpecialBlock::isDestructible()
{
    return true;
}

void tetriq::SpecialBlock::onDestruction()
{}

tetriq::Tetris::Tetris(size_t width, size_t height)
: _width(width), _height(height)
{
    _blocks.resize(_height);
    for (size_t i = 0; i < _height; i++)
        _blocks[i].resize(_width);

    for (size_t i = 0; i < _height; i++) {
        for (size_t j = 0; j < _width; j++) {
            if (j == 0 || j == _width - 1 || i == _height - 1)
                _blocks[i][j] = std::make_unique<IndestructibleBlock>(*this);
            else
                _blocks[i][j] = std::make_unique<StandardBlock>(*this,
                    BlockType::EMPTY);
        }
    }
}

tetriq::Tetris::~Tetris()
{}

uint64_t tetriq::Tetris::getWidth() const
{
    return _width;
}

uint64_t tetriq::Tetris::getHeight() const
{
    return _height;
}

const std::unique_ptr<tetriq::Block> &tetriq::Tetris::getBlockAt(uint64_t x,
    uint64_t y) const
{
    return _blocks[y][x];
}
