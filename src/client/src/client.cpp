#include <iostream>
#include <stdexcept>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.hpp"

/* */
Client::Client() : socket_fd(-1), is_connected(false) {}

/* */
Client::~Client() {
  if (is_connected)
    disconnect();
}

/* */
bool Client::connect_to_server(const std::string& server_ip, int port) {
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
        throw std::runtime_error("Failed to create socket");

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0)
        throw std::runtime_error("Invalid server IP address");

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
        throw std::runtime_error("Failed to connect to the server");

    is_connected = true;
    std::cout << "Successfully connected to the server." << std::endl;
    return true;
}

/* */
void Client::run_client() {
    std::cout << "[INFO] Connected to the server. Type 'exit' to quit." << std::endl;

    listener_thread = std::thread(&Client::receive_message, this);

    std::string user_input;
    while (is_connected) {
        std::getline(std::cin, user_input);
        if (user_input == "exit") {
            disconnect();
            break;
        }
        send_message(user_input);
    }

    if (listener_thread.joinable()) {
        listener_thread.join();
    }
}

/* */
void Client::disconnect() {
    if (is_connected) {
        is_connected = false;
        close(socket_fd);
        std::cout << "[INFO] Disconnected from the server." << std::endl;
    }
}

/* */
void Client::signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cerr << "\n[INFO] Interruption received. Disconnecting..." << std::endl;
	disconnnect();
        exit(EXIT_SUCCESS);
    }
}
