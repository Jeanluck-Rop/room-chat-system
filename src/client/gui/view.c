#include "view.h"

/* Constants and variables */
UsersRequestType current_users_request = USERS_REQUEST_NONE;
static const char* welcome = "Welcome to the Public Chat!";
static const char* rows_ui = "/org/chat/client/resources/rows.ui";
static const char* chat_ui = "/org/chat/client/resources/chat.ui";
static const char* start_ui = "/org/chat/client/resources/start.ui";
static const char* headers_ui = "/org/chat/client/resources/headers.ui";
static const char* main_page_ui = "/org/chat/client/resources/main_page.ui";
static const char* chat_css = "/org/chat/client/resources/css/chat.css";
static const char* rows_css = "/org/chat/client/resources/css/rows.css";
static const char* start_css = "/org/chat/client/resources/css/start.css";
static const char* headers_css = "/org/chat/client/resources/css/headers.css";
static const char* actions_css = "/org/chat/client/resources/css/actions.css";
static const char* notifies_css = "/org/chat/client/resources/css/notifies.css";
static const char* main_page_css = "/org/chat/client/resources/css/main_page.css";
static const char* invitation_css = "/org/chat/client/resources/css/invitation.css";
static const char* message_css = "/org/chat/client/resources/css/message.css";

/**
 * Handles the closing event of the chat window.
 * Disconnects from the chat, and destroys the chat window.
 *
 * @param window The GtkWindow being closed.
 * @param user_data Pointer to the ChatData structure.
 * @return TRUE to stop other handlers from being invoked for the event.
 **/
static gboolean
on_chat_window_close(GtkWindow *window,
                     gpointer user_data)
{
  ChatData *chatty = (ChatData *)user_data;
  controller_disconnect();
  gtk_window_destroy(window);
  g_application_quit(g_application_get_default());
  return TRUE;
}

/**
 * Returns the application view to the home page.
 **/
void
back_to_home_page()
{
  //Check if the window is already the home page
  GtkWindow *window;
  window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  if (window && g_object_get_data(G_OBJECT(window), "init-data") != NULL) 
    return;
  
  ChatData *chatty = get_chat_data();
  int port = chatty->port;
  char *ip = g_strdup(chatty->ip);
  gtk_widget_set_visible(GTK_WIDGET(chatty->window), FALSE);
  //relaunch home-page
  StartData *data = g_new0(StartData, 1);
  data->port = port;
  data->server_ip = ip;
  home_window(GTK_APPLICATION(g_application_get_default()), data);
  g_idle_add(free_chat_data, chatty);
}

/**
 * Launches an alert dialog from the main chat to notify the user of an event.
 *
 * @param detail Message describing the event.
 * @param type Type of dialog.
 **/
void
alert_dialog(const char* detail,
	     DialogType type)
{
  ChatData *data = get_chat_data();
  GtkWindow *parent = data->window;
  GtkAlertDialog *alert;
  const char *message;
  
  switch (type) {
  case SUCCESS_DIALOG:
      message = "Success";
      break;
    case WARNING_DIALOG:
      message = "Warning";
      break;
    case ERROR_DIALOG:
    default:
      message = "Error";
      break;
  }
  
  alert = gtk_alert_dialog_new(message);
  gtk_alert_dialog_set_detail(alert, detail);
  const char *buttons[] = { "OK", NULL };
  gtk_alert_dialog_set_buttons(alert, buttons);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_show(alert, parent);
}

/**
 * Updates the status of a user in the user chat, if visible.
 *
 * @param user_name The name of the user.
 * @param status The new status to display.
 **/
void
update_user_status(const char* user_name,
		   const char* status)
{
  ChatData *chatty = get_chat_data();
  Chat *chat = get_chat(user_name, chatty);
  if (!chat || !chatty->current_chat || g_strcmp0(chatty->current_chat->name, chat->name) != 0)
    return;

  const char *display_status;
  if (g_strcmp0(status, "AWAY") == 0)
    display_status = "Away";
  else if (g_strcmp0(status, "BUSY") == 0)
    display_status = "Busy";
  else
    display_status = "Active";
    
  gtk_label_set_text(GTK_LABEL(chat->status_label), display_status);
}

