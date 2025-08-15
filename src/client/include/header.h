#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/* */
void on_send_message_request(GtkButton *button, gpointer user_data);

/* */
void populate_user_list(GtkBuilder *builder, GtkListBox *list_box, char **usernames, char **statuses, GCallback send_callback);
  
/* */
void set_header(Chat *chat, GtkWidget *header);
  
#endif // HEADER_H
