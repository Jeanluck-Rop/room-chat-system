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
   NORMAL_NOTIF,
   INVITE_NOTIF
  }
    NotifyType;
  
  ///
  typedef enum
  {
    NORMAL_MESSAGE,
    OWN_MESSAGE,
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
    char* message;
    char* room_name;
    NotifyType type;
  }
    Notify;

  ///
  typedef struct
  {
    GList *list;
    GtkWidget *box;
    GtkWidget *button;
    GtkPopover *popover;
  }
    Notifs;

  typedef struct
  {
    Notify *notif;
    Notifs *notifs;
  }
    InviteData;

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
    GtkWidget *messages_scroll;
    GtkWidget *message_entry;
    GtkWidget *send_button;
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

  ///
  typedef struct
  {
    GtkWidget *main_button;
    GtkPopover *popover;
    GtkBuilder *builder;
    GtkWindow *change_status_window;
    GtkWindow *new_room_window;
    GtkWindow *invite_users_window;
    GtkWindow *leave_room_window;
    GtkWindow *disconnect_window;
    GtkButton *selected_room_btn;
    char* selected_roomname;
    char** selected_users;
  }
    ChatActions;

  typedef struct
  {
    GtkEntry *entry;
    GtkWidget *accept_button;
    size_t min_len;
    size_t max_len;
  }
    EntryValidation;

  /* */
  void add_new_notify(const char* msg, const char* room_name, NotifyType type);
  
  /* */
  void message_received(const char* chat_name, const char* sender, const char* content, ChatType chat_type, MessageType msg_type);

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
