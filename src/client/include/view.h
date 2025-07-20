#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C"
{
#endif
  ///
  typedef enum
  {
   PUBLIC_CHAT,
   ROOM_CHAT,
   USER_CHAT
  }
    ChatType;

    ///
  typedef enum
  {
    NORMAL_MESSAGE,
    INFO_MESSAGE
  }
    MessageType;
  
  ///
  typedef struct
  {
    int port;
    char* server_ip;
    GtkWindow *window;
    GtkWidget *port_entry;
    GtkWidget *ip_entry;
  }
    StartData;

  ///
  typedef struct
  {
    GList *list;
    GtkWidget *box;
    GtkWidget *button;
    GtkPopover *popover;
  }
    Notifs;
  
  ///
  typedef struct
  {
    GtkApplication *app;
    GtkBuilder *builder;
    GtkWindow *window;
    GtkWidget *chats_list;
    GtkWidget *main_content;
    GList *chats;
    Notifs *notifs;
  }
    ChatData;
  
  ///
  typedef struct
  {
    char* name;
    ChatType type;
    GList *messages;
    GtkWidget *row_widget;
  }
    Chat;
 

  ///
  typedef struct
  {
    char* sender;
    char* content;
    MessageType type;
  }
    ChatMessage;
  
  /**
   * Launch the initial graphic user interface
   *
   * @param port The inital port.
   * @param server_ip The inital server_ip.
   **/
  void launch_gui(int port, char* server_ip);

  /* */
  void add_new_message(MessageType type, const char* chat_name, const char* sender, const char* content);

  /* */
  void new_chat_row(ChatType type, const char* name, const char* msg);
  
  /* */
  void add_notify(const char* msg);

  /**
   * Class constants and variables.
   **/
  
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
