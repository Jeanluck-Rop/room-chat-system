#include <csignal>
#include <iostream>
#include "client.hpp"

using namespace std;

extern "C"
{
#include "view.h"
}

int
main(int num_args,
     char *argv[])
{
  /*if (num_args != 3) {
    std::cerr << "Use: " << argv[0] << " <server_ip> <port>" << std::endl;
    return EXIT_FAILURE;
  }

  char *server_ip = argv[1];
  int port;  
  try
    {
      port = std::stoi(argv[2]);
      if (port < 1024 || port > 49151) 
	throw std::out_of_range("Port number must be between 1024 and 49151.");
    }
  catch (const std::exception &e)
    {
      std::cerr << "[ERROR] Invalid port: " << e.what() << std::endl;
      return EXIT_FAILURE;
      }*/

  std::signal(SIGINT, Client::signal_handler);
  cout << "Launching gui client...\n";
  //launch_gui(server_ip, port);
  launch_gui();
  return EXIT_SUCCESS;
}
