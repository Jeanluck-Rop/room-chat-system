#include "controller.hpp"
#include "wrapper_controller.h"

/* */
void
controller_try_connection(int port,
			  const char* ip,
			  const char* user_name)
{
  Controller::instance().try_connection(port, ip, std::string(user_name));
}

/* */
void
controller_public_message(const char* msg_content)
{
  std::string message_content = msg_content;
  Controller::instance().public_message(message_content);
}

/* */
void
controller_direct_message(const char* msg_content,
			  const char* recipient)
{
  std::string message_content = msg_content;
  std::string target = recipient;
  Controller::instance().direct_message(message_content, target);
}

/* */
void
controller_change_status(const char* status)
{
  std::string stts = status;
  Controller::instance().change_status(stts);
}

/* */
void
controller_chat_users()
{
  Controller::instance().chat_users();
}

/* */
void
controller_new_room(const char* room_name)
{
  std::string roomname = room_name;
  Controller::instance().new_room(roomname);
}

/* */
void
controller_invite_users(char** guests, const char* room_name)
{
  std::string roomname = room_name;
  Controller::instance().invite_users(guests, roomname);
}

/* */
void
controller_join_room(const char* room_name)
{
  std::string roomname = room_name;
  Controller::instance().join_room(roomname);
}

/* */
void
controller_room_users(const char* room_name)
{
  std::string roomname = room_name;
  Controller::instance().room_users(roomname);
}

/* */
void
controller_room_message(const char* msg_content,
			const char* room_name)
{
  std::string message_content = msg_content;
  std::string roomname = room_name;
  Controller::instance().room_message(message_content, roomname);
}

/* */
void
controller_leave_room(const char* room_name)
{
  std::string roomname = room_name;
  Controller::instance().leave_room(roomname);
}

/* */
int
controller_get_count(const char* name)
{
  std::string chat_name = name;
  return Controller::instance().get_chat_count(chat_name);
}

/* */
void
controller_disconnect()
{
  Controller::instance().disconnect_user();
}
