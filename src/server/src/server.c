#include "server.h"
#include "room.c"

/* Maximum of queued connections */
#define BACKLOG 20
/* Linked list head for all connected clients */
Client *clients = NULL;
/* Server socket file descriptor */
static int server_fd = -1;
/* Mutex to protect access to the global clients list */
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
/* Mutex to protect access to the invited_rooms list of clients */
pthread_mutex_t invitations_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Print a message with a specified type (info, alert, or error).
 *
 * @param text The message to display.
 * @param type Message type: 'i' for info, 'a' for alert, 'e' for error (causes exit).
 **/
static void
print_message(const char* text,
	      char type)
{
  if (type == 'e') {
    perror(text);  
    exit(EXIT_FAILURE);
  }
  else if (type == 'a')
    printf("[ALERT]: %s\n", text);
  else
    printf("[INFO]: %s\n", text);  
}



/* */
static int
open_listener(int port)
{
  int server_sd;
  struct sockaddr_in server_addr;
  //Create the server socket
  print_message("Creating the server socket...", 'i');
  server_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sd == -1)
    print_message("[ERROR]: Error creating socket\n", 'e');
  bzero(&server_addr, sizeof(server_addr));
  //Set server address
  server_addr.sin_family = AF_INET;         //Address-family IPv4
  server_addr.sin_addr.s_addr = INADDR_ANY; //Accept connections on all interfaces
  server_addr.sin_port = htons(port);       //Convert port number to network byte order

  if (bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    print_message("[ERROR]: Socket cannot be associated to the port.\n", 'e');
    abort();
  }
  
  if (listen(server_sd, BACKLOG) == -1) {
    print_message("[ERROR]: Error listening port.\n", 'e');
    abort();
  } else
    print_message("Server is now listening for incoming connections.", 'i');
    
  return server_sd;
}

/* */
static void
load_certificates(SSL_CTX* ctx,
		  char* CertFile,
		  char* KeyFile)
{
  if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    abort();
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    abort();
  }
  if (!SSL_CTX_check_private_key(ctx)) {
    print_message("[ERROR]: Private key does not match the public certificate\n", 'e');
    abort();
  }
}

/* */
static SSL_CTX*
init_serverCTX(void)
{
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  method = TLS_server_method();
  ctx = SSL_CTX_new(method);

  if (ctx == NULL) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  return ctx;
}



/**
 * Handle SIGINT (Ctrl+C) signal for graceful shutdown.
 * Closes the server socket if open and exits the program.
 *
 * @param sig Signal number (unused).
 **/
static void
handle_sigint(int sig)
{
  if (server_fd != -1) {
    close(server_fd);
    print_message("Server socket closed due to SIGINT (Ctrl+C).", 'i');
  }
  (void)sig;
  exit(0);
}

/**
 * Return the number of all the clients connected to the chat.
 **/
static int
get_all_clients_count()
{
  int count = 0;
  pthread_mutex_lock(&clients_mutex);
  Client *current = clients;
  while (current != NULL) {
    if (!current->is_disconnected)
      count++;
    current = current->next;
  }
  pthread_mutex_unlock(&clients_mutex);
  return count;
}

/**
 * Sends a message to a specific client.
 *
 * @param client Pointer to the target client.
 * @param message The message string to send.
 **/
void
send_message(Client *client,
	     const char* message)
{
  if (!client || client->is_disconnected)
    return;

  size_t total_sent = 0;
  size_t msg_len = strlen(message);
  while (total_sent < msg_len) {
    int sent = SSL_write(client->ssl, message + total_sent, msg_len - total_sent);
      if (sent <= 0) {
	int err = SSL_get_error(client->ssl, sent);
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "Failed to send message to client [%s].\n SSL error code: %d\n", client->username, err);
	print_message(buffer, 'e');
	ERR_print_errors_fp(stderr);
	break;
      }
    total_sent += sent;
  }
}

/**
 * Broadcast a message to all connected clients except the sender.
 *
 * @param message The message to broadcast.
 * @param sender_socket The socket file descriptor of the sender (to be excluded).
 **/
