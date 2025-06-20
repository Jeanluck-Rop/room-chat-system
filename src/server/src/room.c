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
  pthread_mutex_lock(&rooms_mutex);

  for (int i = 0; i < room->client_count; ++i) {
    Client *client = room->clients[i];
    if (client->socket_fd != sender_socket && strlen(client->username) > 0)
      send_message(client, message);
  }
  
  pthread_mutex_unlock(&rooms_mutex);
}

/* */
void get_room_user_info(const char *roomname, char ***usernames, char ***statuses, int *count) {
  pthread_mutex_lock(&rooms_mutex);

  Room *room = find_room(roomname);
  if (!room) {
    pthread_mutex_unlock(&rooms_mutex);
    *count = 0;
    *usernames = NULL;
    *statuses = NULL;
    return;
  }

  int capacity = room->client_count;
  char **names = malloc(sizeof(char *) * capacity);
  char **stats = malloc(sizeof(char *) * capacity);
  int size = 0;

  for (int i = 0; i < room->client_count; ++i) {
    Client *c = room->clients[i];
    if (c && strlen(c->username) > 0) {
      names[size] = strdup(c->username);
      stats[size] = strdup(c->status);
      size++;
    }
  }

  pthread_mutex_unlock(&rooms_mutex);

  *usernames = names;
  *statuses = stats;
  *count = size;
}

/* */
char **get_room_users_list(const char *roomname, int *count) {
  pthread_mutex_lock(&rooms_mutex);

  Room *room = find_room(roomname);
  if (!room) {
    pthread_mutex_unlock(&rooms_mutex);
    *count = 0;
    return NULL;
  }

  char **usernames = malloc(sizeof(char *) * room->client_count);
  int size = 0;

  for (int i = 0; i < room->client_count; i++) {
    if (room->clients[i] && strlen(room->clients[i]->username) > 0) {
      usernames[size++] = strdup(room->clients[i]->username);
    }
  }

  pthread_mutex_unlock(&rooms_mutex);

  *count = size;
  return usernames;
}

bool is_member(const char *roomname, const char *username) {
  int count = 0;
  char **usernames = get_room_users_list(roomname, &count);
  if (!usernames)
    return false;

  for (int i = 0; i < count; i++) {
    if (strcmp(usernames[i], username) == 0) {
      for (int j = 0; j < count; j++)
        free(usernames[j]);
      free(usernames);
      return true;
    }
    free(usernames[i]);
  }
  free(usernames);
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
