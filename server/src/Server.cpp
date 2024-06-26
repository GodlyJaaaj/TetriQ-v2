// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Server.hpp"
#include "Channel.hpp"
#include "Logger.hpp"
#include "Messages.hpp"
#include "ServerConfig.hpp"
#include "network/PacketHandler.hpp"

#include <chrono>
#include <cstdint>
#include <thread>
#include <tuple>
#include <utility>

namespace tetriq {
    Server::Server()
        : _address()
        , _server(nullptr)
        , _channels({Channel(this, _channel_id_counter)})
        , _rcon(*this)
    {
        if (init() == false)
            throw ServerInitException();
    }

    Player &Server::getPlayerById(uint64_t id)
    {
        return _players.at(id);
    }

    const ServerConfig &Server::getConfig() const
    {
        return _config;
    }

    std::vector<Channel> &Server::getChannels()
    {
        return _channels;
    }

    bool Server::createChannel()
    {
        _channel_id_counter++;
        _channels.emplace_back(this, _channel_id_counter);
        return true;
    }

    bool Server::deleteChannel(uint64_t id)
    {
        if (id == 0) {
            return false;
        }
        for (size_t i = 0; i < _channels.size(); i++) {
            if (_channels[i].getChannelId() == id) {
                _channels.erase(_channels.begin() + i);
                return true;
            }
        }
        return false;
    }

    bool Server::init()
    {
        Logger::log(LogLevel::INFO, "Server started");
        if (enet_initialize() != 0) {
            Logger::log(LogLevel::CRITICAL, ENET_INIT_ERROR);
            return false;
        }
        Logger::log(LogLevel::INFO, ENET_INIT_SUCCESS);
        if (not setHost() or not createHost())
            return false;
        if (_config.rcon.enabled) {
            _rcon.init();
        }
        return true;
    }

    bool Server::setHost()
    {
        if (enet_address_set_host(&_address, _config.listen_address.c_str()) != 0) {
            LogLevel::ERROR << "Failed to set host address: " << _config.listen_address
                            << std::endl;
            return false;
        }
        _address.port = _config.listen_port;
        LogLevel::INFO << "Host set to " << _config.listen_address << ":" << _config.listen_port
                       << std::endl;
        return true;
    }

    bool Server::createHost()
    {
        _server = enet_host_create(&_address,
            _config.max_clients,
            0,
            _config.max_incoming_bandwidth,
            _config.max_outgoing_bandwidth);
        if (_server == nullptr) {
            Logger::log(LogLevel::CRITICAL, "An error occurred while creating the enet host.");
            return false;
        }
        LogLevel::DEBUG << "Server created with max clients: " << _config.max_clients
                        << ", max incoming bandwidth: " << _config.max_incoming_bandwidth
                        << ", max outgoing bandwidth: " << _config.max_outgoing_bandwidth
                        << std::endl;
        return true;
    }

    void Server::listen()
    {
        _next_tick = std::chrono::steady_clock::now()
                     + std::chrono::nanoseconds(1'000'000'000 / _config.ticks_per_second);
        while (not should_exit && _running) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (now > _next_tick) {
                LogLevel::WARNING << "main loop is running behind" << std::endl;
                _next_tick = std::chrono::steady_clock::now()
                             + std::chrono::nanoseconds(1'000'000'000 / _config.ticks_per_second);
            } else {
                std::this_thread::sleep_until(_next_tick);
                _next_tick += std::chrono::nanoseconds(1'000'000'000 / _config.ticks_per_second);
            }
            if (!handleENetEvents())
                break;
            if (!handleRconEvents())
                break;
            for (Channel &channel : _channels) {
                channel.tick();
            }
        }
    }

    bool Server::handleRconEvents()
    {
        if (_config.rcon.enabled) {
            _rcon.listen();
        }
        return true;
    }

    bool Server::handleENetEvents()
    {
        ENetEvent event;
        while (enet_host_service(_server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    handleNewClient(event);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    handleClientDisconnect(event);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    handleClientPacket(event);
                    break;
                case ENET_EVENT_TYPE_NONE:
                    handleNone(event);
                    break;
            }
        }
        return true;
    }

    bool Server::handleNewClient(ENetEvent &event)
    {
        event.peer->data = new uint64_t(_network_id_counter);
        _channels.front().broadcastPacket(
            ConnectPacket(_network_id_counter, _config.game.width, _config.game.height));
        _players.emplace(std::piecewise_construct,
            std::forward_as_tuple(_network_id_counter),
            std::forward_as_tuple(_network_id_counter, event.peer, _channels.front()));
        Player &player = _players.at(_network_id_counter);
        player.sendInitGamePacket(_config.game);
        player.setGameOver(true);
        // TODO : choose a channel intelligently and dont start game instantly
        _network_id_counter++;
        return true;
    }

    void Server::handleClientDisconnect(ENetEvent &event)
    {
        uint64_t network_id = *(uint64_t *) event.peer->data;
        Player &player = _players.at(network_id);
        Channel &channel = player.getChannel();
        player.disconnect();
        _players.erase(network_id);
        delete (uint64_t *) event.peer->data;
        event.peer->data = nullptr;
        channel.broadcastPacket(DisconnectPacket(network_id));
    }

    void Server::handleClientPacket(ENetEvent &event)
    {
        uint64_t network_id = *(uint64_t *) event.peer->data;
        Player &player = _players.at(network_id);
        /*Channel &channel = player.getChannel();*/

        PacketHandler::decodePacket(event, {&player});
    }

    void Server::handleNone([[maybe_unused]] ENetEvent &event) const
    {
        // Logger::log(LogLevel::DEBUG, "No event occurred");
    }

    Server::~Server()
    {
        enet_host_destroy(_server);
        enet_deinitialize();
        Logger::log(LogLevel::INFO, ENET_DEINIT_SUCCESS);
        Logger::log(LogLevel::INFO, "Server stopped");
    }
}
