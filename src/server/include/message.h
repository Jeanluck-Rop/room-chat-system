#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"

/* Enum for handle the type of the json message protocol */
typedef enum
{
  IDENTIFY,
  STATUS,
  USERS,
  TEXT,
  PUBLIC_TEXT,
  NEW_ROOM,
  INVITE,
  JOIN_ROOM,
  ROOM_USERS,
  ROOM_TEXT,
  LEAVE_ROOM,
  DISCONNECT,
  UNKNOWN
}
  MessageType;

/* Message struc, wrapper for encapsulate a JSON object */
typedef struct
{
  cJSON *json_data; //Internal cJSON object holding message content.
}
  Message;

/**
 * Gets the message type from the "type" field.
 * Maps string value to corresponding enum `MessageType`.
 *
 * @param msg Message pointer.
 * @return MessageType enum value.
 **/
MessageType get_type(const Message *msg);

/**
 * Serializes a Message to a JSON string.
 * Returned string must be freed by the caller.
 *
 * @param msg Message to serialize.
 * @return New string containing compact JSON.
 **/
char* to_json(const Message *msg);

/**
 * Extracts the "username" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char* get_username(const Message *msg);

/**
 * Extracts the "text" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char* get_text(const Message *msg);

/**
 * Extracts the "status" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char* get_status(const Message *msg);

/**
 * Extracts the "roomname" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char* get_roomname(const Message *msg);

/**
 * Extracts a list of usernames from a message.
 *
 * @param msg Message containing a "usernames" array.
 * @param size Output parameter for number of usernames.
 * @return Allocated array of strdup'd usernames.
 **/
char** get_users(const Message *msg, int *size);


/**
 * Parses a raw JSON string into a Message.
 *
 * @param raw_message A null-terminated JSON string.
 * @return Allocated Message or NULL if parsing fails.
 */
Message *parse(const char* raw_message);

/**
 * Creates a message announcing a new connected user.
 *
 * @param username The new user's username.
 * @return Allocated Message instance.
 **/
Message *create_new_user_message(const char* username);

/**
 * Creates a message for a user status change.
 *
 * @param username The user's name.
 * @param status The new status.
 * @return Allocated Message instance.
 **/
Message *create_new_status_message(const char* username, const char* status);

/**
 * Creates a private text message.
 *
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message *create_text_from_message(const char* username, const char* text);

/**
 * Creates a public broadcast text message.
 *
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message *create_public_text_from_message(const char* username, const char* text);

/**
 * Constructs a list of users and their statuses.
 *
 * @param usernames Array of usernames.
 * @param statuses Array of matching statuses.
 * @param count Number of users.
 * @return Allocated Message instance.
 **/
Message *create_users_list_message(char** usernames, char** statuses, int count);

/**
 * Creates an invitation message to a room.
 *
 * @param username Inviter's username.
 * @param roomname Room to join.
 * @return Allocated Message instance.
 **/
Message *create_invite_message(const char* username, const char* roomname);

/**
 * Creates a message indicating a user has joined a room.
 *
 * @param roomname Name of the room.
 * @param username Username of the joining user.
 * @return Allocated Message instance.
 **/
Message *create_joined_room_message(const char* roomname, const char* username);

/**
 * Creates a message listing users inside a specific room.
 *
 * @param roomname The room name.
 * @param usernames Array of usernames.
 * @param statuses Array of statuses.
 * @param count Number of users.
 * @return Allocated Message instance.
 **/
Message *create_room_users_list_message(const char* roomname, const char** usernames, const char** statuses, int count);

/**
 * Creates a room-specific chat message.
 *
 * @param roomname Name of the room.
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message *create_room_text_from_message(const char* roomname, const char* username, const char* text);

/**
 * Creates a notification that a user has left a room.
 *
 * @param roomname Room name.
 * @param username User who left.
 * @return Allocated Message instance.
 **/
Message *create_left_room_message(const char* roomname, const char* username);

/**
 * Creates a message indicating a client has disconnected.
 *
 * @param username The disconnected client's username.
 * @return Allocated Message instance.
 **/
Message *create_disconnected_message(const char* username);

/**
 * Creates a generic response message for client feedback.
 *
 * @param operation The type of operation (e.g., "JOIN_ROOM").
 * @param result Result string (e.g., "SUCCESS", "ERROR").
 * @param extra Optional field (room name, username, etc.).
 * @param count Optional field the users count).
 * @return Allocated Message instance.
 **/
Message *create_response_message(const char* operation, const char* result, const char* extra, int count);

/**
 * Frees memory allocated for a Message.
 * Safe to call with NULL. Also frees internal cJSON.
 *
 * @param msg Pointer to Message to be destroyed.
 **/
void free_message(Message *msg);

#endif // MESSAGE_H
