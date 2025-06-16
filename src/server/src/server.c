#include "server.h"
#include "message.h"

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
  else if (type == 'a')
    printf("[ALERT] %s\n", text);
  else
    printf("[INFO] %s\n", text);  
}

/* SIGINT signal handler */
void handle_sigint(int sig) {
  if (server_fd != -1) {
    close(server_fd);
    print_message("Server socket closed due to SIGINT (Ctrl+C).", 'a');
  }
  (void)sig;
  exit(0);
}

/* */
void disconnect_client(Client *client) {
  if (strlen(client->username) > 0) {
    Message *client_disconnected = create_disconnected_message(client->username);
    char *json_str = to_json(client_disconnected);
    broadcast_message(json_str, client->socket_fd);
    free(json_str);
    free_message(client_disconnected);
  }
  
  pthread_mutex_lock(&clients_mutex);
  
  // Remove the client from the list
  Client **prev = &clients;
  Client *current = clients;
  while (current != NULL) {
    if (current == client) {
      *prev = current->next;
      break;
    }
    prev = &current->next;
    current = current->next;
  }
  
  pthread_mutex_unlock(&clients_mutex);
  close(client->socket_fd);
  free(client);
}

/* Sends a message to one client */
void send_message(Client *client, const char *message) {
  if (send(client->socket_fd, message, strlen(message), 0) < 0) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Failed to send message to client [%s].", client->username);
    print_message(buffer, 'e');
  }
}

/* Send a message to all the clients in the server, except for the sender*/
void broadcast_message(const char *message, int sender_socket) {
  pthread_mutex_lock(&clients_mutex);
  Client *client = clients;
  
  while (client != NULL) {
    if (client->socket_fd != sender_socket && strlen(client->username) > 0)
      send_message(client, message);
    client = client->next;
  }
  
  pthread_mutex_unlock(&clients_mutex);
}

/* */
void invalid_response(Client *client, const char *result) {
  Message *response = create_response_message("INVALID", result, "");
  char *json_str = to_json(response);
  send_message(client, json_str);
  free(json_str);
  free_message(response);
  disconnect_client(client);
}

/* */
void leave_room(Client *client, Message *incoming_message) {
  return;
}

/* */
void room_text(Client *client, Message *incoming_message) {
  return;
}

/* */
void get_room_users(Client *client, Message *incoming_message) {
  return;
}

/* */
void join_room(Client *client, Message *incoming_message) {
  return;
}

/* */
void get_guests(Client *client, Message *incoming_message) {
  return;
}

/* */
void new_room(Client *client, Message *incoming_message) {
  return;
}

/* */
void public_text(Client *client, Message *incoming_message) {
  return;
}

/* */
void private_text(Client *client, Message *incoming_message) {
  return;
}

/* */
void get_users_list(Client *client, Message *incoming_message) {
  return;
}

/* */
void change_status(Client *client, Message *incoming_message) {
  return;
}

/* */
bool handle_identify(Client *client, Message *incoming_message) {
  const MessageType type = get_type(incoming_message);
  if (type != IDENTIFY)
    return false;
  
  const char *username = get_username(incoming_message);
  if (strcmp(username, "") == 0)
    return false;
  
  Message *response;
  char *json_str;
  
  pthread_mutex_lock(&clients_mutex);
  Client *current = clients;
  while (current != NULL) {
    if (strcmp(current->username, username) == 0) {
      pthread_mutex_unlock(&clients_mutex);
      response = create_response_message("IDENTIFY", "USER_ALREADY_EXISTS", username);
      json_str = to_json(response);
      send_message(client, json_str);
      free(json_str);
      free_message(response);
      return false;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&clients_mutex);
  
  // At this point, the username is valid and then we assign it to the client.
  strncpy(client->username, username, sizeof(client->username) - 1);
  client->username[sizeof(client->username) - 1] = '\0';
  strncpy(client->status, "ACTIVE", sizeof(client->status) - 1); //default client status
  client->status[sizeof(client->status) - 1] = '\0';
  response = create_response_message("IDENTIFY", "SUCCESS", client->username);
  json_str = to_json(response);
  send_message(client, json_str);
  free(json_str);
  free_message(response);
  
  // Notify the rest of new connected client 
  Message *new_user = create_new_user_message(client->username);
  json_str = to_json(new_user);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(new_user);
  return true; 
}

/* */
bool client_actions(Client *client, Message *incoming_message) {
  const MessageType type = get_type(incoming_message);

  switch (type) {
  case STATUS:
    change_status(client, incoming_message);
    break;
  case USERS:
    get_users_list(client, incoming_message);
    break;
  case TEXT:
    private_text(client, incoming_message);
    break;
  case PUBLIC_TEXT:
    public_text(client, incoming_message);
    break;
  case NEW_ROOM:
    new_room(client, incoming_message);
    break;
  case INVITE:
    get_guests(client, incoming_message);
    break;
  case JOIN_ROOM:
    join_room(client, incoming_message);
    break;
  case ROOM_USERS:
    get_room_users(client, incoming_message);
    break;
  case ROOM_TEXT:
    room_text(client, incoming_message);
    break;
  case LEAVE_ROOM:
    leave_room(client, incoming_message);
    break;
  case DISCONNECT:
    printf("Client: [%s] requested a disconnection.\n", client->username);
    return false;
  default:
    invalid_response(client, "INVALID");
    return false;
  }
  
  return true;
}

/* Function to handle each client */
void *handle_client(void *arg) {
  Client *client = (Client *)arg;
  char buffer[1024];
  bool identified = false;
  bool is_connected = true;
  int received_bytes;
  
  while (is_connected) {
    received_bytes = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (received_bytes > 0) {
      buffer[received_bytes] = '\0';
      Message *incoming_msg = parse(buffer);
      
      if (!incoming_msg) {
	invalid_response(client, "INVALID");
	print_message("Invalid message received from the client, disconnecting it.", 'a');
	break;
      }

      if (!identified) {
	identified = handle_identify(client, incoming_msg);
	if (!identified) {
	  print_message("Disconnecting unidentified client.", 'i');
	  invalid_response(client, "NOT_IDENTIFIED");
	  free_message(incoming_msg);
	  break;
	}
	print_message("Client connected and identified.", 'i');
      } else {
	is_connected = client_actions(client, incoming_msg);
	if (!is_connected) {
	  free_message(incoming_msg);
	  break;
	}
      }
      free_message(incoming_msg);
    } else {
      if (received_bytes == 0)
	print_message("Client received disconnected.", 'i');
      else
	print_message("Fail receiving client data.", 'e');
      break;
    }
  }

  disconnect_client(client);
  return NULL;
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

    if (pthread_create(&client->thread, NULL, handle_client, client) != 0) {
      print_message("Could not create client thread.", 'e');
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
    print_message("[ERROR] Error setting server port.", 'e');
  else
    print_message("Server address configured.", 'i');

  // Bind the socket to the specified port
  print_message("Binding the socket to port...", 'i');
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    close(server_fd);
    print_message("[ERROR] Socket cannot be associated to the port.", 'e');
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
