#include "view.h"

UsersRequestType current_users_request = USERS_REQUEST_NONE;

/*      Auxiliar functions for handle the gtk gui     */

/* */
static void
load_css(const char *resource_path)
{
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, resource_path);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/* */
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

/* */
static gboolean
on_start_window_close(GtkWindow *window,
                      gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  gtk_window_destroy(window);
  g_application_quit(g_application_get_default());
  return TRUE;
}

/* */
static gboolean
free_chat_data(gpointer data)
{
  ChatData *chatty = (ChatData *)data;
  g_free(chatty->ip);
  g_free(chatty->username);
  g_free(chatty);
  return G_SOURCE_REMOVE;
}

/* */
static ChatData*
get_chat_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "chat-data");
}

/* */
void
back_to_home_page()
{
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

/* */
static Chat*
get_chat(const char *name,
	 ChatData *chatty)
{
  for (GList *l = chatty->chats; l != NULL; l = l->next) {
    Chat *chat = l->data;
    if (g_strcmp0(chat->name, name) == 0) {
      return chat;
    }
  }
  return NULL; // Not found
}

/* */
static void
clear_widget(GtkWidget *widget)
{
  GtkWidget *child;
  GtkWidget *next;
  child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    next = gtk_widget_get_next_sibling(child);
    gtk_widget_unparent(child);
    child = next;
  }
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

/* */
static void
free_chat_message(ChatMessage *msg)
{
  if (!msg)
    return;
  g_free(msg->sender);
  g_free(msg->content);
  g_free(msg);
}

/* */
static gboolean
focus_message_entry(gpointer user_data)
{
  GtkWidget *entry = GTK_WIDGET(user_data);
  gtk_widget_grab_focus(entry);
  return G_SOURCE_REMOVE;
}

/* */
static gboolean
scroll_to_bottom(gpointer user_data)
{
  GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(user_data);
  GtkAdjustment *vadjustment = gtk_scrolled_window_get_vadjustment(scroll);
  gtk_adjustment_set_value(vadjustment, gtk_adjustment_get_upper(vadjustment));
  return G_SOURCE_REMOVE;
}

/* */
static void
on_entry_changed(GtkEditable *editable,
		 gpointer user_data)
{
  EntryValidation *data = (EntryValidation *)user_data;
  GtkEntryBuffer *buffer = gtk_entry_get_buffer(data->entry);
  const char *text = gtk_entry_buffer_get_text(buffer);
  size_t len = text ? strlen(text) : 0;

  gboolean valid = (len >= data->min_len && len <= data->max_len);
  gtk_widget_set_sensitive(data->accept_button, valid);
}

/* */
static gboolean
on_dialog_close(GtkWindow *window,
		gpointer user_data)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  return TRUE;
}

/* */
static void
on_cancel_clicked(GtkButton *button,
		  GtkWindow *window)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
on_cancel_button(GtkBuilder *builder,
		 GtkWindow *window,
		 const char *cancel_id)
{
  GtkWidget *cancel = GTK_WIDGET(gtk_builder_get_object(builder, cancel_id));
  g_signal_connect(cancel, "clicked", G_CALLBACK(on_cancel_clicked), window);
}

/* */
static void
connect_accept_once(const char* accept_id,
		    GCallback callback,
		    ChatActions *actions)
{
  GtkWidget *accept;
  accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, accept_id));
  g_signal_handlers_disconnect_by_func(accept, callback, actions);
  g_signal_connect(accept, "clicked", callback, actions);
}

/* */
static void
show_modal_window(GtkWidget *parent,
		  GtkWidget *window)
{
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gtk_widget_get_root(parent)));
  gtk_window_present(GTK_WINDOW(window));
}
///


/*      */

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
  gtk_label_set_text(GTK_LABEL(chat->status_label), status);
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
///


/*      */

/* */
static GtkWidget*
build_message(ChatMessage *msg)
{
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/message.ui");
  GtkWidget *root;
  root = GTK_WIDGET(gtk_builder_get_object(builder, "message_root"));
  GtkWidget *msg_box;
  if (msg->type == INFO_MESSAGE) {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "info_message_form"));
    GtkWidget *content_label;
    content_label= GTK_WIDGET(gtk_builder_get_object(builder, "info_label"));
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  } else if (msg->type == NORMAL_MESSAGE) {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "message_form"));
    GtkWidget *sender_label;
    sender_label = GTK_WIDGET(gtk_builder_get_object(builder, "sender_label"));
    GtkWidget *content_label;
    content_label = GTK_WIDGET(gtk_builder_get_object(builder, "content_label"));
    gtk_label_set_text(GTK_LABEL(sender_label), msg->sender);
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  } else {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "own_message_form"));
    GtkWidget *sender_label;
    sender_label = GTK_WIDGET(gtk_builder_get_object(builder, "you_label"));
    GtkWidget *content_label;
    content_label = GTK_WIDGET(gtk_builder_get_object(builder, "your_content_label"));
    gtk_label_set_text(GTK_LABEL(sender_label), msg->sender);
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  }
  g_object_ref(msg_box);
  g_object_unref(builder);
  return msg_box;
}

