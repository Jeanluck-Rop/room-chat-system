#include "header.h"

static const char* headers_ui = "/org/chat/client/resources/headers.ui";

/* */
void
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
void
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
    row_builder = gtk_builder_new_from_resource(headers_ui);
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
void
set_header(Chat *chat,
	   GtkWidget *header)
{
  GtkWidget *label;
  const char *header_id;
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(headers_ui);
  switch (chat->type) {
  case USER_CHAT:
    header_id = "user_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "user_name_label"));
    gtk_label_set_text(GTK_LABEL(label), chat->name);
    chat->status_label = GTK_WIDGET(gtk_builder_get_object(builder, "user_status_label"));
    int stts = controller_get_status(chat->name);
    const char *status;
    switch (stts) {
    case 1:
      status = "Away";
      break;
    case 2:
      status = "Busy";
      break;
    default:
      status = "Active";
      break;
    }
    
    gtk_label_set_text(GTK_LABEL(chat->status_label), status);
    break;
  case ROOM_CHAT:
    header_id = "room_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "room_name_label"));
    gtk_label_set_text(GTK_LABEL(label), chat->name);
    chat->room_count_label = GTK_WIDGET(gtk_builder_get_object(builder, "room_users_count"));
    int room_count = controller_get_count(chat->name);
    char *formatt = g_strdup_printf("%d members", room_count);
    gtk_label_set_text(GTK_LABEL(chat->room_count_label), formatt);
    g_free(formatt);
    break;
  default:
    header_id = "public_header";
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
