#include <iostream>
#include <csignal>
#include <memory> 
#include "client.hpp"

std::unique_ptr<Client> client_ptr;

/* */
int main(int num_args, char *argv[]) {
    if (nume_args != 3) {
      std::cerr << "Uso: " << args[0] << " <server_ip> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *server_ip = args[1];
    int port;
    
    try {
      port = std::stoi(args[2]);
      if (port < 1024 || port > 49151) 
	throw std::out_of_range("Port number must be between 1024 and 49151.");
    } catch (const std::exception &e) {
      std::cerr << "[ERROR] Invalid port: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }
    
    std::signal(SIGINT, Client::signal_handler);
    
    try {
        client_ptr = std::make_unique<Client>();
        client_ptr->connect_to_server(server_ip, port);
        client_ptr->run_client();
    } catch (const std::runtime_error &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