/**
 * Updates the member count of a given chat, if currently displayed.
 *
 * @param chat_name Name of the chat.
 * @param users_count Updated number of users.
 **/
void
update_chat_count(const char* chat_name,
	     int users_count)
{
  ChatData *chatty = get_chat_data();
  Chat *chat = get_chat(chat_name, chatty);
  if (!chat || !chatty->current_chat || g_strcmp0(chatty->current_chat->name, chat->name) != 0)
    return;
  if (chat->type == ROOM_CHAT) {
    char *format = g_strdup_printf("%d %s", users_count, "members");
    gtk_label_set_text(GTK_LABEL(chat->room_count_label), format);
    g_free(format);
  } else {
    char *formatted = g_strdup_printf("%d %s", users_count, "users");
    gtk_label_set_text(GTK_LABEL(chat->public_count_label), formatted);
    g_free(formatted);
  }
}

/**
 * Creates and adds a new chat to the chat list.
 * Builds the chat's row in the UI based on its type.
 *
 * @param chatty Pointer to the ChatData structure containing all chats.
 * @param type The type of chat (PUBLIC_CHAT, USER_CHAT, ROOM_CHAT).
 * @param name The name of the chat.
 * @param msg The initial message to display in the recent label.
 * @return Pointer to the newly created Chat structure.
 **/
Chat*
new_chat(ChatData *chatty,
	 ChatType type,
	 const char* name,
	 const char* msg)
{
  const char* row_id;
  const char* label_id;
  const char* recent_id;
  //verify the chat type and set its respective id
  switch (type) {
  case PUBLIC_CHAT:
    row_id = "public_row";
    label_id = "public_row_label";
    recent_id = "public_recent_label";
    break;
  case USER_CHAT:
    row_id = "user_row";
    label_id = "user_row_label";
    recent_id = "user_recent_label";
    break;
  case ROOM_CHAT:
    row_id = "room_row";
    label_id = "room_row_label";
    recent_id = "room_recent_label";
    break;
  default:
    return NULL; //cant happen
  }
  //build the row in the gui
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(rows_ui);
  GtkWidget *row;
  row = GTK_WIDGET(gtk_builder_get_object(builder, row_id));
  GtkWidget *label;
  label = GTK_WIDGET(gtk_builder_get_object(builder, label_id));
  GtkWidget *recent;
  recent = GTK_WIDGET(gtk_builder_get_object(builder, recent_id));
  //set the texts of the row
  gtk_label_set_text(GTK_LABEL(label), name);
  gtk_label_set_text(GTK_LABEL(recent), msg);
  //create the new chat
  Chat *chat = g_new0(Chat, 1);
  chat->name = g_strdup(name);
  chat->type = type;
  chat->messages = NULL;
  chat->row = GTK_WIDGET(row);
  chat->recent_label = recent;
  g_object_set_data(G_OBJECT(row), "chat-data", chat);
  //insert new chat
  gtk_list_box_append(GTK_LIST_BOX(chatty->chats_list), GTK_WIDGET(row));
  chatty->chats = g_list_append(chatty->chats, chat);
  add_new_message(chat, chatty, INFO_MESSAGE, "info", msg);
  g_object_unref(builder);
  return chat;
}

/**
 * Removes a chat from the chat list and frees its resources.
 * Also updates the UI to display the public chat if available.
 *
 * @param chatty Pointer to the ChatData structure containing all chats.
 * @param chat Pointer to the Chat to be removed.
 **/
void
remove_chat(ChatData *chatty,
	    Chat *chat)
{
  if (chat->row)
    gtk_list_box_remove(GTK_LIST_BOX(chatty->chats_list), chat->row);
  chatty->chats = g_list_remove(chatty->chats, chat);
  g_free(chat->name);
  if (chat->messages)
    g_list_free_full(chat->messages, (GDestroyNotify)free_chat_message);
  g_free(chat);
  Chat *public_chat = get_chat("PUBLIC_CHAT", chatty);
  if (public_chat)
    load_main_page(public_chat, chatty);
}

