#ifndef WRAPPER_CONTROLLER_H
#define WRAPPER_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* */
  void controller_try_connection(int port, const char *ip, const char *user_name);

  /* */
  void controller_public_message(const char* msg_content);

  /* */
  void controller_direct_message(const char* msg_content, const char* recipient);

  /* */
  void controller_change_status(const char* status);

  /* */
  void controller_chat_users();

  /* */
  void controller_new_room(const char* room_name);

  /* */
  void controller_invite_users(char** guests, const char* room_name);

  /* */
  void controller_join_room(const char* room_name);

  /* */
  void controller_room_users(const char* room_name);

  /* */
  void controller_room_message(const char* msg_content, const char* room_name);
  
  /* */
  void controller_leave_room(const char* room_name);
  
  /* */
  int controller_get_count(const char* name);
  
  /* */
  void controller_disconnect();
  
#ifdef __cplusplus
}
#endif

#endif // CONTROLLER_WRAPPER_H