/* */
static void
add_new_message(Chat *chat,
		ChatData *chatty,
		MessageType type,
		const char* sender,
		const char* content)
{
  ChatMessage *msg = g_new0(ChatMessage, 1);
  msg->type = type;
  msg->sender = g_strdup(sender);
  msg->content = g_strdup(content);
  chat->messages = g_list_append(chat->messages, msg);
  //update the gui to show the newest message
  if (chatty->current_chat && g_strcmp0(chatty->current_chat->name, chat->name) == 0) {
    GtkWidget *msg_widget = build_message(msg);
    gtk_box_append(GTK_BOX(chatty->messages_box), msg_widget);
    g_timeout_add(50, scroll_to_bottom, chatty->messages_scroll);
  } else if (chat->row)
    gtk_widget_add_css_class(chat->row, "new-message");
  if (chat->recent_label)
    gtk_label_set_text(GTK_LABEL(chat->recent_label), content);
}

/* */
static Chat*
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
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/rows.ui");
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

/* */
static void
remove_chat(ChatData *chatty, Chat *chat)
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

/* */
void
delete_user_chat_row(const char* user_name)
{
  ChatData *chatty = get_chat_data();
  Chat *user_chat = get_chat(user_name, chatty);
  if (!user_chat)
    return;
  remove_chat(chatty, user_chat);
}
///

/*     Functions for handle the actions button and delegate each chat action     */

/* */
static void
free_used_actions(gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  if (actions->selected_room_btn)
    actions->selected_room_btn = NULL;
  if(actions->selected_roomname)
    g_clear_pointer(&actions->selected_roomname, g_free);
  if (actions->selected_users) {
    for (int i = 0; actions->selected_users[i]; i++)
      g_free(actions->selected_users[i]);
    g_free(actions->selected_users);
    actions->selected_users = NULL;
  }
}

