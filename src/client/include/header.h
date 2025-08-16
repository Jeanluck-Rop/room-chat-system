#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/**
 * Handle sending a message request when a user is selected.
 * Opens the chat with the specified user or creates a new chat if it does not exist.
 * Hides the window containing the button after processing.
 *
 * @param button The button triggering the request.
 * @param user_data Pointer to a string containing the username to chat with.
 **/
void on_send_message_request(GtkButton *button, gpointer user_data);

/**
 * Populate a GtkListBox with user items and associated statuses.
 * Each row contains the username, status, and a send message button.
 * The send button is disabled for the current user.
 *
 * @param builder The GtkBuilder used to construct row templates.
 * @param list_box The GtkListBox to populate.
 * @param usernames Array of strings containing usernames (NULL-terminated).
 * @param statuses Array of strings containing user statuses (NULL-terminated).
 * @param send_callback Callback function to connect to the send button.
 **/
void populate_user_list(GtkBuilder *builder, GtkListBox *list_box, char **usernames, char **statuses, GCallback send_callback);
  
/**
 * Set up a chat header widget based on the chat type.
 * Configures labels and counts for user, room, or public chats.
 * Connects the header's row-activated signal to handle user interactions.
 *
 * @param chat Pointer to the Chat struct containing chat data.
 * @param header The GtkWidget container where the header will be added.
 **/
void set_header(Chat *chat, GtkWidget *header);
  
#endif // HEADER_H
