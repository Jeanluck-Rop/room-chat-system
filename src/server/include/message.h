#ifndef MESSAGE_H
#define MESSAGE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cJSON.h"

typedef enum {
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
} MessageType;

typedef struct {
  cJSON *json_data;
} Message;

// Methods for creating different types of messages according to the json protocol
Message *create_new_user_message(const char *username);
Message *create_new_status_message(const char *username, const char *status);
Message *create_text_from_message(const char *username, const char *text);
Message *create_public_text_from_message(const char *username, const char *text);
Message *create_users_list_message(char **usernames, char **statuses, int count);
Message *create_invite_message(const char *username, const char *roomname);
Message *create_joined_room_message(const char *roomname, const char *username);
Message *create_room_users_list_message(const char *roomname, const char **usernames, const char **statuses, int count);
Message *create_room_text_from_message(const char *roomname, const char *username, const char *text);
Message *create_left_room_message(const char *roomname, const char *username);
Message *create_disconnected_message(const char *username);
Message *create_response_message(const char *operation, const char *result, const char *extra);

// Function for parsing incoming messages
Message *parse(const char *raw_message);

// Getters
MessageType get_type(const Message *msg);
const char *get_username(const Message *msg);
const char *get_text(const Message *msg);
const char *get_status(const Message *msg);
const char *get_roomname(const Message *msg);
char **get_users(const Message *msg, int *size);

// Serialization method, convert the message in json type
char *to_json(const Message *msg); //If this function is called you must free memory

// Free resources
void free_message(Message *msg);

#endif // MESSAGE_H
