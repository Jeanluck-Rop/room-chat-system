#include "actions.h"

static const char* actions_ui = "/org/chat/client/resources/actions.ui";

/**
 * Frees the memory used by stored chat actions.
 * Clears selected room button, room name, and list of selected users.
 *
 * @param user_data Pointer to the ChatActions structure to free.
 **/
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

/**
 * Handles the click on a status change button.
 * Updates the user's status, hides the status change window, 
 * and focuses the message entry field.
 *
 * @param button The GTK button clicked.
 * @param window The status change GtkWindow.
 **/
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

/**
 * Connects a status change button to the click handler.
 * Ensures that multiple connections are avoided.
 *
 * @param builder The GtkBuilder containing the button.
 * @param button_id The ID of the button widget.
 * @param window The status change GtkWindow.
 **/
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

/**
 * Handles the "change status" button click.
 * Opens the status change window, sets up status change buttons.
 *
 * @param button The GTK button clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
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

/**
 * Handles the acceptance of a new room creation.
 * Sends the new room name to the controller, clears the input, 
 * and hides the new room window.
 *
 * @param button The GTK button clicked to accept.
 * @param user_data Pointer to the ChatActions structure.
 **/
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

/**
 * Creates an EntryValidation structure, configures length constraints,
 * and connects the "changed" signal to the on_entry_changed handler.
 *
 * @param entry The room name GtkWidget entry.
 * @param accept The accept GtkWidget button to enable/disable.
 **/
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

/**
 * Handles the "new room" button click.
 * Opens the new room creation window, sets up entry, connects accept and cancel actions.
 *
 * @param button The GTK button clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
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

/**
 * Handles selection of a room from the room list.
 * Updates button labels, disables the selected button, 
 * and stores the selected room name in ChatActions.
 *
 * @param button The GTK button clicked to select the room.
 * @param actions Pointer to the ChatActions structure.
 **/
static void
room_selected(GtkButton *button,
              ChatActions *actions)
{
  if (actions->selected_room_btn != NULL) {
    gtk_button_set_label(actions->selected_room_btn, "Select");
    gtk_widget_remove_css_class(GTK_WIDGET(actions->selected_room_btn), "is-clicked");
    gtk_widget_set_sensitive(GTK_WIDGET(actions->selected_room_btn), TRUE);
  }
  gtk_button_set_label(button, "Selected");
  gtk_widget_add_css_class(GTK_WIDGET(button), "is-clicked");
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

/**
 * Displays the list of available rooms in the UI.
 *
 * @param list_box The GtkListBox to populate with room entries.
 * @param row_id The resource ID of the row template.
 * @param callback The function to call when the "select" button is clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
void
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
    row_builder = gtk_builder_new_from_resource(actions_ui);
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

/**
 * Displays a list of available users in the UI.
 * Populates a GtkListBox with rows containing usernames, statuses, 
 * and an "Add" button for inviting them to a room.
 * Disables the "Add" button for the current user.
 *
 * @param list_box The GtkListBox to populate with user entries.
 * @param usernames Null-terminated array of usernames.
 * @param statuses Null-terminated array of statuses corresponding to the usernames.
 * @param callback The function to call when the "Add" button is clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
void
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
    row_builder = gtk_builder_new_from_resource(actions_ui);
    GtkWidget *template_row;
    template_row = GTK_WIDGET(gtk_builder_get_object(row_builder, "user_av_list_item"));
    gtk_widget_set_visible(template_row, TRUE);
    GtkWidget *box;
    box = gtk_widget_get_first_child(template_row);
    GtkWidget *username_label;
    username_label = gtk_widget_get_first_child(box);
    const char *status;
    if (g_strcmp0(statuses[i], "AWAY") == 0)
      status = "Away";
    else if (g_strcmp0(statuses[i], "BUSY") == 0)
      status = "Busy";
    else
      status = "Active";
    GtkWidget *status_label;
    status_label = gtk_widget_get_next_sibling(username_label);
    GtkWidget *add_btn;
    add_btn = gtk_widget_get_next_sibling(status_label);
    gtk_label_set_text(GTK_LABEL(username_label), usernames[i]);
    gtk_label_set_text(GTK_LABEL(status_label), status);
    if (g_strcmp0(user, usernames[i]) == 0)
      gtk_widget_set_sensitive(add_btn, FALSE);
    g_object_set_data_full(G_OBJECT(add_btn), "username", g_strdup(usernames[i]), g_free);
    g_signal_connect(add_btn, "clicked", callback, actions);
    gtk_list_box_append(list_box, template_row);
    g_object_unref(row_builder);
  }
  gtk_widget_set_visible(GTK_WIDGET(list_box), TRUE);
}

/**
 * Handles acceptance of an invite for selected users.
 * Sends the list of selected users and room name to the controller.
 *
 * @param button The GTK button clicked to accept.
 * @param user_data Pointer to the ChatActions structure.
 **/
void
invite_users_accept(GtkButton *button,
		    gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  controller_invite_users(actions->selected_users, actions->selected_roomname);
  gtk_widget_set_visible(GTK_WIDGET(actions->invite_users_window), FALSE);
  free_used_actions(actions);
}

/**
 * Handles the selection and deselection of guests in the invite list.
 *
 * @param button The GTK button clicked to toggle guest selection.
 * @param user_data Pointer to the ChatActions structure.
 **/
void
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
    gtk_widget_remove_css_class(GTK_WIDGET(button), "is-clicked");
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
    gtk_widget_add_css_class(GTK_WIDGET(button), "is-clicked");
  }
  GtkWidget *accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "invite_users_accept_button"));
  gboolean room_selected = actions->selected_roomname != NULL;
  gboolean has_users = actions->selected_users && actions->selected_users[0] != NULL;
  gtk_widget_set_sensitive(accept, room_selected && has_users);
}

