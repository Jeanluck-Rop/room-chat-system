#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "models.h"
#include "wrapper_controller.h"

#ifdef __cplusplus
extern "C"
{
#endif
  
  /* */
  void alert_dialog(const char* detail, DialogType type);
  
  /* *///////
  void update_user_status(const char* user_name, const char* status);
  
  /* *//////
  void update_count(const char* chat_name, int users_count);

  /* */
  void show_invitation_window(char** users, char** statuses);
    
  /* */
  void show_room_users(const char* room_name, char **users, char **statuses);
  
  /* */
  void show_chat_users(char **users, char **statuses);
  
  /* */
  void message_received(const char* chat_name, const char* sender, const char* content, ChatType chat_type, MessageType msg_type);

  /* */
  void add_new_notify(const char* msg, const char* room_name, NotifyType type);
  
  /* */
  void enter_chat();
  
  /* */
  void init_alert_dialog(const char* detail, DialogType type);
  
  /**
   * Launch the initial graphic user interface
   *
   * @param port The inital port.
   * @param server_ip The inital server_ip.
   **/
  void launch_gui(int port, char* server_ip);
 
  /* */
  static void load_main_page(Chat *chat, gpointer user_data);

  /* */
  static void home_window(GtkApplication *app, StartData *data);
  
  /**
   * Class constants and variables.
   **/
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