/* */
static void
new_status_changed(GtkButton *button,
		   GtkWindow *window)
{
  const char *status;
  const char *name;
  name = gtk_widget_get_name(GTK_WIDGET(button));
  if (g_strcmp0(name, "away_button") == 0)
    status = "AWAY";
  else if (g_strcmp0(name, "busy_button") == 0)
    status = "BUSY";
  else
    status = "ACTIVE";

  controller_change_status(status);
  
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
connect_status_button(GtkBuilder *builder,
                      const char *button_id,
                      GtkWindow *window)
{
  GtkWidget *button = GTK_WIDGET(gtk_builder_get_object(builder, button_id));
  gtk_widget_set_name(button, button_id);
  //avoid multiple connections
  g_signal_handlers_disconnect_by_func(button, G_CALLBACK(new_status_changed), window);
  g_signal_connect(button, "clicked", G_CALLBACK(new_status_changed), window);
}


/* */
static void
change_status_clicked(GtkButton *button,
		      gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  actions->change_status_window = GTK_WINDOW(gtk_builder_get_object(actions->builder, "change_status_window"));
  show_modal_window(GTK_WIDGET(actions->main_button), GTK_WIDGET(actions->change_status_window));
  g_signal_connect(actions->change_status_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  connect_status_button(actions->builder, "active_button", actions->change_status_window);
  connect_status_button(actions->builder, "away_button", actions->change_status_window);
  connect_status_button(actions->builder, "busy_button", actions->change_status_window);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
new_room_accept(GtkButton *button,
		gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  GtkEntry *entry;
  entry = GTK_ENTRY(gtk_builder_get_object(actions->builder, "new_room_name"));
  GtkEntryBuffer *buffer;
  buffer = gtk_entry_get_buffer(entry);
  const char *roomname = gtk_entry_buffer_get_text(buffer);
  controller_new_room(roomname);
  gtk_entry_buffer_set_text(buffer, "", -1);
  gtk_widget_set_visible(GTK_WIDGET(actions->new_room_window), FALSE);
}

/* */
static void
room_entry_changed(GtkWidget *entry,
		   GtkWidget *accept)
{
  EntryValidation *val_data = g_new0(EntryValidation, 1);
  val_data->entry = GTK_ENTRY(entry);
  val_data->accept_button = accept;
  val_data->min_len = 3;
  val_data->max_len = 16;
  g_signal_handlers_disconnect_by_func(entry, G_CALLBACK(on_entry_changed), val_data);
  g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), val_data);
}

/* */
static void
new_room_clicked(GtkButton *button,
		 gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  actions->new_room_window = GTK_WINDOW(gtk_builder_get_object(actions->builder, "new_room_window"));
  show_modal_window(GTK_WIDGET(actions->main_button), GTK_WIDGET(actions->new_room_window));
  g_signal_connect(actions->new_room_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  GtkWidget *entry;
  entry = GTK_WIDGET(gtk_builder_get_object(actions->builder, "new_room_name"));
  GtkWidget *accept;
  accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "new_room_accept_button"));
  gtk_widget_set_sensitive(accept, FALSE);
  //avoid multiple conncetions
  room_entry_changed(entry, accept);
  connect_accept_once("new_room_accept_button", G_CALLBACK(new_room_accept), actions);
  on_cancel_button(actions->builder, actions->new_room_window, "new_room_cancel_button");
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
room_selected(GtkButton *button,
              ChatActions *actions)
{
  if (actions->selected_room_btn != NULL) {
    gtk_button_set_label(actions->selected_room_btn, "Select");
    gtk_widget_set_sensitive(GTK_WIDGET(actions->selected_room_btn), TRUE);
  }
  gtk_button_set_label(button, "Selected");
  gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
  actions->selected_room_btn = button;
  GtkWidget *row;
  row = gtk_widget_get_parent(GTK_WIDGET(button));
  GtkWidget *label;
  label = gtk_widget_get_first_child(GTK_WIDGET(row));
  const char *roomname;
  roomname = gtk_label_get_text(GTK_LABEL(label));
  g_clear_pointer(&actions->selected_roomname, g_free);
  actions->selected_roomname = g_strdup(roomname); 
}

/* */
static void
display_rooms_list(GtkListBox *list_box,
		   const char* row_id,
		   GCallback callback,
		   gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  ChatData *chatty = get_chat_data();
  gtk_list_box_remove_all(list_box);
  for (GList *r = chatty->chats; r; r = r->next) {
    Chat *chat = r->data;
    if (chat->type != ROOM_CHAT)
      continue;
    GtkBuilder *row_builder;
    row_builder = gtk_builder_new_from_resource("/org/chat/client/resources/actions.ui");
    GtkWidget *template_row;
    template_row = GTK_WIDGET(gtk_builder_get_object(row_builder, row_id));
    gtk_widget_set_visible(template_row, TRUE);
    GtkWidget *box;
    box = gtk_widget_get_first_child(template_row);
    GtkWidget *roomname_label;
    roomname_label = gtk_widget_get_first_child(box);
    GtkWidget *select_btn;
    select_btn = gtk_widget_get_next_sibling(roomname_label);
    gtk_label_set_text(GTK_LABEL(roomname_label), chat->name);
    g_signal_connect(select_btn, "clicked", callback, actions);
    gtk_list_box_append(list_box, template_row);
    g_object_unref(row_builder);
  }
  gtk_widget_set_visible(GTK_WIDGET(list_box), TRUE);
}

/* */
static void
display_users_list(GtkListBox *list_box,
		   char** usernames,
		   char **statuses,
		   GCallback callback,
		   gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  ChatData* chatty = get_chat_data();
  char* user = chatty->username;
  gtk_list_box_remove_all(list_box);
  for (int i = 0; usernames[i] != NULL; i++) {
    GtkBuilder *row_builder;
    row_builder = gtk_builder_new_from_resource("/org/chat/client/resources/actions.ui");
    GtkWidget *template_row;
    template_row = GTK_WIDGET(gtk_builder_get_object(row_builder, "user_av_list_item"));
    gtk_widget_set_visible(template_row, TRUE);
    GtkWidget *box;
    box = gtk_widget_get_first_child(template_row);
    GtkWidget *username_label;
    username_label = gtk_widget_get_first_child(box);
    GtkWidget *status_label;
    status_label = gtk_widget_get_next_sibling(username_label);
    GtkWidget *add_btn;
    add_btn = gtk_widget_get_next_sibling(status_label);
    gtk_label_set_text(GTK_LABEL(username_label), usernames[i]);
    gtk_label_set_text(GTK_LABEL(status_label), statuses[i]);
    if (g_strcmp0(user, usernames[i]) == 0)
      gtk_widget_set_sensitive(add_btn, FALSE);
    g_object_set_data_full(G_OBJECT(add_btn), "username", g_strdup(usernames[i]), g_free);
    g_signal_connect(add_btn, "clicked", callback, actions);
    gtk_list_box_append(list_box, template_row);
    g_object_unref(row_builder);
  }
  gtk_widget_set_visible(GTK_WIDGET(list_box), TRUE);
}

/* */
static void
invite_users_accept(GtkButton *button,
		    gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  controller_invite_users(actions->selected_users, actions->selected_roomname);
  gtk_widget_set_visible(GTK_WIDGET(actions->invite_users_window), FALSE);
  free_used_actions(actions);
}

/* */
static void
guests_selected(GtkButton *button,
                gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  const char *username = g_object_get_data(G_OBJECT(button), "username");
  GtkWidget *label = gtk_widget_get_first_child(GTK_WIDGET(button));
  gboolean found = FALSE;
  int count = 0;
  if (actions->selected_users) {
    while (actions->selected_users[count]) {
      if (g_strcmp0(actions->selected_users[count], username) == 0)
        found = TRUE;
      count++;
    }
  }
  //remove guest selected
  if (found) {
    char **new_list = g_new0(char *, count);
    int j = 0;
    for (int i = 0; actions->selected_users[i]; i++) {
      if (g_strcmp0(actions->selected_users[i], username) != 0)
        new_list[j++] = actions->selected_users[i];
      else
        g_free(actions->selected_users[i]);
    }
    g_free(actions->selected_users);
    actions->selected_users = new_list;
    gtk_label_set_text(GTK_LABEL(label), "Add"); //return to initial label
  } else {
    //if !found add to selected users
    char **new_list = g_new(char *, count + 2);
    for (int i = 0; i < count; i++)
        new_list[i] = actions->selected_users[i];//copy existing pointers
    new_list[count] = g_strdup(username);//duplicate each new string
    new_list[count + 1] = NULL;
    g_free(actions->selected_users);//free old array
    actions->selected_users = new_list;
    
    gtk_label_set_text(GTK_LABEL(label), "Quit");
  }
  GtkWidget *accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "invite_users_accept_button"));
  gboolean room_selected = actions->selected_roomname != NULL;
  gboolean has_users = actions->selected_users && actions->selected_users[0] != NULL;
  gtk_widget_set_sensitive(accept, room_selected && has_users);
}