/**
 * Removes the chat row for a specific user.
 * Finds the chat associated with the given username and removes it
 * from the ChatData structure, updating the UI.
 *
 * @param user_name The username whose chat should be removed.
 **/
void
delete_user_chat_row(const char* user_name)
{
  ChatData *chatty = get_chat_data();
  Chat *user_chat = get_chat(user_name, chatty);
  if (!user_chat)
    return;
  remove_chat(chatty, user_chat);
}

/**
 * Displays the invitation window with the list of users and their statuses
 * from the general chat and any rooms they belong to.
 *
 * @param users Null-terminated array of usernames.
 * @param statuses Null-terminated array of corresponding statuses for each user.
 **/
void
show_invitation_window(char** users,
		       char** statuses)
{
  ChatData *chatty = get_chat_data();
  ChatActions *actions = chatty->actions;
  actions->invite_users_window = GTK_WINDOW(gtk_builder_get_object(actions->builder, "invite_users_window"));
  show_modal_window(GTK_WIDGET(actions->main_button), GTK_WIDGET(actions->invite_users_window));
  g_signal_connect(actions->invite_users_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  GtkListBox *rooms_list_box;
  rooms_list_box = GTK_LIST_BOX(gtk_builder_get_object(actions->builder, "rooms_available_list"));
  GtkListBox *users_list_box;
  users_list_box = GTK_LIST_BOX(gtk_builder_get_object(actions->builder, "users_available_list"));
  display_rooms_list(rooms_list_box, "room_av_list_item", G_CALLBACK(invitations_room_selected), actions);
  display_users_list(users_list_box, users, statuses, G_CALLBACK(guests_selected), actions);
  GtkWidget *accept;
  accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "invite_users_accept_button"));
  gtk_widget_set_sensitive(accept, FALSE);
  connect_accept_once("invite_users_accept_button", G_CALLBACK(invite_users_accept), actions);
  on_cancel_button(actions->builder, actions->invite_users_window, "invite_users_cancel_button");
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/**
 * Displays a window showing users and their statuses in the specified room.
 *
 * @param room_name Name of the room.
 * @param users Array of usernames in the room.
 * @param statuses Array of corresponding statuses.
 **/
void
show_room_users(const char* room_name,
		char **users,
		char **statuses)
{
  ChatData *chatty = get_chat_data();
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(headers_ui);
  GtkListBox *list_box;
  list_box = GTK_LIST_BOX(gtk_builder_get_object(builder, "room_users_list"));
  GCallback send_callback = G_CALLBACK(on_send_message_request);
  populate_user_list(builder, list_box, users, statuses, send_callback);
  GtkWindow *info_window;
  info_window = GTK_WINDOW(gtk_builder_get_object(builder, "room_info_window"));
  gtk_window_set_modal(info_window, TRUE);
  gtk_window_set_transient_for(info_window, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(chatty->header))));
  g_signal_connect(info_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  gtk_window_present(info_window);
}

/**
 * Displays a window showing users and their statuses in the general chat.
 *
 * @param users Array of usernames.
 * @param statuses Array of corresponding statuses.
 **/
void
show_chat_users(char **users,
		char **statuses)
{
  ChatData *chatty = get_chat_data();
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(headers_ui);
  GtkListBox *list_box;
  list_box = GTK_LIST_BOX(gtk_builder_get_object(builder, "public_users_list"));
  GCallback send_callback = G_CALLBACK(on_send_message_request);
  populate_user_list(builder, list_box, users, statuses, send_callback);
  GtkWindow *info_window;
  info_window = GTK_WINDOW(gtk_builder_get_object(builder, "public_info_window"));
  gtk_window_set_modal(info_window, TRUE);
  gtk_window_set_transient_for(info_window, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(chatty->header))));
  g_signal_connect(info_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  gtk_window_present(info_window);
}

