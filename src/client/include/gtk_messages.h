#ifndef GTK_MESSAGES_H
#define GTK_MESSAGES_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/* */
GtkWidget* build_message(ChatMessage *msg);

/* */
void add_new_message(Chat *chat, ChatData *chatty, MessageType type, const char* sender, const char* content);

/* */
void send_message(GtkWidget *widget, gpointer user_data);

#endif // GTK_MESSAGES_H