/* */
static void
invitations_room_selected(GtkButton *button,
			  gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  room_selected(button, actions);
  if (actions->selected_roomname && actions->selected_users && actions->selected_users[0]) {
    GtkWidget *accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "invite_users_accept_button"));
    gtk_widget_set_sensitive(accept, TRUE);
  }
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

/* */
static void
invite_users_clicked(GtkButton *button,
		     gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  current_users_request = USERS_REQUEST_INVITE;
  controller_chat_users();
}

/* */
static void
leave_room_accept(GtkButton *button,
		  gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  ChatData *chatty = get_chat_data();
  Chat *room = get_chat(actions->selected_roomname, chatty);
  remove_chat(chatty, room);///
  controller_leave_room(actions->selected_roomname);///
  gtk_widget_set_visible(GTK_WIDGET(actions->leave_room_window), FALSE);
  free_used_actions(actions);
}

/* */
static void
room_to_leave_selected(GtkButton *button,
		       gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  room_selected(button, actions);
  GtkWidget *accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "leave_room_accept_button"));
  gtk_widget_set_sensitive(accept, TRUE);
}

/* */
static void
leave_room_clicked(GtkButton *button,
		   gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  actions->leave_room_window = GTK_WINDOW(gtk_builder_get_object(actions->builder, "leave_room_window"));
  show_modal_window(GTK_WIDGET(actions->main_button), GTK_WIDGET(actions->leave_room_window));
  g_signal_connect(actions->leave_room_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  GtkListBox *list_box;
  list_box = GTK_LIST_BOX(gtk_builder_get_object(actions->builder, "rooms_in_list"));
  display_rooms_list(list_box, "room_in_list_item", G_CALLBACK(room_to_leave_selected), actions);
  GtkWidget *accept;
  accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "leave_room_accept_button"));
  gtk_widget_set_sensitive(accept, FALSE);
  connect_accept_once("leave_room_accept_button", G_CALLBACK(leave_room_accept), actions);
  on_cancel_button(actions->builder, actions->leave_room_window, "leave_room_cancel_button");
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
disconnect_response(GObject *source_object,
		    GAsyncResult *res,
		    gpointer user_data)
{
  GtkAlertDialog *dialog = GTK_ALERT_DIALOG(source_object);
  int response = gtk_alert_dialog_choose_finish(dialog, res, NULL);
  if (response == 1)
    controller_disconnect();
}