/**
 * Loads the main page view for a given chat.
 *
 * @param chat Pointer to the chat object to display.
 * @param user_data The ChatData from the app.
 **/
void
load_main_page(Chat *chat,
	       gpointer user_data)
{
  if (chat->row)
    gtk_widget_remove_css_class(chat->row, "new-message");
  ChatData *chatty = (ChatData *)user_data;
  chatty->current_chat = chat;
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(main_page_ui);
  GtkWidget *row_page;
  row_page = GTK_WIDGET(gtk_builder_get_object(builder, "row_page"));
  chatty->header = GTK_WIDGET(gtk_builder_get_object(builder, "header_left"));
  GtkWidget *header_right;
  header_right = GTK_WIDGET(gtk_builder_get_object(builder, "header_right"));
  chatty->messages_box = GTK_WIDGET(gtk_builder_get_object(builder, "messages_box"));
  chatty->messages_scroll = GTK_WIDGET(gtk_builder_get_object(builder, "messages_scroll"));
  chatty->message_entry = GTK_WIDGET(gtk_builder_get_object(builder, "message_entry"));
  chatty->send_button = GTK_WIDGET(gtk_builder_get_object(builder, "send_button"));
  gtk_widget_set_sensitive(chatty->send_button, FALSE);
  //clean previous content
  clear_widget(chatty->main_content);
  //load header
  set_header(chat, chatty->header);
  //load user actions button
  set_actions(builder, chatty);
  //load messages
  for (GList *l = chat->messages; l; l = l->next) {
    ChatMessage *msg = l->data;
    GtkWidget *widget = build_message(msg);
    gtk_box_append(GTK_BOX(chatty->messages_box), widget);
  }
  //send button logic
  check_message_entry(chatty->message_entry, chatty->send_button);
  g_signal_connect(chatty->message_entry, "activate", G_CALLBACK(send_message), chatty);
  g_signal_connect(chatty->send_button, "clicked", G_CALLBACK(send_message), chatty);
  g_idle_add(focus_message_entry, chatty->message_entry);
  gtk_box_append(GTK_BOX(chatty->main_content), row_page);
  gtk_widget_set_visible(chatty->main_content, TRUE);
  g_object_unref(builder);
}

/**
 * Handles the selection of a row in the chat list.
 * Loads the main page for the chat associated with the selected row.
 *
 * @param box The GtkListBox containing the rows.
 * @param row The selected GtkListBoxRow.
 * @param user_data Pointer to the ChatData structure.
 **/
static void
on_row_selected(GtkListBox *box,
		GtkListBoxRow *row,
		gpointer user_data)
{
  Chat *chat = g_object_get_data(G_OBJECT(row), "chat-data");
  if (chat != NULL)
    load_main_page(chat, user_data);
}

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
void
message_received(const char* chat_name,
		 const char* sender,
		 const char* content,
		 ChatType chat_type,
		 MessageType msg_type)
{
  ChatData *chatty = get_chat_data();
  Chat *chat = get_chat(chat_name, chatty);
  
  if (chat) {
    add_new_message(chat, chatty, msg_type, sender, content);
    return;
  }
  //create new user chat
  if (chat_type == USER_CHAT) {
    char *init_msg = g_strdup_printf("Your chat with [%s]", sender);
    Chat *nw_chat = new_chat(chatty, chat_type, chat_name, init_msg);
    add_new_message(nw_chat, chatty, msg_type, sender, content);
    g_free(init_msg); 
  } else { //create new roomxs chat
    char *welcome = g_strdup_printf("Welcome to [%s]!", chat_name);
    Chat *nw_chat = new_chat(chatty, chat_type, chat_name, welcome);
    g_free(welcome); 
  }
}

/**
 * Adds a new notification to the user's notification list to inform about an event.
 *
 * @param msg Notification message.
 * @param room_name Related room name, if any.
 * @param type Notification type.
 **/
