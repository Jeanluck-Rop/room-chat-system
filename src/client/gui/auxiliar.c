#include "auxiliar.h"

/* */
void
load_css(const char *resource_path)
{
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, resource_path);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/* */
gboolean
on_start_window_close(GtkWindow *window,
                      gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  gtk_window_destroy(window);
  g_application_quit(g_application_get_default());
  return TRUE;
}

/* */
gboolean
free_chat_data(gpointer data)
{
  ChatData *chatty = (ChatData *)data;
  g_free(chatty->ip);
  g_free(chatty->username);
  g_free(chatty);
  return G_SOURCE_REMOVE;
}

/* */
ChatData*
get_chat_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "chat-data");
}

/* */
Chat*
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
void
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

/* */
void
free_chat_message(ChatMessage *msg)
{
  if (!msg)
    return;
  g_free(msg->sender);
  g_free(msg->content);
  g_free(msg);
}

/* */
gboolean
focus_message_entry(gpointer user_data)
{
  GtkWidget *entry = GTK_WIDGET(user_data);
  gtk_widget_grab_focus(entry);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
scroll_to_bottom(gpointer user_data)
{
  GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(user_data);
  GtkAdjustment *vadjustment = gtk_scrolled_window_get_vadjustment(scroll);
  gtk_adjustment_set_value(vadjustment, gtk_adjustment_get_upper(vadjustment));
  return G_SOURCE_REMOVE;
}

/* */
void
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
gboolean
on_dialog_close(GtkWindow *window,
		gpointer user_data)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  return TRUE;
}

/* */
void
on_cancel_clicked(GtkButton *button,
		  GtkWindow *window)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
}

/* */
void
on_cancel_button(GtkBuilder *builder,
		 GtkWindow *window,
		 const char *cancel_id)
{
  GtkWidget *cancel = GTK_WIDGET(gtk_builder_get_object(builder, cancel_id));
  g_signal_connect(cancel, "clicked", G_CALLBACK(on_cancel_clicked), window);
}

/* */
void
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
void
show_modal_window(GtkWidget *parent,
		  GtkWidget *window)
{
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gtk_widget_get_root(parent)));
  gtk_window_present(GTK_WINDOW(window));
}

/* */
void
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

/* */
StartData*
get_init_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "init-data");
}

/* */
int
get_port(StartData *data)
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(data->port_entry));
  return atoi(port_text);
}

/* */
const char*
get_ip(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->ip_entry));
}

/* */
const char*
get_username(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->user_name_entry));
}

/* */
void
on_port_changed(GtkEditable *editable,
	        gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  const char* text = gtk_editable_get_text(editable);
  data->port = atoi(text);
}

/* */
void
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
void
quit(GtkButton *button,
     gpointer user_data)
{
  g_application_quit(g_application_get_default());
}
