#ifndef WRAPPER_CONTROLLER_H
#define WRAPPER_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Attempts to connect to the server with the given parameters.
   *
   * @param port The server port.
   * @param ip The server IP address.
   * @param user_name The username to use.
   **/
  void controller_try_connection(int port, const char *ip, const char *user_name);

  /**
   * Sends a public message to all connected users.
   *
   * @param msg_content The message content.
   **/
  void controller_public_message(const char* msg_content);

  /**
   * Sends a direct message to a recipient.
   *
   * @param msg_content The message content.
   * @param recipient The target recipient.
   **/
  void controller_direct_message(const char* msg_content, const char* recipient);

  /**
   * Changes the current user's status.
   *
   * @param status The new status string.
   **/
  void controller_change_status(const char* status);

  /**
   * Requests the list of users in the public chat.
   **/
  void controller_chat_users();

  /**
   * Creates a new chat room with the given name.
   *
   * @param room_name The name of the room to create.
   **/
  void controller_new_room(const char* room_name);

  /**
   * Invites users to a specified room.
   *
   * @param guests Null-terminated array of usernames.
   * @param room_name Name of the room to invite users to.
   **/
  void controller_invite_users(char** guests, const char* room_name);

  /**
   * Joins a chat room with the specified name.
   *
   * @param room_name The name of the room to join.
   **/
  void controller_join_room(const char* room_name);

  /**
   * Requests the list of users in the specified room.
   *
   * @param room_name The name of the room.
   **/
  void controller_room_users(const char* room_name);

  /**
   * Sends a message to a specific chat room.
   *
   * @param msg_content The message to send.
   * @param room_name The target room.
   **/
  void controller_room_message(const char* msg_content, const char* room_name);
  
  /**
   * Leaves the specified chat room.
   *
   * @param room_name The name of the room to leave.
   **/
  void controller_leave_room(const char* room_name);
  
  /**
   * Retrieves the number of users in a chat.
   *
   * @param name The name of the chat.
   * @return The number of users.
   **/
  int controller_get_count(const char* name);

  /* */
  int controller_get_status(const char* user_name);
  
  /**
   * Disconnects the current user from the server.
   **/
  void controller_disconnect();
  
#ifdef __cplusplus
}
#endif

#endif // CONTROLLER_WRAPPER_H
