#ifndef IDLE_H
#define IDLE_H

#include <stdio.h>
#include <stdlib.h>

#include "view.h"
#include "models.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @struct DialogIdle. Structure used to pass dialog alert data to the GTK main loop.
   **/
  typedef struct
  {
    char *detail;
    DialogType type;
  }
    DialogIdle;

  /**
   * @struct MessageIdle. Structure used to pass message data asynchronously to the GUI.
   **/
  typedef struct
  {
    char* chat_name;
    char* sender;
    char* content;
    ChatType chat_type;
    MessageType msg_type;
  }
    MessageIdle;

  /**
   * @struct NotifyIdle. Structure used to send notifications to the GTK main loop.
   **/
  typedef struct
  {
    char* msg;
    char* room_name;
    NotifyType type;
  }
    NotifyIdle;

  /**
   * @struct ChatUsersListIdle. Structure used to update the user list in the public or room chat view.
   **/
  typedef struct
  {
    char** users;
    char** statuses;
  }
    ChatUsersListIdle;

  /**
   * @struct RoomUsersListIdle. Structure used to update the user list specifically for a room.
   **/
  typedef struct
  {
    char* room_name;
    char** users;
    char** statuses;
  }
    RoomUsersListIdle;

  /**
   * @struct CountIdle. Structure used to update the user count for a specific chat.
   **/
  typedef struct
  {
    char* chat_name;
    int users_count;
  }
    CountIdle;  

  /**
   * @struct StatusIdle. Structure used to update a specific user's status in the UI.
   **/
  typedef struct
  {
    char* user_name;
    char* status;
  }
    StatusIdle;

  /**
   * @struct DeleteIdle. Structure used to the existing user chat with the given usern_name in the UI.
   **/
  typedef struct
  {
    char* user_name;
  }
    DeleteIdle;

  /**
   * Idle callback to return the user to the home page.
   *
   * @param call_data NULL.
   * @return G_SOURCE_REMOVE
   **/
  gboolean back_to_home_idle(gpointer call_data);
  
  /**
   * Idle callback to delete a chat user row.
   *
   * @param call_data Pointer to a DeleteIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean delete_chat_idle(gpointer call_data);
  
  /**
   * Idle callback to show an alert dialog on the GTK main loop.
   *
   * @param call_data Pointer to a DialogIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean alert_dialog_idle(gpointer call_data);

  /**
   * Idle callback to update a user's status in the UI.
   *
   * @param call_data Pointer to a StatusIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean update_status_idle(gpointer call_data);
  
  /**
   * Idle callback to update the user count for a chat.
   *
   * @param call_data Pointer to a CountIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean update_count_idle(gpointer call_data);
  
  /**
   * Idle callback to display users and statuses for a specific room.
   *
   * @param call_data Pointer to a RoomUsersListIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean room_users_idle(gpointer call_data);
  
  /**
   * Idle callback to display users for general chat or invitation.
   *
   * @param call_data Pointer to a ChatUsersListIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean show_users_idle(gpointer call_data);
  
  /**
   * Idle callback to process and display a received message in the UI.
   *
   * @param call_data Pointer to a MessageIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean message_received_idle(gpointer call_data);

  /**
   * Idle callback to add a new notification to the notification system.
   *
   * @param call_data Pointer to a NotifyIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean new_notify_idle(gpointer call_data);
  
  /**
   * Idle callback to switch the UI to the chat interface.
   *
   * @param call_data NULL.
   * @return G_SOURCE_REMOVE
   **/
  gboolean enter_chat_idle(gpointer call_data);

  /**
   * Idle callback to initialize and show a startup error/success dialog.
   *
   * @param call_data Pointer to a DialogIdle structure.
   * @return G_SOURCE_REMOVE
   **/
  gboolean init_alert_dialog_idle(gpointer call_data);

#ifdef __cplusplus
}
#endif

#endif // IDLE_H
