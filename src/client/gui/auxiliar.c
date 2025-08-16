#include "auxiliar.h"

/**
 * Load a CSS resource and apply it to the application.
 *
 * @param resource_path The resource path to the CSS file.
 **/
void
load_css(const char *resource_path)
{
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, resource_path);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/**
 * Handle the close event of the start window.
 * Destroys the window and quits the GTK application.
 *
 * @param window The window being closed.
 * @param user_data Pointer to StartData struct (unused here but passed by GTK).
 * @return TRUE to indicate the event has been handled.
 **/
gboolean
on_start_window_close(GtkWindow *window,
                      gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  gtk_window_destroy(window);
  g_application_quit(g_application_get_default());
  return TRUE;
}

/**
 * Free the memory associated with a ChatData struct.
 *
 * @param data Pointer to the ChatData struct to free.
 * @return G_SOURCE_REMOVE to indicate removal from event sources.
 **/
gboolean
free_chat_data(gpointer data)
{
  ChatData *chatty = (ChatData *)data;
  g_free(chatty->ip);
  g_free(chatty->username);
  g_free(chatty);
  return G_SOURCE_REMOVE;
}

/**
 * Retrieve the active ChatData associated with the current application window.
 *
 * @return Pointer to the ChatData struct stored in the window's data.
 **/
ChatData*
get_chat_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "chat-data");
}

/**
 * Search for a chat by name in the ChatData struct.
 *
 * @param name The name of the chat to find.
 * @param chatty Pointer to the ChatData struct containing the list of chats.
 * @return Pointer to the Chat struct if found, or NULL if not found.
 **/
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

/**
 * Remove all child widgets from a container widget.
 *
 * @param widget The container widget to clear.
 **/
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

/**
 * Free the memory associated with a ChatMessage struct.
 *
 * @param msg Pointer to the ChatMessage struct to free.
 **/
void
free_chat_message(ChatMessage *msg)
{
  if (!msg)
    return;
  g_free(msg->sender);
  g_free(msg->content);
  g_free(msg);
}

/**
 * Grab focus for a GtkEntry widget.
 *
 * @param user_data Pointer to the GtkEntry widget.
 * @return G_SOURCE_REMOVE to remove this callback from the main loop.
 **/
gboolean
focus_message_entry(gpointer user_data)
{
  GtkWidget *entry = GTK_WIDGET(user_data);
  gtk_widget_grab_focus(entry);
  return G_SOURCE_REMOVE;
}

/**
 * Scroll a GtkScrolledWindow to the bottom.
 *
 * @param user_data Pointer to the GtkScrolledWindow to scroll.
 * @return G_SOURCE_REMOVE to remove this callback from the main loop.
 **/
gboolean
scroll_to_bottom(gpointer user_data)
{
  GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(user_data);
  GtkAdjustment *vadjustment = gtk_scrolled_window_get_vadjustment(scroll);
  gtk_adjustment_set_value(vadjustment, gtk_adjustment_get_upper(vadjustment));
  return G_SOURCE_REMOVE;
}

/**
 * Validate the contents of a GtkEntry widget and enable/disable a button.
 *
 * @param editable The GtkEditable widget being edited.
 * @param user_data Pointer to an EntryValidation struct containing entry info and constraints.
 **/
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

/**
 * Handle the close event of a dialog window.
 *
 * Hides the dialog instead of destroying it.
 *
 * @param window The GtkWindow being closed.
 * @param user_data Pointer to user data (unused).
 * @return TRUE to indicate the event has been handled.
 **/
gboolean
on_dialog_close(GtkWindow *window,
		gpointer user_data)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  return TRUE;
}

/**
 * Handle the cancel button click for a window.
 *
 * Hides the specified window.
 *
 * @param button The button that was clicked.
 * @param window The GtkWindow to hide.
 **/
void
on_cancel_clicked(GtkButton *button,
		  GtkWindow *window)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
}

/**
 * Connect a cancel button in a builder to hide a window when clicked.
 *
 * @param builder The GtkBuilder containing the cancel button.
 * @param window The GtkWindow to hide when the button is clicked.
 * @param cancel_id The object ID of the cancel button in the builder.
 **/
void
on_cancel_button(GtkBuilder *builder,
		 GtkWindow *window,
		 const char *cancel_id)
{
  GtkWidget *cancel = GTK_WIDGET(gtk_builder_get_object(builder, cancel_id));
  g_signal_connect(cancel, "clicked", G_CALLBACK(on_cancel_clicked), window);
}

/**
 * Connect an accept button to a callback and ensure it is only connected once.
 *
 * @param accept_id The object ID of the accept button in the builder.
 * @param callback The callback to connect to the button.
 * @param actions Pointer to ChatActions containing the builder and context.
 **/
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

/**
 * Show a window as a modal dialog relative to a parent widget.
 *
 * @param parent The parent GtkWidget for transient relationship.
 * @param window The GtkWidget to present as modal.
 **/
void
show_modal_window(GtkWidget *parent,
		  GtkWidget *window)
{
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gtk_widget_get_root(parent)));
  gtk_window_present(GTK_WINDOW(window));
}

/**
 * Set up validation for a message entry and connect it to an accept button.
 *
 * @param entry The GtkEntry widget for message input.
 * @param accept The GtkWidget (button) to enable/disable based on validation.
 **/
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

/**
 * Retrieve the initial data associated with the start window.
 *
 * @return Pointer to the StartData struct stored in the window's data.
 **/
StartData*
get_init_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "init-data");
}

/**
 * Get the port number entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the port entry.
 * @return The port number as an integer.
 **/
int
get_port(StartData *data)
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(data->port_entry));
  return atoi(port_text);
}

/**
 * Get the IP address entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the IP entry.
 * @return The IP address as a string.
 **/
const char*
get_ip(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->ip_entry));
}

/**
 * Get the username entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the username entry.
 * @return The username as a string.
 **/
const char*
get_username(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->user_name_entry));
}

/**
 * Handle changes to the port entry field and update StartData.
 *
 * @param editable The GtkEditable widget being edited.
 * @param user_data Pointer to the StartData struct.
 **/
void
on_port_changed(GtkEditable *editable,
	        gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  const char* text = gtk_editable_get_text(editable);
  data->port = atoi(text);
}

/**
 * Set up validation for the username entry and connect it to an accept button.
 *
 * @param entry The GtkWidget for username input.
 * @param accept The GtkWidget (button) to enable/disable based on validation.
 **/
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

/**
 * Quit the GTK application.
 *
 * @param button The button triggering the quit.
 * @param user_data Pointer to user data (unused).
 **/
void
quit(GtkButton *button,
     gpointer user_data)
{
  g_application_quit(g_application_get_default());
}
