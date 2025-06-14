#include "server.h"

#define BACKLOG 20 //Maximum of queued connections

Client *clients = NULL;

static int server_fd = -1;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Function to print messages with different types: error, alert, or info */
void print_message(const char *text, char type) {
  if (type == 'e') {
    perror(text);  
    exit(EXIT_FAILURE);
  }
  if (type == 'a')
    printf("[ALERT] %s\n", text);
  
  printf("[INFO] %s\n", text);  
}

/* SIGINT signal handler */
void handle_sigint(int sig) {
  if (server_fd != -1) {
    close(server_fd);
    print_message("Server socket closed due to SIGINT (Ctrl+C).", 'a');
  }
  exit(0);
}

/* */
void disconnect_client(Client *client) {
}

/* Function to handle each client */
void handle_client(Client *client) {
  char buffer[1024];
  bool identified = false;
  int received_bytes = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);
  
  while (received_bytes > 0) {
    buffer[received_bytes] = '\0';
    char *raw_message = buffer;
    
  }

  if (bytes_received == 0)
    printf("Cliente [%s] desconectado.\n", client->username);
  else if (bytes_received == -1)
    print_message("Error al recibir datos del cliente",);
  
  disconnect_client(client);
}

/* Function to handle each client */
void server_cycle(int server_fd) {
  int client_fd;
  socklen_t client_len;
  struct sockaddr_in client_addr;
  
  // Infinite loop for chat 
  while (1) {
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_fd < 0) {
      print_message("[ERROR] Server accept failed.", 'e');
      continue;
    }
    
    //Create a new client and handle it in a separated thread
    Client *client = (Client *)malloc(sizeof(Client));
    if (client == NULL) {
      print_message("[ERROR] Could not allocate memory for the client.", 'e');
      close(client_fd);
      continue;
    }
    
    client->socket_fd = client_fd;
    client->username[0] = '\0'; 
    client->next = NULL;
    pthread_mutex_lock(&clients_mutex);
    client->next = clients;
    clients = client;
    pthread_mutex_unlock(&clients_mutex);

    if (pthread_create(&client->thread, NULL, (void *)handle_client, client) != 0) {
      print_message("[ERROR] Could not create client thread.", 'e');
      pthread_mutex_lock(&clients_mutex);
      clients = clients->next;
      pthread_mutex_unlock(&clients_mutex);
      free(client);
      close(client_fd);
      continue;
    }
    
    pthread_detach(client->thread);
  }
}
  
/* Function to start the server */
void start_server(int port) {
  signal(SIGINT, handle_sigint);
  
  //int server_fd;
  struct sockaddr_in server_addr;
  
  // Create the server socket
  print_message("Creating the server socket...", 'i');
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
    print_message("[ERROR] Error creating socket\n", 'e');
  else
    print_message("Socket succesfully created.", 'i');
  
  // Set server address
  print_message("Configuring the server address...", 'i');
  server_addr.sin_family = AF_INET;         // Address-family IPv4
  server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on all interfaces
  server_addr.sin_port = htons(port);       // Convert port number to network byte order

  if (server_addr.sin_port == 0)
    print_message("[ERROR] Error setting server port", 'e');
  else
    print_message("Server address configured.", 'i');

  // Bind the socket to the specified port
  print_message("Binding the socket to port...", 'i');
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    close(server_fd);
    print_message("[ERROR] Socket cannot be associated to the port\n", 'e');
  } else
    print_message("Socket successfully bound to the port.", 'i');

  // Put the server in listening mode
  print_message("Putting the server in listening mode...", 'i');
  if (listen(server_fd, BACKLOG) == -1) {
    close(server_fd);
    print_message("[ERROR] Error listening port.", 'e');
  } else
    print_message("Server is now listening for incoming connections.", 'i');
  
  server_cycle(server_fd);

  // Closing server
  print_message("Closing the server socket...", 'i');
  if (close(server_fd) == 0)
    print_message("Server socket closed successfully.", 'i');
  else
    print_message("[ERROR] Error closing the server socket.", 'e');
}
