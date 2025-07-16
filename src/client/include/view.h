#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <adwaita.h>

#ifdef __cplusplus
extern "C"
{
#endif
  ///
  typedef struct
  {
    char* server_ip;
    int port;
  }
    StartData;

  ///
  typedef enum
  {
   CHAT_TYPE_PUBLIC,
   CHAT_TYPE_ROOM,
   CHAT_TYPE_USER
  }
    ChatType;

  ///
  typedef struct
  {
    char *name;
    ChatType type;
    GList *messages;
    GtkWidget *row_widget;
  }
    Chat;
  
  ///
  typedef enum
  {
    MESSAGE_TYPE_NORMAL,
    MESSAGE_TYPE_INFO
  }
    MessageType;

  ///
  typedef struct
  {
    MessageType type;
    char *sender;
    char *content;
  }
    ChatMessage;
  
  /* */
  void launch_gui(char* server_ip, int port);

  /* */
  void add_notify(const char *msg);
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
