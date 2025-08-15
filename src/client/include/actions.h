#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/* */
void display_rooms_list(GtkListBox *list_box, const char* row_id, GCallback callback, gpointer user_data);

/* */
void display_users_list(GtkListBox *list_box, char** usernames, char **statuses, GCallback callback, gpointer user_data);
  
/* */
void guests_selected(GtkButton *button, gpointer user_data);

/* */
void invitations_room_selected(GtkButton *button, gpointer user_data);

/* */
void invite_users_accept(GtkButton *button, gpointer user_data);

/* */
void set_actions(GtkBuilder *builder, gpointer user_data);

#endif // ACTIONS_H
