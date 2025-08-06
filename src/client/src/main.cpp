#include <iostream>
using namespace std;
extern "C"
{
#include "view.h"
}

int
main(int num_args,
     char *argv[])
{
  if (num_args != 3) {
    cerr << "Use: " << argv[0] << " <server_ip> <port>" << endl;
    return EXIT_FAILURE;
  }

  char *server_ip = argv[1];
  int port;  
  try
    {
      port = stoi(argv[2]);
      if (port < 1024 || port > 49151) 
	throw out_of_range("Port number must be between 1024-49151.");
    }
  catch (const std::exception &e)
    {
      cerr << "[ERROR] Invalid port: " << e.what() << endl;
      return EXIT_FAILURE;
    }

  cout << "Launching gui client..." << endl;
  launch_gui(port, server_ip);
  return EXIT_SUCCESS;
}
