#ifndef IDLE_H
#define IDLE_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct
  {
    char *detail;
    DialogType type;
  }
    DialogIdle;

  typedef struct
  {
    char* chat_name;
    char* sender;
    char* content;
    ChatType chat_type;
    MessageType msg_type;
  }
    MessageIdle;

  typedef struct
  {
    char* msg;
    char* room_name;
    NotifyType type;
  }
    NotifyIdle;

  typedef struct
  {
    char** users;
    char** statuses;
  }
    ChatUsersListIdle;

  typedef struct
  {
    char* room_name;
    char** users;
    char** statuses;
  }
    RoomUsersListIdle;

  typedef struct
  {
    char* chat_name;
    int users_count;
  }
    CountIdle;  

  typedef struct
  {
    char* user_name;
    char* status;
  }
    StatusIdle;
  
  /* */
  gboolean alert_dialog_idle(gpointer call_data);

  /* */
  gboolean update_status_idle(gpointer call_data);
  
  /* */
  gboolean update_count_idle(gpointer call_data);
  
  /* */
  gboolean room_users_idle(gpointer call_data);
  
  /* */
  gboolean show_users_idle(gpointer call_data);
  
  /* */
  gboolean new_notify_idle(gpointer call_data);
  
  /* */
  gboolean message_received_idle(gpointer call_data);
  
  /* */
  gboolean enter_chat_idle(gpointer call_data);

  /* */
  gboolean init_alert_dialog_idle(gpointer call_data);

#ifdef __cplusplus
}
#endif

#endif // IDLE_H
