#include "controller.hpp"
#include "wrapper_controller.h"

#define controller Controller::instance()

/**
 * Attempts to connect to the server with the given parameters.
 *
 * @param port The server port.
 * @param ip The server IP address.
 * @param user_name The username to use.
 **/
void
controller_try_connection(int port,
			  const char* ip,
			  const char* user_name)
{
  controller.try_connection(port, ip, std::string(user_name));
}

/**
 * Sends a public message to all connected users.
 *
 * @param msg_content The message content.
 **/
void
controller_public_message(const char* msg_content)
{
  std::string message_content = msg_content;
  controller.public_message(message_content);
}

/**
 * Sends a direct (private) message to a recipient.
 *
 * @param msg_content The message content.
 * @param recipient The target recipient.
 **/
void
controller_direct_message(const char* msg_content,
			  const char* recipient)
{
  std::string message_content = msg_content;
  std::string target = recipient;
  controller.direct_message(message_content, target);
}

/**
 * Changes the current user's status.
 *
 * @param status The new status string.
 **/
void
controller_change_status(const char* status)
{
  std::string stts = status;
  controller.change_status(stts);
}


/**
 * Requests the list of users in the public chat.
 **/
void
controller_chat_users()
{
  controller.chat_users();
}

/**
 * Creates a new chat room with the given name.
 *
 * @param room_name The name of the room to create.
 **/
void
controller_new_room(const char* room_name)
{
  std::string roomname = room_name;
  controller.new_room(roomname);
}

/**
 * Invites users to a specified room.
 *
 * @param guests Null-terminated array of usernames.
 * @param room_name Name of the room to invite users to.
 **/
void
controller_invite_users(char** guests, const char* room_name)
{
  std::string roomname = room_name;
  controller.invite_users(guests, roomname);
}

/**
 * Joins a chat room with the specified name.
 *
 * @param room_name The name of the room to join.
 **/
void
controller_join_room(const char* room_name)
{
  std::string roomname = room_name;
  controller.join_room(roomname);
}

/**
 * Requests the list of users in the specified room.
 *
 * @param room_name The name of the room.
 **/
void
controller_room_users(const char* room_name)
{
  std::string roomname = room_name;
  controller.room_users(roomname);
}

/**
 * Sends a message to a specific chat room.
 *
 * @param msg_content The message to send.
 * @param room_name The target room.
 **/
void
controller_room_message(const char* msg_content,
			const char* room_name)
{
  std::string message_content = msg_content;
  std::string roomname = room_name;
  controller.room_message(message_content, roomname);
}

/**
 * Leaves the specified chat room.
 *
 * @param room_name The name of the room to leave.
 **/
void
controller_leave_room(const char* room_name)
{
  std::string roomname = room_name;
  controller.leave_room(roomname);
}

/**
 * Retrieves the number of users in a chat.
 *
 * @param name The name of the chat.
 * @return The number of users.
 **/
int
controller_get_count(const char* name)
{
  std::string chat_name = name;
  return controller.get_chat_count(chat_name);
}

/* */
int
controller_get_status(const char* user_name)
{
  std::string username = user_name;
  return controller.get_user_status(username);
}

/**
 * Disconnects the current user from the server.
 **/
void
controller_disconnect()
{
  controller.disconnect_user();
}
