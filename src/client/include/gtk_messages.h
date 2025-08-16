#ifndef GTK_MESSAGES_H
#define GTK_MESSAGES_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/**
 * Build a GTK widget representing a chat message.
 *
 * @param msg Pointer to the ChatMessage struct containing message data.
 * @return A GtkWidget representing the formatted message. The caller owns a reference.
 **/
GtkWidget* build_message(ChatMessage *msg);

/**
 * Add a new message to a chat and update the GUI.
 *
 * @param chat Pointer to the Chat struct where the message will be stored.
 * @param chatty Pointer to the ChatData struct managing the GUI.
 * @param type The type of message (OWN_MESSAGE, NORMAL_MESSAGE, INFO_MESSAGE, etc.).
 * @param sender The sender name of the message.
 * @param content The text content of the message.
 **/
void add_new_message(Chat *chat, ChatData *chatty, MessageType type, const char* sender, const char* content);

/**
 * Handle sending a message from the input entry.
 * Reads the text from the message entry, sends it to the controller and updates the chat GUI.
 *
 * @param widget The widget triggering this callback (typically the send button).
 * @param user_data Pointer to the ChatData struct managing the GUI and current chat.
 **/
void send_message(GtkWidget *widget, gpointer user_data);

#endif // GTK_MESSAGES_H
