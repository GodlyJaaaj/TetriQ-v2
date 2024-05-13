// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Client.hpp"
#include "IDisplay.hpp"
#include "Logger.hpp"
#include "Messages.hpp"

#include <utility>

namespace tetriq {
    Client::Client(std::string ip, std::string port, IDisplay &display)
        : _username("Unknown")
        , _server_ip(std::move(ip))
        , _server_port(std::move(port))
        , _game(12, 22)
        , _display(display)
    {
        if (init() == false)
            throw ClientInitException();
        _client = enet_host_create(nullptr, 1, 0,
                                   _config.max_incoming_bandwidth, _config.max_outgoing_bandwidth);
        if (_client == nullptr or not setServer())
            throw ClientInitException();
        if (not connectToServer())
            throw ClientConnectionException();
        if (!_display.loadGame(_game.getState()))
            throw ClientInitException();
    }

    Client::~Client()
    {
        enet_peer_disconnect_now(_server, 0);
        enet_host_destroy(_client);
        enet_deinitialize();
        Logger::log(LogLevel::INFO, ENET_DEINIT_SUCCESS);
        Logger::log(LogLevel::INFO, "Client stopped");
    }

    void Client::loop()
    {
        ENetEvent _event;
        while (true) {
            if (!_display.draw(_game.getState()))
                break;
            // if (!_display.handleEvents(_game.getState()))
            //     break;
            if (enet_host_service(_client, &_event, _config.server_timeout) < 0)
                break;
            if (_event.type == ENET_EVENT_TYPE_RECEIVE) {
                _game.decodePacket(_event);
            }
            if (_event.type == ENET_EVENT_TYPE_DISCONNECT) {
                Logger::log(LogLevel::INFO, "Disconnected from the server");
                break;
            }
        }
    }

    bool Client::init() const
    {
        Logger::log(LogLevel::INFO, "Client started");
        if (enet_initialize() != 0) {
            Logger::log(LogLevel::CRITICAL, ENET_INIT_ERROR);
            return false;
        }
        Logger::log(LogLevel::INFO, ENET_INIT_SUCCESS);
        return true;
    }

    bool Client::connectToServer()
    {
        ENetEvent _event;
        _server = enet_host_connect(_client, &_address, 0, 0);
        if (_server == nullptr) {
            Logger::log(LogLevel::CRITICAL, "An error occurred while connecting to the server : "
                        + _server_ip + ":" + _server_port);
            return false;
        }
        if (enet_host_service(_client, &_event, _config.server_timeout) > 0 and
            _event.type == ENET_EVENT_TYPE_CONNECT) {
            Logger::log(LogLevel::INFO, "Connected to the server : "
                        + _server_ip + ":" + _server_port);
        } else {
            enet_peer_reset(_server);
            Logger::log(LogLevel::CRITICAL, "Failed to connect to the server : "
                        + _server_ip + ":" + _server_port);
            return false;
        }
        return true;
    }

    bool Client::setServer()
    {
        if (enet_address_set_host(&_address, _server_ip.c_str()) != 0) {
            const std::string message = "Failed to set server address: " + _server_ip;
            Logger::log(LogLevel::ERROR, message);
            return false;
        }
        try {
            _address.port = std::stoi(_server_port);
        } catch (const std::invalid_argument &e) {
            const std::string message = "Failed to set server port: " + _server_port;
            Logger::log(LogLevel::ERROR, message);
            return false;
        }
        const std::string message = "Server set to " + _server_ip + ":" + _server_port;
        Logger::log(LogLevel::INFO, message);
        return true;
    }
}
