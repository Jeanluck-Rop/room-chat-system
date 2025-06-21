#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "room.h"
#include "server.h"
#include "message.h"

Room *rooms = NULL;

pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

/* */
void cleanup_empty_rooms() {
  pthread_mutex_lock(&rooms_mutex);
  
  // Remove the room from the list
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

/* */
void broadcast_to_room(Room *room, const char *message, int sender_socket) {
  if (!room)
    return;

  pthread_mutex_lock(&rooms_mutex);

  int count = room->client_count;
  Client **clients_copy = malloc(sizeof(Client*) * count);

  for (int i = 0; i < count; ++i)
    clients_copy[i] = room->clients[i]; //copy reference

  pthread_mutex_unlock(&rooms_mutex);
    
  for (int i = 0; i < count; ++i) {
    Client *client = clients_copy[i];
    if (client && client->socket_fd != sender_socket && !client->is_disconnected)
      send_message(client, message);
  }

  free(clients_copy);
}

/* */
void leave_room(Client *client, Room *room) {
  if (!remove_client_from_room(room, client))
    return;

  unmark_as_invited(client, room->roomname);

  Message *left_message = create_left_room_message(room->roomname, client->username);
  char *json_str = to_json(left_message);
  broadcast_to_room(room, json_str, client->socket_fd);
  free(json_str);
  free_message(left_message);
}

/* */
bool is_member(const char *username, const char *roomname) {
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

/* */
Room *find_room(const char *roomname) {
  Room *current = rooms;

  while (current != NULL) {
    if (strcmp(current->roomname, roomname) == 0)
      return current;
    current = current->next;
  }
  
  return NULL;
}

/* */
bool remove_client_from_room(Room *room, Client *client) {
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

/* */
bool add_client_to_room(Room *room, Client *client) {
  pthread_mutex_lock(&rooms_mutex);

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

/* */
Room *create_room(const char *roomname) {
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
  room->client_count = 0;
  room->capacity = 15;
  room->next = rooms;
  rooms = room;
  pthread_mutex_unlock(&rooms_mutex);
  return room;
}
