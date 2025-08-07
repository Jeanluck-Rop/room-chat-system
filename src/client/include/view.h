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
  void back_to_home_page();
  
  /* */
  void delete_user_chat_row(const char* user_name);
  
  /**
   * Launches an alert dialog from the main chat to notify the user of an event.
   *
   * @param detail Message describing the event.
   * @param type Type of dialog.
   **/
  void alert_dialog(const char* detail, DialogType type);
  
  /**
   * Updates the status of a user in the user chat, if visible.
   *
   * @param user_name The name of the user.
   * @param status The new status to display.
   **/////////
  void update_user_status(const char* user_name, const char* status);
  
  /**
   * Updates the member count of a given chat, if currently displayed.
   *
   * @param chat_name Name of the chat.
   * @param users_count Updated number of users.
   **/
  void update_chat_count(const char* chat_name, int users_count);

  /**
   * Displays the invitation window with the list of users and their statuses
   * from the general chat and any rooms they belong to.
   *
   * @param users Null-terminated array of usernames.
   * @param statuses Null-terminated array of corresponding statuses for each user.
   **/
  void show_invitation_window(char** users, char** statuses);
    
  /**
   * Displays a window showing users and their statuses in the specified room.
   *
   * @param room_name Name of the room.
   * @param users Array of usernames in the room.
   * @param statuses Array of corresponding statuses.
   **/
  void show_room_users(const char* room_name, char **users, char **statuses);
  
  /**
   * Displays a window showing users and their statuses in the general chat.
   *
   * @param users Array of usernames.
   * @param statuses Array of corresponding statuses.
   **/
  void show_chat_users(char **users, char **statuses);
  
  /**
   * Handles a message received from the server, delegating it to the appropriate
   * GUI handler based on the chat and message type.
   *
   * @param chat_name Name of the chat the message belongs to.
   * @param sender Sender of the message.
   * @param content Content of the message.
   * @param chat_type Type of the chat.
   * @param msg_type Type of the message.
   **/
  void message_received(const char* chat_name, const char* sender, const char* content, ChatType chat_type, MessageType msg_type);

  /**
   * Adds a new notification to the user's notification list to inform about an event.
   *
   * @param msg Notification message.
   * @param room_name Related room name, if any.
   * @param type Notification type.
   **/
  void add_new_notify(const char* msg, const char* room_name, NotifyType type);
  
  /**
   * Displays the main chat interface once the user successfully connects.
   **/
  void enter_chat();
  
  /**
   * Launches an alert dialog from the home screen to notify the user of an event.
   *
   * @param detail Message describing the event.
   * @param type Dialog type (warning, success, error).
   **/
  void init_alert_dialog(const char* detail, DialogType type);
  
  /**
   * Launch the initial graphic user interface.
   *
   * @param port The inital port.
   * @param server_ip The inital server_ip.
   **/
  void launch_gui(int port, char* server_ip);
 
  /**
   * Loads the main page view for a given chat.
   *
   * @param chat Pointer to the chat object to display.
   * @param user_data The ChatData from the app.
   **/
  static void load_main_page(Chat *chat, gpointer user_data);

  /**
   * Launches the home page window so the user can connect to a server or quit the app.
   *
   * @param app Pointer to the GtkApplication instance.
   * @param data Pointer to the StartData structure with initial connection data.
   **/
  static void home_window(GtkApplication *app, StartData *data);
  
  /**
   * Class constants and variables.
   **/
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