static void
broadcast_message(const char* message,
		  int sender_socket)
{
  pthread_mutex_lock(&clients_mutex);
  Client *client = clients;
  while (client != NULL) {
    if (client->socket_fd != sender_socket && strlen(client->username) > 0)
      send_message(client, message);
    client = client->next;
  }
  pthread_mutex_unlock(&clients_mutex);
}

/**
 * Check if a client was invited to a specific room.
 *
 * @param client Pointer to the client.
 * @param roomname Name of the room to check.
 * @return true if the client is invited, false otherwise.
 **/
static bool
was_invited(Client *client,
	    const char* roomname)
{
  if (!client || !roomname)
    return false;

  pthread_mutex_lock(&invitations_mutex);
  for (int i = 0; i < client->invited_count; i++)
    if (client->invited_rooms[i] && strcmp(client->invited_rooms[i], roomname) == 0) {
      pthread_mutex_unlock(&invitations_mutex);
      return true;
    }
  pthread_mutex_unlock(&invitations_mutex);
  return false;
}

/**
 * @brief Remove a room invitation from a client's invited list.
 *
 * Frees the room entry and compacts the array by replacing
 * the removed room with the last in the list.
 *
 * @param client Pointer to the client.
 * @param roomname Name of the room to remove from invitations.
 **/
static void
unmark_as_invited(Client *client,
		  const char* roomname)
{
  if (!client || !roomname)
    return;

  pthread_mutex_lock(&invitations_mutex);
  if (!client->invited_rooms) {
    pthread_mutex_unlock(&invitations_mutex);
    return;
  }
  for (int i = 0; i < client->invited_count; ++i) {
    if (strcmp(client->invited_rooms[i], roomname) == 0) {
      free(client->invited_rooms[i]);
      client->invited_rooms[i] = client->invited_rooms[client->invited_count - 1];
      client->invited_count--;
      break;
    }
  }
  pthread_mutex_unlock(&invitations_mutex);
}

/**
 * Mark a client as invited to a specific room.
 *
 * @param client Pointer to the client.
 * @param roomname Name of the room to mark.
 * @return true on success, false if memory allocation fails.
 **/
static bool
mark_as_invited(Client *client,
		const char* roomname)
{
  if (!client || !roomname)
    return false;
  
  pthread_mutex_lock(&invitations_mutex);
  for (int i = 0; i < client->invited_count; i++) {
    if (client->invited_rooms[i] && strcmp(client->invited_rooms[i], roomname) == 0) {
      pthread_mutex_unlock(&invitations_mutex);
      return true;
    }
  }
  if (client->invited_capacity == 0) {
    client->invited_capacity = 4;
    client->invited_rooms = malloc(sizeof(char *) * client->invited_capacity);
    if (!client->invited_rooms) {
      print_message("[ERROR]: malloc failed in mark_as_invited", 'e');
      pthread_mutex_unlock(&invitations_mutex);
      return false;
    }
  } else if (client->invited_count == client->invited_capacity) {
    int new_capacity = client->invited_capacity * 2;
    char **new_rooms = realloc(client->invited_rooms, sizeof(char *) * new_capacity);
    if (!new_rooms) {
      pthread_mutex_unlock(&invitations_mutex);
      print_message("realloc failed in mark_as_invited", 'e');
      return false;
    }
    client->invited_capacity = new_capacity;
    client->invited_rooms = new_rooms;
  }
  client->invited_rooms[client->invited_count++] = strdup(roomname);
  pthread_mutex_unlock(&invitations_mutex);
  
  return true;
}

/**
 * Finds a connected client by username.
 *
 * @param username The username to look for.
 * @return Pointer to the Client structure if found, NULL otherwise.
 **/
