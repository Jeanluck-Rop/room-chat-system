#include "server.h"
#include "cJSON.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <pthread.h>
#include <arpa/inet.h> 

#define BACKLOG 10 // Maximum of queued connections

Client *clients = NULL;

/* Function to print messages with different types: error, alert, or info */
void print_message(const char *text, char type) {
  if (type == 'e') {
    perror(text);     
    exit(EXIT_FAILURE);
  } else if (type == 'a')
    printf("[ALERT] %s\n", text); 
  else 
    printf("[INFO] %s\n", text);
}

/* Function to start the server */
void start_server(int port) {
  int server_fd;
  struct sockaddr_in server_addr;

  // Create the server socket
  print_message("Creating the server socket...", 'i');
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
    print_message("[ERROR] Error creating socket\n", 'e');
  else 
    print_message("Socket succesfully created.", 'i');

  //Set server address
  print_message("Configuring the server address...", 'i');
  server_addr.sin_family = AF_INET;         // Address-family IPv4
  server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on all interfaces
  server_addr.sin_port = htons(port);       // Convert port number to network byte order

  if (server_addr.sin_port == 0)
    print_message("Error setting server port", 'e');
  else
    print_message("Server address configured.", 'i');

  // Bind the socket to the specified port
  print_message("Binding the socket to port...", 'i');
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    close(server_fd);
    print_message("[ERROR] Socket cannot be associated to the port\n", 'e');
  } else
    print_message("Socket successfully bound to the port.", 'i');

  // Put the server in listening mode
  print_message("Putting the server in listening mode...", 'i');
  if(listen(server_fd, BACKLOG) == -1) {
    close(server_fd);
    print_message("[ERROR] Error listening port\n", 'e');
  } else
    print_message("Server is now listening for incoming connections.", 'i');

  printf("Server listening on port %d\n", port);

  print_message("Closing the server socket...", 'i');
  if (close(server_fd) == 0)
    print_message("Server socket closed successfully.", 'i');
  else
    print_message("Error closing the server socket.", 'i');
}
