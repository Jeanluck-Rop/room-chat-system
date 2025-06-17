#include "server.h"
#include "message.h"
#include "room.h"

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
Client *find_client_by_username(const char *username) {
  Client *current = clients;
  while (current != NULL) {
    if (strcmp(current->username, username) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

/* */
bool was_invited(Client *client, const char *roomname) {
  for (int i = 0; i < client->invited_count; i++)
    if (strcmp(client->invited_rooms[i], roomname) == 0)
      return true;
  
  return false;
}

/* */
void mark_as_invited(Client *client, const char *roomname) {
  if (was_invited(client, roomname))
    return;

  if (client->invited_capacity == 0) {
    client->invited_capacity = 4;
    client->invited_rooms = malloc(sizeof(char *) * client->invited_capacity);
  } else if (client->invited_count == client->invited_capacity) {
    client->invited_capacity *= 2;
    client->invited_rooms = realloc(client->invited_rooms, sizeof(char *) * client->invited_capacity);
  }

  client->invited_rooms[client->invited_count++] = strdup(roomname);
}

/* */
void room_response(Client *client, const char *operation, const char *result, const char *roomname) {
  Message *response = create_response_message(operation, result, roomname);
  char *json_str = to_json(response);
  send_message(client, json_str);
  free(json_str);
  free_message(response);
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
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0)
    return;
  
  Room *room_to_leave = find_room(roomname);
  if (room_to_leave == NULL) {
    room_response(client, "LEAVE_ROOM", "NO_SUCH_ROOM", roomname);
    printf("[INFO] Client [%s] tried to leave a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }

  if (!was_invited(client, roomname)) {
    room_response(client, "LEAVE_ROOM_", "NOT_JOINED", roomname);
    printf("[INFO] Client [%s] tried to leave a room [%s] that was not invited.\n", client->username, roomname);
    return;
  }
  
  if (!remove_client_from_room(room_to_leave, client)) {
    printf("[ALERT] Could not remove [%s] from the room [%s].\n", client->username, roomname);
    return;
  }
  
  Message *response = create_left_room_message(roomname, client->username);
  char *json_str = to_json(response);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(response);
}

/* */
void room_text(Client *client, Message *incoming_message) {
  const char *roomname = get_roomname(incoming_message);
  const char *text_content = get_text(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0 || !text_content || strcmp(text_content, "") == 0)
    return;
  
  Room *target_room = find_room(roomname);
  if (target_room == NULL) {
    room_response(client, "ROOM_TEXT", "NO_SUCH_ROOM", roomname);
    printf("[INFO] Client [%s] tried to send text to a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }

  if (!was_invited(client, roomname)) {
    room_response(client, "ROOM_TEXT", "NOT_JOINED", roomname);
    printf("[INFO] Client [%s] tried to send text to a room [%s] that was not invited.\n", client->username, roomname);
    return;
  }
  
  Message *msg = create_room_text_from_message(roomname, client->username, text_content);
  char *json_str = to_json(msg);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(msg);
}

/* */
void get_room_users(Client *client, Message *incoming_message) {
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0)
    return;

  Room *target_room = find_room(roomname);
  if (target_room == NULL) {
    room_response(client, "ROOM_USERS", "NO_SUCH_ROOM", roomname);
    printf("[INFO] Client [%s] tried to get room users from a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }

  if (!was_invited(client, roomname)) {
    room_response(client, "ROOM_USERS", "NOT_JOINED", roomname);
    printf("[INFO] Client [%s] tried to get room users from a room [%s] that was not invited.\n", client->username, roomname);
    return;
  }
  
  char **usernames = NULL;
  char **statuses = NULL;
  int count = 0;

  get_room_user_info(roomname, &usernames, &statuses, &count);

  Message *msg = create_room_users_list_message(roomname, (const char **)usernames, (const char **)statuses, count);
  char *json_str = to_json(msg);
  send_message(client, json_str);

  for (int i = 0; i < count; i++) {
    free(usernames[i]);
    free(statuses[i]);
  }
  free(usernames);
  free(statuses);
  free(json_str);
  free_message(msg);
}

/* */
void join_room(Client *client, Message *incoming_message) {
    const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0)
    return;
  
  Room *room_to_join = find_room(roomname);
  if (room_to_join == NULL) {
    room_response(client, "INVITE", "NO_SUCH_ROOM", roomname);
    printf("[INFO] Client [%s] tried to join a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }

  if (!was_invited(client, roomname)) {
    room_response(client, "JOIN_ROOM", "NOT_INVITED", roomname);
    printf("[INFO] Client [%s] tried to join a room [%s] which he was not invited\n", client->username, roomname);
    return;
  }
  
  if (!add_client_to_room(room_to_join, client)) {
    printf("[ALERT] Could not add [%s] to requested room [%s].\n", client->username, roomname);
    return;
  }
  
  room_response(client, "JOIN_ROOM", "SUCCESS", roomname);
  
  Message *response = create_joined_room_message(roomname, client->username);
  char *json_str = to_json(response);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(response);  
}

/* */
void invite_guests(Client *client, Message *incoming_message) {
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0)
    return;
  
  Room *target_room = find_room(roomname);
  if (target_room == NULL) {
    room_response(client, "INVITE", "NO_SUCH_ROOM", roomname);
    printf("[INFO] Client [%s] tried to invite users to an inexisting room.\n", client->username);
    return;
  }

  int guest_count = 0;
  char **guests_list = get_users(incoming_message, &guest_count);
  if (!guests_list)
    return;
  
  pthread_mutex_lock(&clients_mutex);

  for (int i = 0; i < guest_count; ++i) {
    if (!guests_list[i])
      continue;
    
    Client *exists = find_client_by_username(guests_list[i]);
    if (!exists)
      room_response(client, "INVITE", "NO_SUCH_USER", guests_list[i]);
    else {
      mark_as_invited(exists, roomname);
      Message *invitation = create_invite_message(guests_list[i], roomname);
      char *json_str = to_json(invitation);
      send_message(client, json_str);
      free(json_str);
      free_message(invitation);
    }
  }
  
  pthread_mutex_unlock(&clients_mutex);
  
  for (int i = 0; i < guest_count; i++)
    free(guests_list[i]);
  free(guests_list);
}

/* */
void new_room(Client *client, Message *incoming_message) {
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0)
    return;
  
  Room *new_room = create_room(roomname);
  if (new_room == NULL) {
    room_response(client, "NEW_ROOM", "ROOM_ALREADY_EXISTS", roomname);
    printf("[INFO] Client [%s] tried to create and existing room.\n", client->username);
    return;
  }

  //implement later a way to notify the customer that he couldnt be added to the new room
  if (!add_client_to_room(new_room, client)) {
    printf("[ALERT] Could not add [%s] to new room created.\n", client->username);
    cleanup_empty_rooms();
    return;
  }

  room_response(client, "NEW_ROOM", "SUCCESS", roomname);
}

/* */
void public_text(Client *client, Message *incoming_message) {
  const char *text_content = get_text(incoming_message);
  
  if (!text_content || strcmp(text_content, "") == 0)
    return;
  
  Message *text_message = create_public_text_from_message(client->username, text_content);
  char *json_str = to_json(text_message);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(text_message);
}

/* */
void private_text(Client *client, Message *incoming_message) {
  const char *text_content = get_text(incoming_message);
  const char *target_username = get_username(incoming_message);
	      
  if (!text_content || !target_username || strcmp(text_content, "") == 0 || strcmp(target_username, "") == 0)
    return;
  
  pthread_mutex_lock(&clients_mutex);
  Client *target_client = find_client_by_username(target_username);
  pthread_mutex_unlock(&clients_mutex);
  
  if (target_client) {
    Message *msg = create_text_from_message(client->username, text_content);
    char *json_str = to_json(msg);
    send_message(target_client, json_str);
    free(json_str);
    free_message(msg);
  } else {
    Message *response = create_response_message("TEXT", "NO_SUCH_USER", target_username);
    char *json_str = to_json(response);
    send_message(client, json_str);
    free(json_str);
    free_message(response);
  }
}

/* */
void users_list(Client *client, Message *incoming_message) {
  pthread_mutex_lock(&clients_mutex);
  
  int capacity = BACKLOG;
  int count = 0;
  char **users_list = malloc(sizeof(char *) * capacity);
  char **statuses = malloc(sizeof(char *) * capacity);
  
  Client *current = clients;
  while (current != NULL) {
    if (strlen(current->username) > 0) {
      if (count == capacity) {
        capacity *= 2;
        users_list = realloc(users_list, sizeof(char *) * capacity);
        statuses = realloc(statuses, sizeof(char *) * capacity);
      }
      users_list[count] = strdup(current->username);
      statuses[count] = strdup(current->status);
      count++;
    }
    current = current->next;
  }

  pthread_mutex_unlock(&clients_mutex);
  
  Message *list_message = create_users_list_message(users_list, statuses, count);
  char *json_str = to_json(list_message);
  send_message(client, json_str);

  for (int i = 0; i < count; i++) {
    free(users_list[i]);
    free(statuses[i]);
  }
  free(users_list);
  free(statuses);
  free(json_str);
  free_message(list_message);
}

/* */
void change_status(Client *client, Message *incoming_message) {
  const char *username = get_username(incoming_message);
  const char *new_status = get_status(incoming_message);
  
  Message *status_message = create_new_status_message(username, new_status);
  char *json_str = to_json(status_message);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(status_message);
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
    users_list(client, incoming_message);
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
    invite_guests(client, incoming_message);
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
    printf("[INFO] Client: [%s] requested a disconnection.\n", client->username);
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
