#include "room.h"
#include "server.h"

/* Linked list head for all chat rooms */
Room *rooms = NULL;
/* Mutex to protect access to the global rooms list */
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Removes empty rooms from the global room list.
 **/
void cleanup_empty_rooms()
{
  pthread_mutex_lock(&rooms_mutex);
  Room **prev = &rooms;
  Room *current = rooms;
  while (current != NULL) {
    if (current->client_count == 0) {
      Room *to_delete = current;
      *prev = current->next;
      current = current->next;
      free(to_delete->clients);
      free(to_delete);
    } else {
      prev = &current->next;
      current = current->next;
    }
  }
  pthread_mutex_unlock(&rooms_mutex);
}

/**
 * Sends a message to all members of a room except the sender.
 *
 * @param room Pointer to the target room.
 * @param message JSON-formatted message string to send.
 * @param sender_socket Socket descriptor of the sending client (to be excluded).
 **/
void broadcast_to_room(Room *room, const char *message, int sender_socket)
{
  if (!room)
    return;

  pthread_mutex_lock(&rooms_mutex);
  int count = room->client_count;
  Client **clients_copy = malloc(sizeof(Client*) * count);
  if (!clients_copy) {
    pthread_mutex_unlock(&rooms_mutex);
    return;
  }
  for (int i = 0; i < count; ++i)
    clients_copy[i] = room->clients[i];
  pthread_mutex_unlock(&rooms_mutex);
  
  for (int i = 0; i < count; ++i) {
    Client *client = clients_copy[i];
    if (client && client->socket_fd != sender_socket && !client->is_disconnected)
      send_message(client, message);
  }
  free(clients_copy);
}

/**
 * Checks if a user is a member of a given room.
 *
 * @param username The name of the user to check.
 * @param roomname The name of the room.
 * @return true if user is a member of the room, false otherwise.
 **/
bool is_member(const char *username, const char *roomname)
{
  pthread_mutex_lock(&rooms_mutex);
  Room *room = find_room(roomname);
  if (!room) {
    pthread_mutex_unlock(&rooms_mutex);
    return false;
  }
  for (int i = 0; i < room->client_count; ++i) {
    if (room->clients[i] && strcmp(room->clients[i]->username, username) == 0) {
      pthread_mutex_unlock(&rooms_mutex);
      return true;
    }
  }
  pthread_mutex_unlock(&rooms_mutex);
  return false;
}

/**
 * Finds a room by name.
 *
 * @param roomname The name of the room to find.
 * @return Pointer to the Room if found, NULL otherwise.
 **/
Room *find_room(const char *roomname)
{
  Room *current = rooms;
  while (current != NULL) {
    if (strcmp(current->roomname, roomname) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

/**
 * Removes a client from a room's client list.
 *
 * @param room Pointer to the room.
 * @param client Pointer to the client to remove.
 * @return true if the client was found and removed, false otherwise.
 **/
bool remove_client_from_room(Room *room, Client *client)
{
  if (!room || !client)
    return false;
    
  pthread_mutex_lock(&rooms_mutex);
  for (int i = 0; i < room->client_count; ++i) {
    if (room->clients[i] == client) {
      for (int j = i; j < room->client_count - 1; ++j)
        room->clients[j] = room->clients[j + 1];
      room->client_count--;
      pthread_mutex_unlock(&rooms_mutex);
      return true;
    }
  }
  pthread_mutex_unlock(&rooms_mutex);
  return false;
}

/**
 * Adds a client to a room, growing the list if needed.
 *
 * @param room Pointer to the room.
 * @param client Pointer to the client to add.
 * @return true on success, false on memory allocation failure.
 **/
bool add_client_to_room(Room *room, Client *client)
{
  if (!room || !client)
    return false;
  
  pthread_mutex_lock(&rooms_mutex);
  for (int i = 0; i < room->client_count; ++i) {
    if (room->clients[i] == client) {
      pthread_mutex_unlock(&rooms_mutex);
      return true; // client already in room
    }
  }
  if (room->client_count >= room->capacity) {
    int new_capacity = room->capacity * 2;
    Client **new_clients = realloc(room->clients, sizeof(Client *) * new_capacity);
    if (!new_clients) {
      pthread_mutex_unlock(&rooms_mutex);
      return false;
    }
    room->clients = new_clients;
    room->capacity = new_capacity;
  }
  room->clients[room->client_count++] = client;
  pthread_mutex_unlock(&rooms_mutex);
  return true;
}

/**
 * Creates a new room with the specified name.
 *
 * @param roomname Name of the room to create (max 16 characters).
 * @return Pointer to the newly created Room, or NULL on error or duplicate.
 **/
Room *create_room(const char *roomname)
{
  pthread_mutex_lock(&rooms_mutex);
  if (find_room(roomname) != NULL) {
    pthread_mutex_unlock(&rooms_mutex);
    return NULL;
  } 
  Room *room = malloc(sizeof(Room));
  if (!room) {
    pthread_mutex_unlock(&rooms_mutex);
    return NULL;
  }
  strncpy(room->roomname, roomname, sizeof(room->roomname) - 1);
  room->roomname[sizeof(room->roomname) - 1] = '\0';
  room->clients = malloc(sizeof(Client *) * 15);
  if (!room->clients) {
    free(room);
    pthread_mutex_unlock(&rooms_mutex);
    return NULL;
  }
  room->client_count = 0;
  room->capacity = 15;
  room->next = rooms;
  rooms = room;
  pthread_mutex_unlock(&rooms_mutex);
  return room;
}
