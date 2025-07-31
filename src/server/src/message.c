#include "message.h"

/**
 * Helper to extract a string field from a Message.
 * Returns empty string if key not found or invalid.
 *
 * @param msg Pointer to Message.
 * @param key JSON field key.
 * @return Pointer to internal string, or "".
 **/
static const char*
get_string(const Message* msg,
	   const char* key)
{
  cJSON *item = cJSON_GetObjectItem(msg->json_data, key);
  return cJSON_IsString(item) ? item->valuestring : "";
}

/**
 * Internal helper to create a base message with a type field.
 * Allocates a new Message and adds the `type` field.
 *
 * @param type Type string to set (e.g., "TEXT", "JOIN_ROOM").
 * @return Pointer to a new Message instance.
 **/
static Message*
create_base_message(const char* type)
{
  Message *msg = malloc(sizeof(Message));
  msg->json_data = cJSON_CreateObject();
  cJSON_AddStringToObject(msg->json_data, "type", type);
  return msg;
}

/**
 * Gets the message type from the "type" field.
 *
 * @param msg Message pointer.
 * @return MessageType enum value.
 **/
MessageType
get_type(const Message* msg)
{
  const char *type = get_string(msg, "type");
  if (strcmp(type, "IDENTIFY") == 0)
    return IDENTIFY;
  if (strcmp(type, "STATUS") == 0)
    return STATUS;
  if (strcmp(type, "USERS") == 0)
    return USERS;
  if (strcmp(type, "TEXT") == 0)
    return TEXT;
  if (strcmp(type, "PUBLIC_TEXT") == 0)
    return PUBLIC_TEXT;
  if (strcmp(type, "NEW_ROOM") == 0)
    return NEW_ROOM;
  if (strcmp(type, "INVITE") == 0)
    return INVITE;
  if (strcmp(type, "JOIN_ROOM") == 0)
    return JOIN_ROOM;
  if (strcmp(type, "ROOM_USERS") == 0)
    return ROOM_USERS;
  if (strcmp(type, "ROOM_TEXT") == 0)
    return ROOM_TEXT;
  if (strcmp(type, "LEAVE_ROOM") == 0)
    return LEAVE_ROOM;
  if (strcmp(type, "DISCONNECT") == 0)
    return DISCONNECT;
  return UNKNOWN;
}

/**
 * Serializes a Message to a JSON string.
 *
 * @param msg Message to serialize.
 * @return New string containing compact JSON.
 **/
char*
to_json(const Message *msg)
{
  return cJSON_PrintUnformatted(msg->json_data);
}

/**
 * Extracts the "username" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char*
get_username(const Message *msg)
{
  return get_string(msg, "username");
}

/**
 * Extracts the "text" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char*
get_text(const Message *msg)
{
  return get_string(msg, "text");
}

/**
 * Extracts the "status" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char*
get_status(const Message *msg)
{
  return get_string(msg, "status");
}

/**
 * Extracts the "roomname" field from a message.
 *
 * @param msg Message pointer.
 * @return String value, "" in other case.
 **/
const char*
get_roomname(const Message *msg)
{
  return get_string(msg, "roomname");
}

/**
 * Extracts a list of usernames from a message.
 *
 * @param msg Message containing a "usernames" array.
 * @param size Output parameter for number of usernames.
 * @return Allocated array of strdup'd usernames.
 **/
char**
get_users(const Message *msg,
	  int *size)
{
  cJSON *usernames_array = cJSON_GetObjectItem(msg->json_data, "usernames");
  if (!cJSON_IsArray(usernames_array))
    return NULL;
  
  *size = cJSON_GetArraySize(usernames_array);
  if (*size <= 0)
    return NULL;  
  char **usernames = malloc(*size * sizeof(char *));
  if (!usernames)
    return NULL;
  for (int i = 0; i < *size; ++i) {
    cJSON *item = cJSON_GetArrayItem(usernames_array, i);
    if (cJSON_IsString(item) && item->valuestring != NULL) {
      usernames[i] = strdup(item->valuestring);
      if (!usernames[i]) {
	// If strdup fails, we free everything previously assigned 
	for (int j = 0; j < i; ++j)
	  free(usernames[j]);
	free(usernames);
	return NULL;
      }
    } else
      usernames[i] = NULL;
  }
  
  return usernames;
}

/**
 * Parses a raw JSON string into a Message.
 *
 * @param raw_message A null-terminated JSON string.
 * @return Allocated Message or NULL if parsing fails.
 **/
Message*
parse(const char* raw_message)
{
  cJSON *data = cJSON_Parse(raw_message);
  if (!data)
    return NULL;
  Message *msg = malloc(sizeof(Message));
  if (!msg) {
    cJSON_Delete(data);
    return NULL;
  }
  msg->json_data = data;
  return msg;
}

