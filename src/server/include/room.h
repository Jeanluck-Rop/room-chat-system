#ifndef ROOM_H
#define ROOM_H

#include "server.h"
#include "message.h"

typedef struct Client Client;

typedef struct Room {
  char roomname[17];
  Client **clients;
  int client_count;
  int capacity;
  struct Room *next;
} Room;

void cleanup_empty_rooms();
void broadcast_to_room(Room *room, const char *message, int sender_socket);
void leave_room(Client *client, Room *room);
bool is_member(const char *username, const char *roomname);
Room *find_room(const char *roomname);
bool remove_client_from_room(Room *room, Client *client);
bool add_client_to_room(Room *room, Client *client);
Room *create_room(const char *roomname);

#endif // ROOM_H