void
add_new_notify(const char *msg, const char* roomname, NotifyType type)
{
  ChatData *chatty = get_chat_data();
  Notify *notif = g_new0(Notify, 1);
  notif->type = type;
  notif->message = g_strdup(msg);
  notif->room_name = g_strdup(roomname);
  chatty->notifs->list = g_list_append(chatty->notifs->list, notif);
  gtk_widget_add_css_class(chatty->notifs->button, "has-notifications");
}



///
static void fake_chat_info(ChatData *chatty) {
  Chat *room_chat = new_chat(chatty, ROOM_CHAT, "Room 1", "Welcome to [Room 1]");
  Chat *room_cha = new_chat(chatty, ROOM_CHAT, "Room A", "Welcome to [Room A]");
  add_new_notify("Jose se desconecto del chat", NULL, NORMAL_NOTIF);
  add_new_notify("Jose abando no el cuarto {Pumitas}", NULL, NORMAL_NOTIF);
  add_new_notify("Aileen se unió al chat", NULL, NORMAL_NOTIF);
  add_new_notify("Sarah te invito al cuarto [Pumitas]", "Pumitas", INVITE_NOTIF);
  add_new_notify("Pablo te invito al cuarto [Panas]", "Panas", INVITE_NOTIF);
  message_received("Room 1", "Pope", "Hello to everyone on this chat! Lorem Ipsum destere mortum inicedi", ROOM_CHAT, NORMAL_MESSAGE);
  message_received("PUBLIC_CHAT", "Pancrasio", "Lorem Ipsum es simplemente el texto de relleno de las imprentas y archivos de texto. Lorem Ipsum ha sido el texto de relleno estándar de las industrias desde el año 1500, cuando un impresor (N. del T. persona que se dedica a la imprenta) desconocido usó una galería de textos y los mezcló de tal manera que logró hacer un libro de textos especimen. ", PUBLIC_CHAT, NORMAL_MESSAGE);
}
///



/**
 * Displays the main chat interface once the user successfully connects.
 **/
void
enter_chat()
{
  StartData *data = get_init_data();
  gtk_widget_set_visible(GTK_WIDGET(data->window), FALSE);
  gtk_window_destroy(data->window);
  ChatData *chatty = g_new0(ChatData, 1); //allocate memory for the ChatData struct
  chatty->port = data->port;
  chatty->ip = g_strdup(data->server_ip);
  chatty->username = g_strdup(data->username);
  chatty->app = GTK_APPLICATION(g_application_get_default());
  //Load the css
  load_css(chat_css);
  load_css(rows_css);
  load_css(headers_css);
  load_css(main_page_css);
  load_css(notifies_css);
  load_css(actions_css);
  load_css(invitation_css);
  load_css(message_css);
  //Define the chat builder
  chatty->builder = gtk_builder_new_from_resource(chat_ui);
  //Set initial chatty data
  chatty->window = GTK_WINDOW(gtk_builder_get_object(chatty->builder, "chat_window"));
  chatty->chats_list = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "chats_list"));
  chatty->main_content = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "main_content"));
  //set the main app
  gtk_window_set_application(chatty->window, chatty->app);
  g_object_set_data(G_OBJECT(chatty->window), "chat-data", chatty);
  //set the notif button/popover
  set_notifs(chatty);
  //set the public_chat
  Chat *public_chat = new_chat(chatty, PUBLIC_CHAT, "PUBLIC_CHAT", welcome);
  //connect each chat_row
  g_signal_connect(chatty->chats_list, "row-activated", G_CALLBACK(on_row_selected), chatty);
  //load the public chat page
  load_main_page(public_chat, chatty);
  //testing
  fake_chat_info(chatty);
  //show the window app
  gtk_window_present(chatty->window);
  g_signal_connect(chatty->window, "close-request", G_CALLBACK(on_chat_window_close), chatty);
  g_object_unref(chatty->builder);
}

/**
 * Launches an alert dialog from the home screen to notify the user of an event.
 *
 * @param detail Message describing the event.
 * @param type Dialog type.
 **/
