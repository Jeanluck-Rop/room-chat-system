#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "models.h"

/* */
void load_css(const char *resource_path);

/* */
gboolean on_start_window_close(GtkWindow *window, gpointer user_data);

/* */
gboolean free_chat_data(gpointer data);

/* */
ChatData* get_chat_data();

/* */
Chat* get_chat(const char *name, ChatData *chatty);
  
/* */
void clear_widget(GtkWidget *widget);

/* */
void free_chat_message(ChatMessage *msg);

/* */
gboolean focus_message_entry(gpointer user_data);

/* */
gboolean scroll_to_bottom(gpointer user_data);

/* */
void on_entry_changed(GtkEditable *editable, gpointer user_data);

/* */
gboolean on_dialog_close(GtkWindow *window, gpointer user_data);

/* */
void on_cancel_clicked(GtkButton *button, GtkWindow *window);

/* */
void on_cancel_button(GtkBuilder *builder, GtkWindow *window, const char *cancel_id);

/* */
void connect_accept_once(const char* accept_id, GCallback callback, ChatActions *actions);

/* */
void show_modal_window(GtkWidget *parent, GtkWidget *window);

/* */
void check_message_entry(GtkWidget *entry, GtkWidget *accept);

/* */
StartData* get_init_data();

/* */
int get_port(StartData *data);

/* */
const char* get_ip(StartData *data);

/* */
const char* get_username(StartData *data);

void on_port_changed(GtkEditable *editable, gpointer user_data);

/* */
void on_user_name_changed(GtkWidget *entry, GtkWidget *accept);

/* */
void quit(GtkButton *button, gpointer user_data);
  
#endif // AUXILIAR_H