/**
 * Creates a message announcing a new connected user.
 *
 * @param username The new user's username.
 * @return Allocated Message instance.
 **/
Message*
create_new_user_message(const char* username)
{
  Message *msg = create_base_message("NEW_USER");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

/**
 * Creates a message for a user status change.
 *
 * @param username The user's name.
 * @param status The new status.
 * @return Allocated Message instance.
 **/
Message*
create_new_status_message(const char* username,
			  const char* status)
{
  Message *msg = create_base_message("NEW_STATUS");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "status", status);
  return msg;
}

/**
 * Creates a private text message.
 *
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message*
create_text_from_message(const char* username,
			 const char* text)
{
  Message *msg = create_base_message("TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

/**
 * Creates a public broadcast text message.
 *
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message*
create_public_text_from_message(const char* username,
				const char* text)
{
  Message *msg = create_base_message("PUBLIC_TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

/**
 * Constructs a list of users and their statuses.
 *
 * @param usernames Array of usernames.
 * @param statuses Array of matching statuses.
 * @param count Number of users.
 * @return Allocated Message instance.
 **/
Message*
create_users_list_message(char** usernames,
			  char** statuses,
			  int count)
{
  Message *msg = create_base_message("USER_LIST");
  cJSON *users_object = cJSON_CreateObject();
  for (int i = 0; i < count; ++i)
    cJSON_AddStringToObject(users_object, usernames[i], statuses[i]);
  cJSON_AddItemToObject(msg->json_data, "users", users_object);
  return msg;
}

/**
 * Creates an invitation message to a room.
 *
 * @param username Inviter's username.
 * @param roomname Room to join.
 * @return Allocated Message instance.
 **/
Message*
create_invite_message(const char* username,
		      const char* roomname)
{
  Message *msg = create_base_message("INVITATION");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  return msg;
}

/**
 * Creates a message indicating a user has joined a room.
 *
 * @param roomname Name of the room.
 * @param username Username of the joining user.
 * @return Allocated Message instance.
 **/
Message*
create_joined_room_message(const char* roomname,
			   const char* username)
{
  Message *msg = create_base_message("JOINED_ROOM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

/**
 * Creates a message listing users inside a specific room.
 *
 * @param roomname The room name.
 * @param usernames Array of usernames.
 * @param statuses Array of statuses.
 * @param count Number of users.
 * @return Allocated Message instance.
 **/
Message*
create_room_users_list_message(const char* roomname,
			       const char** usernames,
			       const char** statuses,
			       int count)
{
  Message *msg = create_base_message("ROOM_USER_LIST");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON *users = cJSON_CreateObject();
  for (int i = 0; i < count; i++) {
    cJSON_AddStringToObject(users, usernames[i], statuses[i]);
  }
  cJSON_AddItemToObject(msg->json_data, "users", users);
  return msg;
}

/**
 * Creates a room-specific chat message.
 *
 * @param roomname Name of the room.
 * @param username Sender's username.
 * @param text Message content.
 * @return Allocated Message instance.
 **/
Message*
create_room_text_from_message(const char* roomname,
			      const char* username,
			      const char* text)
{
  Message *msg = create_base_message("ROOM_TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

/**
 * Creates a notification that a user has left a room.
 *
 * @param roomname Room name.
 * @param username User who left.
 * @return Allocated Message instance.
 **/
Message*
create_left_room_message(const char* roomname,
			 const char* username)
{
  Message *msg = create_base_message("LEFT_ROOM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

/**
 * Creates a message indicating a client has disconnected.
 *
 * @param username The disconnected client's username.
 * @return Allocated Message instance.
 **/
Message*
create_disconnected_message(const char* username)
{
  Message *msg = create_base_message("DISCONNECTED");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

/**
 * Creates a generic response message for client feedback.
 *
 * @param operation The type of operation (e.g., "JOIN_ROOM").
 * @param result Result string (e.g., "SUCCESS", "ERROR").
 * @param extra Optional field (room name, username, etc.).
 * @return Allocated Message instance.
 **/
Message*
create_response_message(const char* operation,
			const char* result,
			const char* extra,
			int count)
{
  Message *msg = create_base_message("RESPONSE");
  cJSON_AddStringToObject(msg->json_data, "operation", operation);
  cJSON_AddStringToObject(msg->json_data, "result", result);
  if (strcmp(extra, "") != 0)
    cJSON_AddStringToObject(msg->json_data, "extra", extra);
   if (count != 0)
    cJSON_AddNumberToObject(msg->json_data, "count", count);
  return msg;
}

/**
 * Frees memory allocated for a Message.
 *
 * @param msg Pointer to Message to be destroyed.
 **/
void
free_message(Message *msg)
{
  if (!msg)
    return;
  if (msg->json_data)
    cJSON_Delete(msg->json_data);
  free(msg);
}
