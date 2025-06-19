#include "message.h"
#include "cJSON.h"

static Message *create_base_message(const char *type) {
  Message *msg = malloc(sizeof(Message));
  msg->json_data = cJSON_CreateObject();
  cJSON_AddStringToObject(msg->json_data, "type", type);
  return msg;
}

Message *create_new_user_message(const char *username) {
  Message *msg = create_base_message("NEW_USER");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

Message *create_new_status_message(const char *username, const char *status) {
  Message *msg = create_base_message("NEW_STATUS");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "status", status);
  return msg;
}

Message *create_text_from_message(const char *username, const char *text) {
  Message *msg = create_base_message("TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

Message *create_public_text_from_message(const char *username, const char *text) {
  Message *msg = create_base_message("PUBLIC_TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

Message *create_users_list_message(char **usernames, char **statuses, int count) {
  Message *msg = create_base_message("USER_LIST");
  
  cJSON *users_object = cJSON_CreateObject();
  for (int i = 0; i < count; ++i)
    cJSON_AddStringToObject(users_object, usernames[i], statuses[i]);
  cJSON_AddItemToObject(msg->json_data, "users", users_object);
  
  return msg;
}

Message *create_invite_message(const char *username, const char *roomname) {
  Message *msg = create_base_message("INVITATION");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  return msg;
}

Message *create_joined_room_message(const char *roomname, const char *username) {
  Message *msg = create_base_message("JOINED_ROOM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

Message *create_room_users_list_message(const char *roomname, const char **usernames, const char **statuses, int count) {
  Message *msg = create_base_message("ROOM_USER_LIST");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON *users = cJSON_CreateObject();
  for (int i = 0; i < count; i++) {
    cJSON_AddStringToObject(users, usernames[i], statuses[i]);
  }

  cJSON_AddItemToObject(msg->json_data, "users", users);
  return msg;
}

Message *create_room_text_from_message(const char *roomname,const char *username, const char *text) {
  Message *msg = create_base_message("ROOM_TEXT_FROM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  cJSON_AddStringToObject(msg->json_data, "text", text);
  return msg;
}

Message *create_left_room_message(const char *roomname, const char *username) {
  Message *msg = create_base_message("LEFT_ROOM");
  cJSON_AddStringToObject(msg->json_data, "roomname", roomname);
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

Message *create_disconnected_message(const char *username) {
  Message *msg = create_base_message("DISCONNECTED");
  cJSON_AddStringToObject(msg->json_data, "username", username);
  return msg;
}

Message *create_response_message(const char *operation, const char *result, const char *extra) {
  Message *msg = create_base_message("RESPONSE");
  cJSON_AddStringToObject(msg->json_data, "operation", operation);
  cJSON_AddStringToObject(msg->json_data, "result", result);
  if (strcmp(extra, "") != 0)
    cJSON_AddStringToObject(msg->json_data, "extra", extra);
  return msg;
}

Message *parse(const char *raw_message) {
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

static const char *get_string(const Message *msg, const char *key) {
  cJSON *item = cJSON_GetObjectItem(msg->json_data, key);
  return cJSON_IsString(item) ? item->valuestring : "";
}

const char *get_username(const Message *msg) {
  return get_string(msg, "username");
}

const char *get_text(const Message *msg) {
  return get_string(msg, "text");
}

const char *get_status(const Message *msg) {
  return get_string(msg, "status");
}

const char *get_roomname(const Message *msg) {
  return get_string(msg, "roomname");
}

char **get_users(const Message *msg, int *size) {
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

MessageType get_type(const Message *msg) {
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

char *to_json(const Message *msg) {
  return cJSON_PrintUnformatted(msg->json_data);
}

void free_message(Message *msg) {
  if (!msg)
    return;
  if (msg->json_data)
    cJSON_Delete(msg->json_data);
  free(msg);
}
