#ifndef MODELS_H
#define MODELS_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C"
{
#endif
  
  /**
   * @enum ChatType. Defines the type of chat.
   **/
  typedef enum
  {
   PUBLIC_CHAT,
   ROOM_CHAT,
   USER_CHAT
  }
    ChatType;

  /**
   * @enum NotifyType. Specifies the type of notification.
   **/
  typedef enum
  {
   NORMAL_NOTIF,
   INVITE_NOTIF
  }
    NotifyType;
  
  /**
   * @enum MessageType. Represents the type of a message in a chat.
   **/
  typedef enum
  {
    NORMAL_MESSAGE,
    OWN_MESSAGE,
    INFO_MESSAGE
  }
    MessageType;

  /**
   * @enum DialogType. Defines the types of dialog alerts shown in the UI.
   **/
  typedef enum
  {
    SUCCESS_DIALOG,
    WARNING_DIALOG,
    ERROR_DIALOG
  }
    DialogType;

  /**
   * @enum UsersRequestType. Specifies the type of user list request.
   **/
  typedef enum
  {
    USERS_REQUEST_NONE,
    USERS_REQUEST_INVITE,
    USERS_REQUEST_PUBLIC
  }
    UsersRequestType;
  
  /**
   * @struct StartData. Holds startup configuration and UI references for initial connection.
   **/
  typedef struct
  {
    int port;
    char* server_ip;
    char* username;
    GtkWindow *window;
    GtkWidget *port_entry;
    GtkWidget *ip_entry;
    GtkWidget *user_name_entry;
  }
    StartData;
  
  /**
   * @struct Notify. Represents a single notification in the system.
   **/
  typedef struct
  {
    char* message;
    char* room_name;
    NotifyType type;
  }
    Notify;

  /**
   * @struct Notifs. Holds UI elements related to notifications.
   **/
  typedef struct
  {
    GList *list;
    GtkWidget *box;
    GtkWidget *button;
    GtkPopover *popover;
  }
    Notifs;

  /**
   * @struct InviteData. Holds data related to processing a user invitation.
   **/
  typedef struct
  {
    Notify *notif;
    Notifs *notifs;
  }
    InviteData;

  /**
   * @struct Chat. Represents a chat session (publi chat, room or direct message).
   **/
  typedef struct
  {
    char* name;
    ChatType type;
    GList *messages;
    GtkWidget *row;
    GtkWidget *recent_label;
    GtkWidget *status_label;
    GtkWidget *public_count_label;
    GtkWidget *room_count_label;
  }
    Chat;

  /**
   * @struct ChatMessage. Represents a single message inside a chat.
   **/
  typedef struct
  {
    char* sender;
    char* content;
    MessageType type;
  }
    ChatMessage;

  /**
   * @struct ChatActions. Holds references to all chat-related action dialogs and elements.
   **/
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

  /**
   * @struct ChatData. Holds all global state and widgets for the chat application.
   **/
  typedef struct
  {
    int port;
    char *ip;
    char *username;
    GtkApplication *app;
    GtkBuilder *builder;
    GtkWindow *window;
    GtkWidget *chats_list;
    GtkWidget *main_content;
    GtkWidget *header;
    GtkWidget *messages_box;
    GList *chats;
    Chat *current_chat;
    Notifs *notifs;
    GtkWidget *messages_scroll;
    GtkWidget *message_entry;
    GtkWidget *send_button;
    ChatActions *actions;
  }
    ChatData;
  
  /**
   * @struct EntryValidation. Holds information for validating an input entry field.
   **/
  typedef struct
  {
    GtkEntry *entry;
    GtkWidget *accept_button;
    size_t min_len;
    size_t max_len;
  }
    EntryValidation;

  /**
   * Stores the current type of user request for handle which dialog users list will be displayed.
   **/
  extern UsersRequestType current_users_request;
  
#ifdef __cplusplus
}
#endif

#endif // MODELS_H
