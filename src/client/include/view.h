#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "models.h"

#ifdef __cplusplus
extern "C"
{
#endif
 
  /* */
  void add_new_notify(const char* msg, const char* room_name, NotifyType type);
  
  /* */
  void message_received(const char* chat_name, const char* sender, const char* content, ChatType chat_type, MessageType msg_type);

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
  
  /**
   * Class constants and variables.
   **/
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
