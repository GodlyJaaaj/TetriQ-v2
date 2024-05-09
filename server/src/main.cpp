// SPDX-FileCopyrightText: 2024 The TetriQ authors
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Server.hpp"

#include <iostream>

bool should_exit = false;

/**
 * @brief Main function of the server
 *
 *@todo use a configuration file
 */
int main(int argc, char* argv[])
{
    signal(SIGINT, [](int) { should_exit = true; });
    try {
        tetriq::Server server("0.0.0.0", "4242");
        server.listen();
    } catch (const tetriq::Server::ServerException &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
