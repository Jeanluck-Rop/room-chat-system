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
    
  /* */
  gboolean new_notify_idle(gpointer call_data);
  
  /* */
  gboolean message_received_idle(gpointer call_data);

  /* */
  gboolean alert_chat_idle(gpointer call_data);
  
  /* */
  gboolean enter_chat_idle(gpointer call_data);

  /* */
  gboolean init_alert_dialog_idle(gpointer call_data);

#ifdef __cplusplus
}
#endif

#endif // IDLE_H