/* */
static void
disconnect_user_clicked(GtkButton *button,
			gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  ChatData *chatty = get_chat_data();
  GtkWindow *parent = chatty->window;
  GtkAlertDialog *alert;
  const char *message = "Disconnect";
  const char *detail = "Are you sure you want to disconnect from the chat?";
  const char *buttons[] = { "Cancel", "OK", NULL };
  alert = gtk_alert_dialog_new(message);
  gtk_alert_dialog_set_detail(alert, detail);
  gtk_alert_dialog_set_buttons(alert, buttons);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_choose(alert, parent, NULL, disconnect_response, chatty);
}

/* */
static void
display_user_actions(GtkButton *button,
		     gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popup(actions->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
set_actions(GtkBuilder *builder,
	    gpointer user_data)
{
  ChatActions *actions = g_new0(ChatActions, 1);
  actions->builder = gtk_builder_new_from_resource("/org/chat/client/resources/actions.ui");
  actions->popover = GTK_POPOVER(gtk_builder_get_object(builder, "chat_actions_popover"));
  actions->main_button = GTK_WIDGET(gtk_builder_get_object(builder, "info_button"));
  g_signal_connect(actions->main_button, "clicked", G_CALLBACK(display_user_actions), actions);
  GtkWidget *disconnect_button;
  disconnect_button = GTK_WIDGET(gtk_builder_get_object(builder, "disconnect_button"));
  g_signal_connect(disconnect_button, "clicked", G_CALLBACK(disconnect_user_clicked), actions);
  gtk_popover_set_has_arrow(actions->popover, FALSE);
  gtk_popover_set_autohide(actions->popover, TRUE);
  GtkWidget *status_btn;
  status_btn = GTK_WIDGET(gtk_builder_get_object(builder, "change_status"));
  g_signal_connect(status_btn, "clicked", G_CALLBACK(change_status_clicked), actions);
  GtkWidget *new_room_btn;
  new_room_btn = GTK_WIDGET(gtk_builder_get_object(builder, "new_room"));
  g_signal_connect(new_room_btn, "clicked", G_CALLBACK(new_room_clicked), actions);
  GtkWidget *invite_users_btn;
  invite_users_btn = GTK_WIDGET(gtk_builder_get_object(builder, "invite_users"));
  g_signal_connect(invite_users_btn, "clicked", G_CALLBACK(invite_users_clicked), actions);
  GtkWidget *leave_room_btn;
  leave_room_btn = GTK_WIDGET(gtk_builder_get_object(builder, "leave_room"));
  g_signal_connect(leave_room_btn, "clicked", G_CALLBACK(leave_room_clicked), actions);
  GtkWidget *disconnect_user_btn;
  disconnect_user_btn = GTK_WIDGET(gtk_builder_get_object(builder, "disconnect_user"));
  g_signal_connect(disconnect_user_btn, "clicked", G_CALLBACK(disconnect_user_clicked), actions);
  
  ChatData *chatty = get_chat_data();
  chatty->actions = actions;
}
///


/*     Functions for handle the each chat header information     */

/* */
static void
on_send_message_request(GtkButton *button,
			gpointer user_data)
{
  const char *username = user_data;
  ChatData *chatty = get_chat_data();
  Chat *chat = get_chat(username, chatty);
  if (chat)
    load_main_page(chat, chatty);
  else {
    char *request = g_strdup_printf("Your chat with [%s]", username);
    Chat *nw_chat = new_chat(chatty, USER_CHAT, username, request);
    load_main_page(nw_chat, chatty);
    g_free(request);
  }
  GtkWidget *widget;
  widget = GTK_WIDGET(button);
  GtkWidget *ancestor;
  ancestor= gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
  if (ancestor)
    gtk_widget_set_visible(ancestor, FALSE);
  g_free(user_data);
}

/* */
static void
populate_user_list(GtkBuilder *builder,
		   GtkListBox *list_box,
                   char **usernames,
		   char **statuses,
                   GCallback send_callback)
{
  ChatData* chatty = get_chat_data();
  char* user = chatty->username;
  gtk_list_box_remove_all(list_box);
  for (int i = 0; usernames[i] != NULL; i++) {
    GtkBuilder *row_builder;
    row_builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
    GtkWidget *template_row;
    template_row = GTK_WIDGET(gtk_builder_get_object(row_builder, "user_list_item"));
    gtk_widget_set_visible(template_row, TRUE);
    GtkWidget *box;
    box = gtk_widget_get_first_child(template_row);
    GtkWidget *username_label;
    username_label = gtk_widget_get_first_child(box);
    GtkWidget *status_label;
    status_label = gtk_widget_get_next_sibling(username_label);
    GtkWidget *send_button;
    send_button = gtk_widget_get_next_sibling(status_label);
    gtk_label_set_text(GTK_LABEL(username_label), usernames[i]);
    gtk_label_set_text(GTK_LABEL(status_label), statuses[i]);

    if (g_strcmp0(user, usernames[i]) == 0)
      gtk_widget_set_sensitive(send_button, FALSE);
    
    g_signal_connect(send_button, "clicked", send_callback, g_strdup(usernames[i]));
    gtk_list_box_append(list_box, template_row);
    g_object_unref(row_builder);
  }
  gtk_widget_set_visible(GTK_WIDGET(list_box), TRUE);
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
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
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
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
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

/* */
static void
on_header_activated(GtkListBox *box,
                    GtkListBoxRow *row,
                    gpointer user_data)
{
  const char *name = gtk_widget_get_name(GTK_WIDGET(row));
  GtkBuilder *builder = GTK_BUILDER(user_data);
  if (g_strcmp0(name, "room_header") == 0) {
    GtkWidget *room_label;
    room_label = GTK_WIDGET(gtk_builder_get_object(builder, "room_name_label"));
    const char *room_name = gtk_label_get_text(GTK_LABEL(room_label));
    controller_room_users(room_name);
  } else if (g_strcmp0(name, "public_header") == 0) {
    current_users_request = USERS_REQUEST_PUBLIC;
    controller_chat_users();
  }
  else {
    GCallback send_callback;
    send_callback = G_CALLBACK(on_send_message_request);
    GtkWindow *info_window;
    info_window = GTK_WINDOW(gtk_builder_get_object(builder, "user_info_window"));
    gtk_window_set_modal(info_window, TRUE);
    gtk_window_set_transient_for(info_window, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(box))));
    g_signal_connect(info_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
    gtk_window_present(info_window);
  }
}

/* */
static void
set_header(Chat *chat,
	   GtkWidget *header)
{
  GtkWidget *label;
  const char *header_id;
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
  switch (chat->type) {
  case USER_CHAT:
    header_id = "user_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "user_name_label"));
    gtk_label_set_text(GTK_LABEL(label), chat->name);
    //GtkWidget *status_label;
    chat->status_label = GTK_WIDGET(gtk_builder_get_object(builder, "user_status_label"));
    const char *status = "ACTIVE";///
    gtk_label_set_text(GTK_LABEL(chat->status_label), status);
    break;
  case ROOM_CHAT:
    header_id = "room_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "room_name_label"));
    gtk_label_set_text(GTK_LABEL(label), chat->name);
    //GtkWidget *room_count_label;
    chat->room_count_label = GTK_WIDGET(gtk_builder_get_object(builder, "room_users_count"));
    int room_count = controller_get_count(chat->name);
    char *formatt = g_strdup_printf("%d members", room_count);
    gtk_label_set_text(GTK_LABEL(chat->room_count_label), formatt);
    g_free(formatt);
    break;
  default:
    header_id = "public_header";
    //GtkWidget *public_users_label;
    chat->public_count_label = GTK_WIDGET(gtk_builder_get_object(builder, "public_users_count"));
    int users_count = controller_get_count("PUBLIC_CHAT");
    char *formatted = g_strdup_printf("%d users", users_count);
    gtk_label_set_text(GTK_LABEL(chat->public_count_label), formatted);
    g_free(formatted);   
    break;
  }
  GtkWidget *custom_header;
  custom_header = GTK_WIDGET(gtk_builder_get_object(builder, header_id));
  gtk_list_box_append(GTK_LIST_BOX(header), GTK_WIDGET(custom_header));
  gtk_widget_set_visible(custom_header, TRUE);
  g_signal_connect(header, "row-activated", G_CALLBACK(on_header_activated), builder);
}
///


/*            */

/* */
static void
send_message(GtkWidget *widget,
	     gpointer user_data)
{
  ChatData *chatty = (ChatData *)user_data;
  GtkEntryBuffer *buffer;
  buffer = gtk_entry_get_buffer(GTK_ENTRY(chatty->message_entry));
  const char *msg_content = gtk_entry_buffer_get_text(buffer);
  if (msg_content && *msg_content != '\0') {
    add_new_message(chatty->current_chat, chatty, OWN_MESSAGE, "You", msg_content);
    if (chatty->current_chat->type == ROOM_CHAT)
      controller_room_message(msg_content, chatty->current_chat->name);
    else if (chatty->current_chat->type == USER_CHAT)
      controller_direct_message(msg_content, chatty->current_chat->name);
    else
      controller_public_message(msg_content);
    gtk_entry_buffer_set_text(buffer, "", -1);
    gtk_widget_set_sensitive(chatty->send_button, FALSE);
  }
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
check_message_entry(GtkWidget *entry,
		    GtkWidget *accept)
{
  EntryValidation *val_data = g_new0(EntryValidation, 1);
  val_data->entry = GTK_ENTRY(entry);
  val_data->accept_button = accept;
  val_data->min_len = 1;
  val_data->max_len = 512;
  g_signal_handlers_disconnect_by_func(entry, G_CALLBACK(on_entry_changed), val_data);
  g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), val_data);
}
///


/*     Load the main page of the chat     */

/**
 * Loads the main page view for a given chat.
 *
 * @param chat Pointer to the chat object to display.
 * @param user_data The ChatData from the app.
 **/
static void
load_main_page(Chat *chat,
	       gpointer user_data)
{
  if (chat->row)
    gtk_widget_remove_css_class(chat->row, "new-message");
  ChatData *chatty = (ChatData *)user_data;
  chatty->current_chat = chat;
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/main_page.ui");
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
///


/*     Functions for handle the chat rows and their messages    */

/* */
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
///


/*     Functions for handle the notifications     */

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
  notif->message = g_strdup(msg);
  notif->room_name = g_strdup(roomname);
  notif->type = type;
  chatty->notifs->list = g_list_append(chatty->notifs->list, notif);
  gtk_widget_add_css_class(chatty->notifs->button, "has-notifications");
}

/* */
static void
remove_notify(const char *msg,
	      gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notifs *notifs = data->notifs;
  GList *found = NULL;
  for (GList *l = notifs->list; l; l = l->next) {
    Notify *notif = l->data;
    if (g_strcmp0(notif->message, msg) == 0) {
      found = l;
      break;
    }
  }
  if (found) {
    Notify *notif = found->data;
    g_free(notif->message);
    g_free(notif->room_name);
    g_free(notif);
    notifs->list = g_list_delete_link(notifs->list, found);
  }
  if (!notifs->list)
    gtk_widget_remove_css_class(notifs->button, "has-notifications");
}

/* */
static void
on_invite_accepted(GtkButton *button,
		   gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW);
  controller_join_room(notif->room_name);
  remove_notify(notif->message, data);
  gtk_widget_set_visible(window, FALSE);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
on_invitation_clicked(GtkButton *button,
		      gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  Notifs *notifs = data->notifs;
  char *text = g_strdup_printf("Are you sure you want to join to the room [%s]?", notif->room_name);
  gtk_popover_popdown(notifs->popover);
  GtkBuilder *builder;
  builder = builder = gtk_builder_new_from_resource("/org/chat/client/resources/invitation.ui");
  GtkWidget *window;
  window = GTK_WIDGET(gtk_builder_get_object(builder, "invitation_window"));
  GtkWidget *label;
  label = GTK_WIDGET(gtk_builder_get_object(builder, "invitation_label"));
  gtk_label_set_text(GTK_LABEL(label), text);
  GtkWidget *accept;
  accept= GTK_WIDGET(gtk_builder_get_object(builder, "accept_button"));
  on_cancel_button(builder, GTK_WINDOW(window), "cancel_button");
  g_signal_handlers_disconnect_by_func(accept, G_CALLBACK(on_invite_accepted), data);
  g_signal_connect(accept, "clicked", G_CALLBACK(on_invite_accepted), data);
  show_modal_window(GTK_WIDGET(notifs->button), window);
  g_free(text);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
on_normal_notify_clicked(GtkButton *button,
			 gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  const char *msg = gtk_button_get_label(button);
  InviteData *data = g_new0(InviteData, 1);
  data->notifs = notifs;
  remove_notify(msg, data);
  gtk_popover_popdown(notifs->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
display_notifications(GtkButton *button,
		      gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  clear_widget(notifs->box); //clear previous childs
  if (!notifs->list) {
    GtkWidget *label = gtk_label_new("No notifications yet");
    gtk_widget_add_css_class(label, "no-notifs-label");
    gtk_box_append(GTK_BOX(notifs->box), label);
  } else {
    for (GList *l = notifs->list; l; l = l->next) {
      Notify *notif = l->data;
      GtkWidget *btn = gtk_button_new_with_label(notif->message);
      gtk_widget_add_css_class(btn, "notif-item");
      if (notif->type == NORMAL_NOTIF)
        g_signal_connect(btn, "clicked", G_CALLBACK(on_normal_notify_clicked), notifs);
      else if (notif->type == INVITE_NOTIF) {
	InviteData *data = g_new0(InviteData, 1);
        data->notif = notif;
        data->notifs = notifs;
        g_signal_connect_data(btn, "clicked", G_CALLBACK(on_invitation_clicked), data, (GClosureNotify)g_free, 0);
      }
      gtk_box_append(GTK_BOX(notifs->box), btn);
    }
  }
  gtk_popover_popup(notifs->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
set_notifs(ChatData *chatty)
{
  Notifs *notifs = g_new0(Notifs, 1);
  notifs->button = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_button"));
  notifs->popover = GTK_POPOVER(gtk_builder_get_object(chatty->builder, "notifs_popover"));
  gtk_popover_set_has_arrow(notifs->popover, FALSE);
  gtk_popover_set_autohide(notifs->popover, TRUE);
  notifs->box = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_box"));
  g_signal_connect(notifs->button, "clicked", G_CALLBACK(display_notifications), notifs);
  chatty->notifs = notifs;
}
///


///
static void fake_chat_info(ChatData *chatty) {
  Chat *room_chat = new_chat(chatty, ROOM_CHAT, "Room 1", "Welcome to [Room 1]");
  Chat *room_cha = new_chat(chatty, ROOM_CHAT, "Room A", "Welcome to [Room A]");
  add_new_notify("Jose se desconecto del chat", NULL, NORMAL_NOTIF);
  add_new_notify("Jose abando no el cuarto {Pumitas}", NULL, NORMAL_NOTIF);
  add_new_notify("Aileen se unió al chat", NULL, NORMAL_NOTIF);
  add_new_notify("Sarah te invito al cuarto [Pumitas]", "Pumitas", INVITE_NOTIF);
  add_new_notify("Pablo te invito al cuarto [Panas]", "Panas", INVITE_NOTIF);
}
///


/*     Functions for handle the user initial connection     */

/* */
static StartData*
get_init_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "init-data");
}

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
  load_css("/org/chat/client/resources/css/chat.css");
  load_css("/org/chat/client/resources/css/rows.css");
  load_css("/org/chat/client/resources/css/headers.css");
  load_css("/org/chat/client/resources/css/main_page.css");
  load_css("/org/chat/client/resources/css/notifies.css");
  load_css("/org/chat/client/resources/css/actions.css");
  load_css("/org/chat/client/resources/css/invitation.css");

  //Define the chat builder
  chatty->builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  
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
  Chat *public_chat = new_chat(chatty, PUBLIC_CHAT, "PUBLIC_CHAT", "Welcome to the Public Chat!");
  
  //connect each chat_row
  g_signal_connect(chatty->chats_list, "row-activated", G_CALLBACK(on_row_selected), chatty);

  //load the public chat page
  load_main_page(public_chat, chatty);
  
  //testing fake_chat_info(chatty);

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

/* */
static int
get_port(StartData *data)
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(data->port_entry));
  return atoi(port_text);
}

/* */
static const char*
get_ip(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->ip_entry));
}