/**
 * Handles selection of a room in the invitations UI.
 *
 * @param button The GTK button clicked to select the room.
 * @param user_data Pointer to the ChatActions structure.
 **/
void
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
 * Handles the "invite users" button click.
 * Sets the users request type for invitations,
 * and requests the list of chat users from the controller.
 *
 * @param button The GTK button clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
static void
invite_users_clicked(GtkButton *button,
		     gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popdown(actions->popover);
  current_users_request = USERS_REQUEST_INVITE;
  controller_chat_users();
}

/**
 * Handles the "leave room" button click.
 * Removes the room from the user's chat list, sends a leave request to the controller.
 *
 * @param button The GTK button triggering this action.
 * @param user_data Pointer to the ChatActions structure.
 **/
static void
leave_room_accept(GtkButton *button,
		  gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  ChatData *chatty = get_chat_data();
  Chat *room = get_chat(actions->selected_roomname, chatty);
  remove_chat(chatty, room);
  controller_leave_room(actions->selected_roomname);
  gtk_widget_set_visible(GTK_WIDGET(actions->leave_room_window), FALSE);
  free_used_actions(actions);
}

/**
 * Handles the selection of a room to leave.
 * It marks the room as selected and enables the accept button.
 *
 * @param button The GTK button representing the selected room.
 * @param user_data Pointer to the ChatActions structure.
 **/
static void
room_to_leave_selected(GtkButton *button,
		       gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  room_selected(button, actions);
  GtkWidget *accept = GTK_WIDGET(gtk_builder_get_object(actions->builder, "leave_room_accept_button"));
  gtk_widget_set_sensitive(accept, TRUE);
}

/**
 * Displays the "leave room" dialog.
 * Shows a dialog listing all rooms the user can leave.
 * The accept button is initially disabled until a room is selected.
 *
 * @param button The GTK button triggering this action.
 * @param user_data Pointer to the ChatActions structure.
 **/
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

/**
 * Handles the response to the disconnect confirmation dialog.
 * Checks the user's selection in the disconnect alert dialog and triggers the
 * disconnect process if confirmed.
 *
 * @param source_object  The GtkAlertDialog that generated the response.
 * @param res            The asynchronous result of the dialog.
 * @param user_data      User-defined data (unused here).
 **/
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

/**
 * Handles the "leave room" button click.
 * Displays a confirmation dialog for disconnecting from the chat.
 *
 * @param button The GTK button triggering this action.
 * @param user_data Pointer to the ChatActions structure.
 **/
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

/**
 * Displays the user actions popover.
 * It shows the actions popover.
 *
 * @param button The GTK button triggering this action.
 * @param user_data Pointer to the ChatActions structure.
 **/
static void
display_user_actions(GtkButton *button,
		     gpointer user_data)
{
  ChatActions *actions = (ChatActions *)user_data;
  gtk_popover_popup(actions->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/**
 * Initializes and connects all chat-related UI actions.
 * This function creates a new ChatActions structure, loads the UI elements
 * from resources, and connects all relevant buttons to their corresponding callbacks.
 *
 * @param builder The GtkBuilder object containing the main UI.
 * @param user_data User-defined data (unused here).
 **/
void
set_actions(GtkBuilder *builder,
	    gpointer user_data)
{
  ChatActions *actions = g_new0(ChatActions, 1);
  actions->builder = gtk_builder_new_from_resource(actions_ui);
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
