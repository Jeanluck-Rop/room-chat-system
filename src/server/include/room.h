#ifndef ROOM_H
#define ROOM_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "server.h"

typedef struct Client Client;

/* Room struct to represent a chat room containing clients. */
typedef struct Room
{
  char roomname[17];   // Name of the room (maximum 16 chars + null terminator).
  Client **clients;    // Dynamic array of client pointers.
  int client_count;    // Number of clients currently in the room.
  int capacity;        // Maximum capacity of clients before resizing.
  struct Room *next;   // Pointer to the next room in the global list.
}
  Room;

/**
 * Removes empty rooms from the global room list.
 * Frees memory for rooms that have zero clients.
 * Thread-safe using rooms_mutex.
 **/
void cleanup_empty_rooms();

/**
 * Sends a message to all members of a room except the sender.
 * Thread-safe and avoids use-after-free by copying the client list first.
 *
 * @param room Pointer to the target room.
 * @param message JSON-formatted message string to send.
 * @param sender_socket Socket descriptor of the sending client (to be excluded).
 **/
void broadcast_to_room(Room *room, const char* message, int sender_socket);

/**
 * Checks if a user is a member of a given room.
 * Thread-safe read operation on the room list.
 *
 * @param username The name of the user to check.
 * @param roomname The name of the room.
 * @return true if user is a member of the room, false otherwise.
 **/
bool is_member(const char* username, const char* roomname);

/**
 * Finds a room by name.
 * This function is not thread-safe by itself, must be protected externally if needed.
 *
 * @param roomname The name of the room to find.
 * @return Pointer to the Room if found, NULL otherwise.
 **/
Room *find_room(const char* roomname);

/**
 * Removes a client from a room's client list.
 * Thread-safe. Compact the list after removal.
 *
 * @param room Pointer to the room.
 * @param client Pointer to the client to remove.
 * @return true if the client was found and removed, false otherwise.
 **/
bool remove_client_from_room(Room *room, Client *client);

/**
 * Adds a client to a room, growing the list if needed.
 * Thread-safe. Automatically reallocates memory if capacity is full.
 *
 * @param room Pointer to the room.
 * @param client Pointer to the client to add.
 * @return true on success, false on memory allocation failure.
 **/
bool add_client_to_room(Room *room, Client *client);

/**
 * Creates a new room with the specified name.
 * Thread-safe. Fails if a room with the same name already exists.
 *
 * @param roomname Name of the room to create (max 16 characters).
 * @return Pointer to the newly created Room, or NULL on error or duplicate.
 **/
Room *create_room(const char* roomname);

#endif // ROOM_H
