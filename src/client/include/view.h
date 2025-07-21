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
    char* name;
    ChatType type;
    GList *messages;
    GtkWidget *row;
    GtkWidget *recent_label;
  }
    Chat;
  
  ///
  typedef struct
  {
    GtkApplication *app;
    GtkBuilder *builder;
    GtkWindow *window;
    GtkWidget *chats_list;
    GtkWidget *main_content;
    GtkWidget *messages_box;
    GList *chats;
    Chat *current_chat;
    Notifs *notifs;
  }
    ChatData;

  ///
  typedef struct
  {
    char* sender;
    char* content;
    MessageType type;
  }
    ChatMessage;


  /* */
  void add_notify(const char* msg);

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
  static void add_new_message(Chat *chat, ChatData *chatty, MessageType type, const char* sender, const char* content);
  
  /* */
  static Chat* new_chat_row(ChatData *chatty, ChatType type, const char* name, const char* msg);

   
  /**
   * Class constants and variables.
   **/
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