/* */
static const char*
get_username(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->user_name_entry));
}

/* */
static void
on_port_changed(GtkEditable *editable,
	        gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  const char* text = gtk_editable_get_text(editable);
  data->port = atoi(text);
}

/* */
static void
on_user_name_changed(GtkWidget *entry,
		     GtkWidget *accept)
{
  EntryValidation *ent_data = g_new0(EntryValidation, 1);
  ent_data->entry = GTK_ENTRY(entry);
  ent_data->accept_button = accept;
  ent_data->min_len = 2;
  ent_data->max_len = 8;
  g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), ent_data);
}

/* */
static void
quit(GtkButton *button,
     gpointer user_data)
{
  g_application_quit(g_application_get_default());
}

/* */
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
  controller_try_connection(port, ip, user_name); //enter_chat();
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
  load_css("/org/chat/client/resources/css/start.css");
  
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/start.ui");
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

/* */
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
  app = gtk_application_new("org.chat.client",
			    //G_APPLICATION_DEFAULT_FLAGS
			    G_APPLICATION_NON_UNIQUE);
  g_signal_connect_data(app, "activate", G_CALLBACK (activate), data, (GClosureNotify)g_free, G_CONNECT_AFTER);
  //Run the gtk application
  g_application_run(G_APPLICATION (app), 0, NULL);
}
