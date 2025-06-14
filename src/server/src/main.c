#include <stdio.h>
#include <stdlib.h>
#include "server.h"

int main(int num_args, char *argv[]) {
  if (num_args != 2) {
    fprintf(stderr, "Use: ./src/server/server <port> \n");
    return EXIT_FAILURE;
  }

  int port = atoi(argv[1]);
  if (port < 1024 || port > 49151) {
     fprintf(stderr, "Invalid port.\n Valid ports between 1024 and 49151");
     return EXIT_FAILURE; 
  }

  printf("Starting server at the port %d:\n", port);
  start_server(port);
  return EXIT_SUCCESS;
}