static Client*
find_client(const char* username)
{
  Client *current = clients;
  while (current != NULL) {
    if (strcmp(current->username, username) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

/**
 * Sends a JSON message to a client based on type and content.
 *
 * @param client Destination client.
 * @param type Short message type: "PT" or "IV".
 * @param username Sender's username.
 * @param content Text or room name, depending on the message type.
 **/
static void
send_json(Client *client,
	  const char* type,
	  const char* username,
	  const char* content)
{
  Message *message = NULL;
  if (strcmp(type, "PT") == 0)
    message = create_text_from_message(username, content);
  else if (strcmp(type, "IV") == 0)
    message = create_invite_message(username, content);
  char *json_str = to_json(message);
  send_message(client, json_str);
  free(json_str);
  free_message(message);
}

/**
 * Broadcasts a specific type of message constructed from a username and content.
 *
 * @param client The client to exclude from broadcast.
 * @param type Message type: "ID", "ST", or "PT".
 * @param username Sender username.
 * @param content Status or message text (depending on type).
 **/
static void
broadcast_json(Client *client,
	       const char* type,
	       const char* username,
	       const char* content)
{
  Message *message = NULL;
  if (strcmp(type, "ID") == 0)
    message = create_new_user_message(username);
  else if (strcmp(type, "ST") == 0)
    message = create_new_status_message(username, content);
  else if (strcmp(type, "PT") == 0)
    message = create_public_text_from_message(username, content);
  char *json_str = to_json(message);
  broadcast_message(json_str, client->socket_fd);
  free(json_str);
  free_message(message);
}

/**
 * Broadcasts a room-related JSON message to all members, excluding sender.
 *
 * @param client Sender (excluded from broadcast).
 * @param room Target room.
 * @param type Message type: "JN" or "RT".
 * @param roomname Name of the room.
 * @param username Username of the sender.
 * @param content Optional message text.
 **/
static void
broadcast_room_json(Client *client,
		    Room *room,
		    const char* type,
		    const char* roomname,
		    const char* username,
		    const char* content)
{
  Message *message = NULL;
  if (strcmp(type, "JN") == 0)
    message = create_joined_room_message(roomname, username);
  else if (strcmp(type, "RT") == 0)
    message = create_room_text_from_message(roomname, username, content);
  else if (strcmp(type, "LR") == 0)
    message = create_left_room_message(roomname, username);
  char *json_str = to_json(message);
  broadcast_to_room(room, json_str, client->socket_fd);
  free(json_str);
  free_message(message);
}

/**
 * Sends an "INVALID" response message to a client.
 *
 * @param client Target client.
 * @param result Reason for invalid response.
 **/
static void
invalid_response(Client *client,
		 const char* result)
{
  Message *response = create_response_message("INVALID", result, "", 0);
  char *json_str = to_json(response);
  send_message(client, json_str);
  free(json_str);
  free_message(response);
}

/**
 * Sends the respective "RESPONSE" message to a client.
 *
 * @param client Target client.
 * @param operation Type of operation (e.g., "NEW_ROOM").
 * @param result Result of the operation (e.g., "SUCCESS", "FAILED").
 * @param extra The extra information.
 **/
static void
response(Client *client,
	 const char* operation,
	 const char* result,
	 const char* extra,
	 int count)
{
  Message *response = create_response_message(operation, result, extra, count);
  char *json_str = to_json(response);
  send_message(client, json_str);
  free(json_str);
  free_message(response);
}

/**
 * Removes a client from a room and notifies the room members.
 *
 * @param client Pointer to the client leaving the room.
 * @param room Pointer to the room the client is leaving.
 **/
static void
remove_room_client(Client *client,
		   Room *room)
{
  if (!remove_client_from_room(room, client))
    return;
  unmark_as_invited(client, room->roomname);
  broadcast_room_json(client, room, "LR", room->roomname, client->username, NULL);
}

/**
 * Disconnects a client, leaving all rooms and removing them from the server.
 * Ensuring thread-safe disconnection and room cleanup.
 *
 * @param client Pointer to the client to disconnect.
 */
static void
disconnect_client(Client *client)
{
  if (!client)
    return;
  
  /* 1. Protection added for avoiding multiple disconnections */
  static pthread_mutex_t disconnect_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&disconnect_mutex);
  if (client->is_disconnected) {
    pthread_mutex_unlock(&disconnect_mutex);
    return;
  }
  client->is_disconnected = true;
  pthread_mutex_unlock(&disconnect_mutex);
  /* 2. Get client rooms */
  pthread_mutex_lock(&rooms_mutex);
  Room *rooms_copy = NULL;
  Room *current_room = rooms;
  while (current_room) {
    for (int i = 0; i < current_room->client_count; ++i) {
      if (current_room->clients[i] == client) {
	Room *copy = malloc(sizeof(Room));
	if (!copy)
	  continue;
	strncpy(copy->roomname, current_room->roomname, sizeof(copy->roomname));
	copy->next = rooms_copy;
	rooms_copy = copy;
	break;
      }
    }
    current_room = current_room->next;
  }
  pthread_mutex_unlock(&rooms_mutex);
  /* 3. Leave each client rooms */
  while (rooms_copy) {
    Room *next = rooms_copy->next;
    Room *actual_room = find_room(rooms_copy->roomname);
    if (actual_room)
      remove_room_client(client, actual_room);
    free(rooms_copy);
    rooms_copy = next;
  }
  /* 4. Notify client disconnection */
  if (strlen(client->username) > 0) {
    Message *client_disconnected = create_disconnected_message(client->username);
    char *json_str = to_json(client_disconnected);
    broadcast_message(json_str, client->socket_fd);
    free(json_str);
    free_message(client_disconnected);
  }
  /* 5. Remove the client from the list */
  pthread_mutex_lock(&clients_mutex);
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
  /* 6. Free invitations memory */
  pthread_mutex_lock(&invitations_mutex);
  for (int i = 0; i < client->invited_count; ++i)
    free(client->invited_rooms[i]);
  free(client->invited_rooms);
  client->invited_count = 0;
  client->invited_capacity = 0;
  client->invited_rooms = NULL;
  pthread_mutex_unlock(&invitations_mutex);
  /* 7. Close client socket and free client */
  close(client->socket_fd);
  free(client);
  cleanup_empty_rooms();
}

/**
 * Handles a client's request to leave a room.
 *
 * @param client Pointer to the client leaving the room.
 * @param incoming_message Message containing the room name.
 **/
static void
leave_room(Client *client,
	   Message *incoming_message)
{
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0) {
    response(client, "LEAVE_ROOM", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to leave an invalid room.\n", client->username);
    return;
  }
  
  Room *room_to_leave = find_room(roomname);
  if (!room_to_leave) {
    response(client, "LEAVE_ROOM", "NO_SUCH_ROOM", roomname, 0);
    printf("[INFO] Client [%s] tried to leave a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }
  if (!is_member(client->username, roomname)) {
    response(client, "LEAVE_ROOM", "NOT_JOINED", roomname, 0);
    printf("[INFO] Client [%s] tried to leave a room [%s] that is not member.\n", client->username, roomname);
    return;
  }
  remove_room_client(client, room_to_leave);
  cleanup_empty_rooms();
}

/**
 * Sends a text message to all users in a specified room.
 *
 * @param client The sender client.
 * @param incoming_message Message containing text and target room.
 **/
static void
send_room_text(Client *client,
	       Message *incoming_message)
{
  cleanup_empty_rooms();
  const char *roomname = get_roomname(incoming_message);
  const char *text_content = get_text(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0 || !text_content || strcmp(text_content, "") == 0) {
    response(client, "ROOM_TEXT", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to send an invalid text or send a text to an invalid room.\n", client->username);
    return;
  }
  
  Room *target_room = find_room(roomname);
  if (!target_room) {
    response(client, "ROOM_TEXT", "NO_SUCH_ROOM", roomname, 0);
    printf("[INFO] Client [%s] tried to send text to a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }
  if (!is_member(client->username, roomname)) {
    response(client, "ROOM_TEXT", "NOT_JOINED", roomname, 0);
    printf("[INFO] Client [%s] tried to send text to a room [%s] that is not member.\n", client->username, roomname);
    return;
  }
  broadcast_room_json(client, target_room, "RT", roomname, client->username, text_content);
}

/**
 * Sends the list of users in a room to the requesting client.
 *
 * @param client Requesting client.
 * @param incoming_message Message containing the room name.
 **/
static void
send_room_users(Client *client,
		Message *incoming_message)
{
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0) {
    response(client, "ROOM_USERS", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to invite users an invalid room.\n", client->username);
    return;
  }
  
  Room *target_room = find_room(roomname);
  if (!target_room) {
    response(client, "ROOM_USERS", "NO_SUCH_ROOM", roomname, 0);
    printf("[INFO] Client [%s] tried to get room users from a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }
  if (!is_member(client->username, roomname)) {
    response(client, "ROOM_USERS", "NOT_JOINED", roomname, 0);
    printf("[INFO] Client [%s] tried to get room users from a room [%s] that is not member.\n", client->username, roomname);
    return;
  }
  
  pthread_mutex_lock(&rooms_mutex);
  int count = target_room->client_count;
  char **usernames = malloc(sizeof(char *) * count);
  char **statuses = malloc(sizeof(char *) * count);
  for (int i = 0; i < count; ++i) {
    Client *room_client = target_room->clients[i];
    usernames[i] = strdup(room_client->username);
    statuses[i] = strdup(room_client->status);
  }
  pthread_mutex_unlock(&rooms_mutex);

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

/**
 * Handles a client's request to join a room.
 *
 * @param client The client joining the room.
 * @param incoming_message Message containing the room name.
 **/
static void
join_room(Client *client,
	  Message *incoming_message)
{
  cleanup_empty_rooms();
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0) {
    response(client, "JOIN_ROOM", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to join an invalid room.\n", client->username);
    return;
  }
  
  Room *room_to_join = find_room(roomname);
  if (!room_to_join) {
    response(client, "JOIN_ROOM", "NO_SUCH_ROOM", roomname, 0);
    printf("[INFO] Client [%s] tried to join a room [%s] that does not exist.\n", client->username, roomname);
    return;
  }
  if (!was_invited(client, roomname)) {
    response(client, "JOIN_ROOM", "NOT_INVITED", roomname, 0);
    printf("[INFO] Client [%s] tried to get room users from a room [%s] that was not invited.\n", client->username, roomname);
    return;
  }
  if(is_member(client->username, roomname)) {
    response(client, "JOIN_ROOM", "ALREADY_MEMBER", roomname, 0);
    printf("[INFO]: Client [%s] is already member of the room [%s].\n", client->username, roomname);
    return;
  }
  if (!add_client_to_room(room_to_join, client)) {
    response(client, "JOIN_ROOM", "ERROR_JOINING", roomname, 0);
    printf("[ALERT] Could not add [%s] to requested room [%s].\n", client->username, roomname);
    return;
  }
  int count = get_room_clients_count(room_to_join);
  response(client, "JOIN_ROOM", "SUCCESS", roomname, count);
  printf("[INFO]: Client [%s] successfully joined to the room [%s].\n", client->username, roomname);
  broadcast_room_json(client, room_to_join, "JN", roomname, client->username, NULL);
}

/**
 *
 * Sends invitations to other users to join a room.
 * @param client Client sending invitations.
 * @param incoming_message Message with room name and guest list.
 **/
static void
send_invitation(Client *client,
		Message *incoming_message)
{
  cleanup_empty_rooms();
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0) {
    response(client, "INVITE", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to invite users to an invalid room.\n", client->username);
    return;
  }
  
  Room *target_room = find_room(roomname);
  if (!target_room) {
    response(client, "INVITE", "NO_SUCH_ROOM", roomname, 0);
    printf("[INFO] Client [%s] tried to invite users to an inexisting room.\n", client->username);
    return;
  }
  if (!is_member(client->username, roomname)) {
    response(client, "INVITE", "NOT_JOINED", roomname, 0);
    printf("[INFO] Client [%s] tried to invite users to a room [%s] which he is not member.\n", client->username, roomname);
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
    Client *exists = find_client(guests_list[i]);
    if (!exists) {
      response(client, "INVITE", "NO_SUCH_USER", guests_list[i], 0);
      printf("[INFO]: Client [%s] tried to invite an non existing user [%s] to the room [%s].\n", client->username, exists->username, roomname);
      continue;
    }
    if (strcmp(exists->username, client->username) == 0) {
      response(client, "INVITE", "SELF_INVITE", guests_list[i], 0);
      printf("[INFO]: Client [%s] tried to invite himself to the room [%s].\n", client->username, roomname);
      continue;
    }
    if (was_invited(exists, roomname) || is_member(exists->username, roomname)) {
      response(client, "INVITE", "ALREADY_MEMBER_OR_INVITED", guests_list[i], 0);
      printf("[INFO]: Client [%s] is already invited or member of [%s].\n", exists->username, roomname);
      continue;
    }
    if (!mark_as_invited(exists, roomname)) {
      response(client, "INVITE", "ERROR_MARKING", guests_list[i], 0);
      printf("[ERROR]: Could not mark [%s] as invited to room [%s].\n", exists->username, roomname);
      continue;
    }
    send_json(exists, "IV", client->username, roomname);
  }
  pthread_mutex_unlock(&clients_mutex);

  for (int i = 0; i < guest_count; i++)
    free(guests_list[i]);
  free(guests_list);
}

/**
 * Creates a new chat room if it doesn't exist and adds the creator to it.
 *
 * @param client Pointer to the client requesting room creation.
 * @param incoming_message Parsed message containing room information.
 **/
static void
create_new_room(Client *client,
		Message *incoming_message)
{
  cleanup_empty_rooms();
  const char *roomname = get_roomname(incoming_message);
  if (!roomname || strcmp(roomname, "") == 0) {
    response(client, "NEW_ROOM", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to create a room with an invalid name.\n", client->username);
    return;
  }
  
  Room *new_room = create_room(roomname);
  if (!new_room) {
    response(client, "NEW_ROOM", "ROOM_ALREADY_EXISTS", roomname, 0);
    printf("[INFO]: Client [%s] tried to create and existing room.\n", client->username);
    return;
  }
  if (!add_client_to_room(new_room, client)) {
    response(client, "NEW_ROOM", "ERROR_JOINING", roomname, 0);
    printf("[ALERT]: Could not add [%s] to new room created.\n", client->username);
    cleanup_empty_rooms();
    return;
  }
  if (!mark_as_invited(client, roomname)) {
    response(client, "INVITE", "ERROR_MARKING", client->username, 0);
    printf("[ALERT]: Could not mark [%s] as invited to the new room [%s].\n", client->username, roomname);
    return;
  }
  response(client, "NEW_ROOM", "SUCCESS", roomname, 0);
  printf("[INFO]: Room [%s] successfully created by the client [%s].\n", roomname, client->username);
}

/**
 * Broadcasts a public chat message to all clients except the sender.
 *
 * @param client Sender client.
 * @param incoming_message Message containing the public text.
 **/
static void
send_public_text(Client *client,
		 Message *incoming_message)
{
  const char *text_content = get_text(incoming_message);
  if (!text_content || strcmp(text_content, "") == 0) {
    response(client, "PUBLIC_TEXT", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to send an invalid public text.\n", client->username);
    return;
  }
  broadcast_json(client, "PT", client->username, text_content);
}

/**
 * Sends a private message from one client to another.
 *
 * @param client Sender client.
 * @param incoming_message Message with target username and content.
 **/
static void
send_private_text(Client *client,
		  Message *incoming_message)
{
  const char *text_content = get_text(incoming_message);
  const char *target_username = get_username(incoming_message);
  if (!text_content || !target_username || strcmp(text_content, "") == 0 || strcmp(target_username, "") == 0) {
    response(client, "TEXT", "INVALID", "", 0);
    printf("[INFO] Client [%s] tried to text an invalid user or send an invalid private text.\n", client->username);
    return;
  }
  
  pthread_mutex_lock(&clients_mutex);
  Client *target_client = find_client(target_username);
  pthread_mutex_unlock(&clients_mutex);

  if (!target_client) {
    response(client, "TEXT", "NO_SUCH_USER", target_username, 0);
    printf("[INFO]: Client [%s] tried to send a private text to an non existing user [%s].\n", client->username, target_client->username);
    return;
  }
  send_json(target_client, "PT", client->username, text_content);
}

/**
 * Sends a list of all connected users and their statuses to a client.
 *
 * @param client Requesting client.
 * @param incoming_message Unused, but included for consistency.
 **/
static void
send_users_list(Client *client,
		Message *incoming_message)
{
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

/**
 * Changes the status of a client and notifies all others.
 *
 * @param client Client who is changing status.
 * @param incoming_message Message containing the new status.
 **/
static void
change_status(Client *client,
	      Message *incoming_message)
{
  const char *new_status = get_status(incoming_message);
  if (!new_status) {
    response(client, "STATUS", "INVALID", "", 0);
    printf("[INFO] Client [%s] sent an invalid change status request.\n", client->username);
    return;
  }

  printf("[INFO]: Client [%s] changed his status to [%s].\n", client->username, new_status);
  strncpy(client->status, new_status, sizeof(client->status) - 1);
  client->status[sizeof(client->status) - 1] = '\0';
  broadcast_json(client, "ST", client->username, new_status);
}

/**
 * Validate and set client identity.
 *
 * @param client Client to identify.
 * @param incoming_message Message containing identification details.
 * @return true if identification is successful, false otherwise.
 **/
static bool
check_identify(Client *client,
	       Message *incoming_message)
{
  const MessageType type = get_type(incoming_message);
  if (type != IDENTIFY)
    return false;
  const char *username = get_username(incoming_message);
  if (strcmp(username, "") == 0)
    return false;
  
  pthread_mutex_lock(&clients_mutex);
  Client *current = clients;
  while (current != NULL) {
    if (strcmp(current->username, username) == 0) {
      pthread_mutex_unlock(&clients_mutex);
      response(client, "IDENTIFY", "USER_ALREADY_EXISTS", username, 0);
      return false;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&clients_mutex);
  
  //At this point, the username is valid and then we assign it to the client.
  int count = get_all_clients_count();//the current clients connected
  strncpy(client->username, username, sizeof(client->username) - 1);
  client->username[sizeof(client->username) - 1] = '\0';
  strncpy(client->status, "ACTIVE", sizeof(client->status) - 1); //Default client status
  client->status[sizeof(client->status) - 1] = '\0';
  response(client, "IDENTIFY", "SUCCESS", "", count);
  printf("[INFO]: Client [%s] connected and identified.\n", client->username);
  broadcast_json(client, "ID", client->username, NULL);
  return true;
}

/**
 * Routes a message to the appropriate handler based on its type.
 *
 * @param client Pointer to the client.
 * @param incoming_message The parsed incoming message.
 * @return true if the client should remain connected, false otherwise.
 * @return false if the message is a DISCONNECT or invalid.
 **/
static bool
client_actions(Client *client,
	       Message *incoming_message)
{
  const MessageType type = get_type(incoming_message);

  switch (type) {
  case STATUS:
    change_status(client, incoming_message);
    break;
  case USERS:
    send_users_list(client, incoming_message);
    break;
  case TEXT:
    send_private_text(client, incoming_message);
    break;
  case PUBLIC_TEXT:
    send_public_text(client, incoming_message);
    break;
  case NEW_ROOM:
    create_new_room(client, incoming_message);
    break;
  case INVITE:
    send_invitation(client, incoming_message);
    break;
  case JOIN_ROOM:
    join_room(client, incoming_message);
    break;
  case ROOM_USERS:
    send_room_users(client, incoming_message);
    break;
  case ROOM_TEXT:
    send_room_text(client, incoming_message);
    break;
  case LEAVE_ROOM:
    leave_room(client, incoming_message);
    break;
  case DISCONNECT:
    printf("[INFO]: Client [%s] disconnected.\n", client->username);
    return false;
  default:
    invalid_response(client, "INVALID");
    printf("[INFO]: Invalid message received from the client [%s], disconnecting it.\n", client->username);
    return false;
  }
  
  return true;
}

/**
 * Thread function to handle the communication with a connected client.
 *
 * @param arg A pointer to a Client structure containing the client's state and socket.
 * @return NULL Return NULL when the client is disconnected or an error occurs.
 **/
static void*
handle_client(void *arg)
{
  Client *client = (Client *)arg;
  char buffer[1024];
  int received_bytes;
  bool identified = false;
  bool is_connected = true;
  
  while (is_connected) {
    received_bytes = SSL_read(client->ssl, buffer, sizeof(buffer) - 1);
    
    if (received_bytes > 0) {
      buffer[received_bytes] = '\0'; //Null-terminate for the received string
      Message *incoming_msg = parse(buffer);
      if (!incoming_msg) {
	invalid_response(client, "INVALID");
	printf("[INFO]: Invalid message received from the client [%s], disconnecting it.", client->username);
	break;
      }

      if (!identified) {
	identified = check_identify(client, incoming_msg);
	if (!identified) {
	  print_message("Disconnecting unidentified client.", 'i');
	  invalid_response(client, "NOT_IDENTIFIED");
	  free_message(incoming_msg);
	  break;
	}
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
      else {
	print_message("Fail receiving client data.", 'e');
	ERR_print_errors_fp(stderr);
      }
      break;
    }
  }

  disconnect_client(client);
  return NULL;
}

/**
 * Accept and manage incoming client connections in an infinite loop.
 *
 * @param server_fd File descriptor of the server socket returned by socket().
 **/
static void
server_cycle(SSL_CTX *ctx)
{
  int client_fd;
  socklen_t client_len;
  struct sockaddr_in client_addr;
  
  while (1) {
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len); //Accept incoming connection
    
    if (client_fd < 0) {
      print_message("[ERROR]: Server accept failed.", 'e');
      continue;
    }
    
    //Allocate memory and initialize new client
    Client *client = (Client *)malloc(sizeof(Client));
    if (!client) {
      print_message("[ERROR]: Could not allocate memory for the client.", 'e');
      close(client_fd);
      continue;
    }

    //Set default values for client
    client->socket_fd = client_fd;
    
    client->ssl = SSL_new(ctx);
    if (!client->ssl) {
      ERR_print_errors_fp(stderr);
      close(client_fd);
      free(client);
      continue;
    }
    SSL_set_fd(client->ssl, client_fd);
    if (SSL_accept(client->ssl) <= 0) {
      ERR_print_errors_fp(stderr);
      SSL_free(client->ssl);
      close(client_fd);
      free(client);
      continue;
    }
	
    client->username[0] = '\0'; 
    client->invited_count = 0;
    client->invited_capacity = 0;
    client->invited_rooms = NULL;
    client->is_disconnected = false;
    client->next = NULL;
    
    print_message("New client connected.", 'i');
    
    //Add client to the client list
    pthread_mutex_lock(&clients_mutex);
    client->next = clients;
    clients = client;
    pthread_mutex_unlock(&clients_mutex);
    //We create a thread to handle the client
    if (pthread_create(&client->thread, NULL, handle_client, client) != 0) {
      print_message("Could not create client thread.", 'e');
      pthread_mutex_lock(&clients_mutex);
      clients = clients->next;
      pthread_mutex_unlock(&clients_mutex);
      SSL_free(client->ssl);
      close(client_fd);
      free(client);
      continue;
    }
    pthread_detach(client->thread); //Auto-cleanup when thread exits
  }
}
  
/**
 * Function to initialize and start the server on the specified port.
 *
 * @param port Port number on which the server will listen for incoming connections.
 **/
void
start_server(int port)
{
  signal(SIGINT, handle_sigint);
  
  SSL_CTX *ctx;
  SSL_library_init();
  ctx = init_serverCTX();
  load_certificates(ctx, "certi.pem", "certi.pem");
  server_fd = open_listener(port);
  
  //Start server life cycle
  server_cycle(ctx);
  
  //Closing server
  close(server_fd);
  SSL_CTX_free(ctx);
}
