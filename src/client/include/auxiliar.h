#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "models.h"

/**
 * Load a CSS resource and apply it to the application.
 *
 * @param resource_path The resource path to the CSS file.
 **/
void load_css(const char *resource_path);

/**
 * Handle the close event of the start window.
 * Destroys the window and quits the GTK application.
 *
 * @param window The window being closed.
 * @param user_data Pointer to StartData struct (unused here but passed by GTK).
 * @return TRUE to indicate the event has been handled.
 **/
gboolean on_start_window_close(GtkWindow *window, gpointer user_data);

/**
 * Free the memory associated with a ChatData struct.
 *
 * @param data Pointer to the ChatData struct to free.
 * @return G_SOURCE_REMOVE to indicate removal from event sources.
 **/
gboolean free_chat_data(gpointer data);

/**
 * Retrieve the active ChatData associated with the current application window.
 *
 * @return Pointer to the ChatData struct stored in the window's data.
 **/
ChatData* get_chat_data();

/**
 * Search for a chat by name in the ChatData struct.
 *
 * @param name The name of the chat to find.
 * @param chatty Pointer to the ChatData struct containing the list of chats.
 * @return Pointer to the Chat struct if found, or NULL if not found.
 **/
Chat* get_chat(const char *name, ChatData *chatty);
  
/**
 * Remove all child widgets from a container widget.
 *
 * @param widget The container widget to clear.
 **/
void clear_widget(GtkWidget *widget);

/**
 * Free the memory associated with a ChatMessage struct.
 *
 * @param msg Pointer to the ChatMessage struct to free.
 **/
void free_chat_message(ChatMessage *msg);

/**
 * Grab focus for a GtkEntry widget.
 *
 * @param user_data Pointer to the GtkEntry widget.
 * @return G_SOURCE_REMOVE to remove this callback from the main loop.
 **/
gboolean focus_message_entry(gpointer user_data);

/**
 * Scroll a GtkScrolledWindow to the bottom.
 *
 * @param user_data Pointer to the GtkScrolledWindow to scroll.
 * @return G_SOURCE_REMOVE to remove this callback from the main loop.
 **/
gboolean scroll_to_bottom(gpointer user_data);

/**
 * Validate the contents of a GtkEntry widget and enable/disable a button.
 *
 * @param editable The GtkEditable widget being edited.
 * @param user_data Pointer to an EntryValidation struct containing entry info and constraints.
 **/
void on_entry_changed(GtkEditable *editable, gpointer user_data);

/**
 * Handle the close event of a dialog window.
 *
 * Hides the dialog instead of destroying it.
 *
 * @param window The GtkWindow being closed.
 * @param user_data Pointer to user data (unused).
 * @return TRUE to indicate the event has been handled.
 **/
gboolean on_dialog_close(GtkWindow *window, gpointer user_data);

/**
 * Handle the cancel button click for a window.
 *
 * Hides the specified window.
 *
 * @param button The button that was clicked.
 * @param window The GtkWindow to hide.
 **/
void on_cancel_clicked(GtkButton *button, GtkWindow *window);

/**
 * Connect a cancel button in a builder to hide a window when clicked.
 *
 * @param builder The GtkBuilder containing the cancel button.
 * @param window The GtkWindow to hide when the button is clicked.
 * @param cancel_id The object ID of the cancel button in the builder.
 **/
void on_cancel_button(GtkBuilder *builder, GtkWindow *window, const char *cancel_id);

/**
 * Connect an accept button to a callback and ensure it is only connected once.
 *
 * @param accept_id The object ID of the accept button in the builder.
 * @param callback The callback to connect to the button.
 * @param actions Pointer to ChatActions containing the builder and context.
 **/
void connect_accept_once(const char* accept_id, GCallback callback, ChatActions *actions);

/**
 * Show a window as a modal dialog relative to a parent widget.
 *
 * @param parent The parent GtkWidget for transient relationship.
 * @param window The GtkWidget to present as modal.
 **/
void show_modal_window(GtkWidget *parent, GtkWidget *window);

/**
 * Set up validation for a message entry and connect it to an accept button.
 *
 * @param entry The GtkEntry widget for message input.
 * @param accept The GtkWidget (button) to enable/disable based on validation.
 **/
void check_message_entry(GtkWidget *entry, GtkWidget *accept);

/**
 * Retrieve the initial data associated with the start window.
 *
 * @return Pointer to the StartData struct stored in the window's data.
 **/
StartData* get_init_data();

/**
 * Get the port number entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the port entry.
 * @return The port number as an integer.
 **/
int get_port(StartData *data);

/**
 * Get the IP address entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the IP entry.
 * @return The IP address as a string.
 **/
const char* get_ip(StartData *data);

/**
 * Get the username entered in the start window.
 *
 * @param data Pointer to the StartData struct containing the username entry.
 * @return The username as a string.
 **/
const char* get_username(StartData *data);

/**
 * Handle changes to the port entry field and update StartData.
 *
 * @param editable The GtkEditable widget being edited.
 * @param user_data Pointer to the StartData struct.
 **/
void on_port_changed(GtkEditable *editable, gpointer user_data);

/**
 * Set up validation for the username entry and connect it to an accept button.
 *
 * @param entry The GtkWidget for username input.
 * @param accept The GtkWidget (button) to enable/disable based on validation.
 **/
void on_user_name_changed(GtkWidget *entry, GtkWidget *accept);

/**
 * Quit the GTK application.
 *
 * @param button The button triggering the quit.
 * @param user_data Pointer to user data (unused).
 **/
void quit(GtkButton *button, gpointer user_data);
  
#endif // AUXILIAR_H
