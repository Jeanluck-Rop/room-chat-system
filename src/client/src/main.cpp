#include <iostream>
#include <csignal> // For handling signals like SIGINT
#include <memory> // For using smart pointers like std::unique_ptr
#include "client.hpp"

/* */
int main(int num_args, char *argv[]) {
  // Check if the correct number of arguments is provided, <run_command_in_cmake> <server_ip> <port>
  if (num_args != 3) {
    std::cerr << "Use: " << argv[0] << " <server_ip> <port>" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse server IP and port number from command-line arguments
  const char *server_ip = argv[1];
  int port;
  
  try {
    port = std::stoi(argv[2]);
    if (port < 1024 || port > 49151) 
      throw std::out_of_range("Port number must be between 1024 and 49151.");
  } catch (const std::exception &e) {
    std::cerr << "[ERROR] Invalid port: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Set up a signal handler for SIGINT (like Ctrl+C interrupt)
  // This allows us the program to perform cleanup before exiting
  std::signal(SIGINT, Client::signal_handler);
  
  try {
    auto& client = Client::instance();
    // Attempt to connect to the server using the provided IP and port
    client.connect_to_server(server_ip, port);
    client.run_client();
  } catch (const std::runtime_error &e) {
    std::cerr << "[ERROR]: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
    
  return EXIT_SUCCESS;
}