void
init_alert_dialog(const char* detail,
		  DialogType type)
{
  StartData *data = get_init_data();
  GtkWindow *parent = data->window;
  GtkAlertDialog *alert;
  const char *message;
  switch (type) {
    case WARNING_DIALOG:
      message = "Warning";
      break;
    case ERROR_DIALOG:
    default:
      message = "Error";
      break;
  }
  alert = gtk_alert_dialog_new(message);
  gtk_alert_dialog_set_detail(alert, detail);
  const char *buttons[] = { "OK", NULL };
  gtk_alert_dialog_set_buttons(alert, buttons);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_show(alert, parent);
}

/**
 * Validates the initial connection data from the GUI.
 * Checks port number range, retrieves IP and username.
 * Attempts a connection through the controller.
 *
 * @param button The GTK button triggering the validation.
 * @param user_data Pointer to the StartData structure.
 **/
static void
validate_data(GtkButton *button,
	      gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  int port = get_port(data);
  const char* ip = get_ip(data);
  const char* user_name = get_username(data);
  if (port < 1024 || port > 49151) {
    init_alert_dialog("Port number must be between 1024 and 49151.", WARNING_DIALOG);
    return;
  }
  data->username = g_strdup(user_name);
  //controller_try_connection(port, ip, user_name); //
  enter_chat();
}

/**
 * Launches the home page window so the user can connect to a server or quit the app.
 *
 * @param app Pointer to the GtkApplication instance.
 * @param data Pointer to the StartData structure with initial connection data.
 **/
static void
home_window(GtkApplication *app,
	    StartData *data)
{
  load_css(start_css);
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(start_ui);
  GtkWindow *window;
  window = GTK_WINDOW(gtk_builder_get_object(builder, "start_window"));
  //reference the initial widgets
  data->window = window;
  data->port_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port_entry"));
  data->ip_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ip_entry"));
  data->user_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "username_entry"));
  g_signal_connect(data->port_entry, "changed", G_CALLBACK(on_port_changed), data);
  gtk_window_set_application(window, app);
  g_object_set_data(G_OBJECT(data->window), "init-data", data);
  //set given port and ip 
  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%d", data->port);
  gtk_editable_set_text(GTK_EDITABLE(data->port_entry), port_str);
  gtk_editable_set_text(GTK_EDITABLE(data->ip_entry), data->server_ip);
  //conect to the server
  GtkWidget *btn_connect;
  btn_connect = GTK_WIDGET(gtk_builder_get_object(builder, "connect_button"));
  gtk_widget_set_sensitive(btn_connect, FALSE);
  on_user_name_changed(data->user_name_entry, btn_connect);
  g_signal_connect(btn_connect, "clicked", G_CALLBACK(validate_data), data);
  //cancel and quit the program
  GtkWidget *btn_quit;
  btn_quit = GTK_WIDGET(gtk_builder_get_object(builder, "quit_button"));
  g_signal_connect(btn_quit, "clicked", G_CALLBACK(quit), NULL);
  //show the window
  gtk_window_present(window);
  g_signal_connect(window, "close-request", G_CALLBACK(on_start_window_close), data);
  g_object_unref(builder);
}

/**
 * Activates the main application window.
 * Called when the GTK application is launched.
 * Loads the home window with the provided StartData.
 *
 * @param app The GtkApplication instance.
 * @param user_data Pointer to the StartData structure.
 **/
static void
activate(GtkApplication *app,
	 gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  home_window(app, data);
}

/**
 * Launch the initial graphic user interface.
 *
 * @param port The inital port.
 * @param server_ip The inital server_ip.
 **/
void
launch_gui(int port,
	   char* server_ip)
{
  //allocate memory to the user initial data struct
  StartData *data = g_new0(StartData, 1);
  data->server_ip = g_strdup(server_ip);
  data->port = port;
  //Create the gtk app and connect it with the active signal
  GtkApplication *app;
  app = gtk_application_new("org.chat.client", G_APPLICATION_NON_UNIQUE);
  g_signal_connect_data(app, "activate", G_CALLBACK (activate), data, (GClosureNotify)g_free, G_CONNECT_AFTER);
  //Run the gtk application
  g_application_run(G_APPLICATION (app), 0, NULL);
}
