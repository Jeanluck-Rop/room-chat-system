#include "notifies.h"

static const char* invitation_ui = "/org/chat/client/resources/invitation.ui";

/* */
static void
remove_notify(const char *msg,
	      gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notifs *notifs = data->notifs;
  GList *found = NULL;
  for (GList *l = notifs->list; l; l = l->next) {
    Notify *notif = l->data;
    if (g_strcmp0(notif->message, msg) == 0) {
      found = l;
      break;
    }
  }
  if (found) {
    Notify *notif = found->data;
    g_free(notif->message);
    g_free(notif->room_name);
    g_free(notif);
    notifs->list = g_list_delete_link(notifs->list, found);
  }
  if (!notifs->list)
    gtk_widget_remove_css_class(notifs->button, "has-notifications");
}

/* */
static void
on_invite_accepted(GtkButton *button,
		   gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW);
  controller_join_room(notif->room_name);
  remove_notify(notif->message, data);
  gtk_widget_set_visible(window, FALSE);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
on_invitation_clicked(GtkButton *button,
		      gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  Notifs *notifs = data->notifs;
  char *text = g_strdup_printf("Are you sure you want to join to the room [%s]?", notif->room_name);
  gtk_popover_popdown(notifs->popover);
  GtkBuilder *builder;
  builder = builder = gtk_builder_new_from_resource(invitation_ui);
  GtkWidget *window;
  window = GTK_WIDGET(gtk_builder_get_object(builder, "invitation_window"));
  GtkWidget *label;
  label = GTK_WIDGET(gtk_builder_get_object(builder, "invitation_label"));
  gtk_label_set_text(GTK_LABEL(label), text);
  GtkWidget *accept;
  accept= GTK_WIDGET(gtk_builder_get_object(builder, "accept_button"));
  on_cancel_button(builder, GTK_WINDOW(window), "cancel_button");
  g_signal_handlers_disconnect_by_func(accept, G_CALLBACK(on_invite_accepted), data);
  g_signal_connect(accept, "clicked", G_CALLBACK(on_invite_accepted), data);
  show_modal_window(GTK_WIDGET(notifs->button), window);
  g_free(text);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
on_normal_notify_clicked(GtkButton *button,
			 gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  const char *msg = gtk_button_get_label(button);
  InviteData *data = g_new0(InviteData, 1);
  data->notifs = notifs;
  remove_notify(msg, data);
  gtk_popover_popdown(notifs->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
static void
display_notifications(GtkButton *button,
		      gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  clear_widget(notifs->box); //clear previous childs
  if (!notifs->list) {
    GtkWidget *label = gtk_label_new("No notifications yet");
    gtk_widget_add_css_class(label, "no-notifs-label");
    gtk_box_append(GTK_BOX(notifs->box), label);
  } else {
    for (GList *l = notifs->list; l; l = l->next) {
      Notify *notif = l->data;
      GtkWidget *btn = gtk_button_new_with_label(notif->message);
      gtk_widget_add_css_class(btn, "notif-item");
      if (notif->type == NORMAL_NOTIF)
        g_signal_connect(btn, "clicked", G_CALLBACK(on_normal_notify_clicked), notifs);
      else if (notif->type == INVITE_NOTIF) {
	InviteData *data = g_new0(InviteData, 1);
        data->notif = notif;
        data->notifs = notifs;
        g_signal_connect_data(btn, "clicked", G_CALLBACK(on_invitation_clicked), data, (GClosureNotify)g_free, 0);
      }
      gtk_box_append(GTK_BOX(notifs->box), btn);
    }
  }
  gtk_popover_popup(notifs->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/* */
void
set_notifs(ChatData *chatty)
{
  Notifs *notifs = g_new0(Notifs, 1);
  notifs->button = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_button"));
  notifs->popover = GTK_POPOVER(gtk_builder_get_object(chatty->builder, "notifs_popover"));
  gtk_popover_set_has_arrow(notifs->popover, FALSE);
  gtk_popover_set_autohide(notifs->popover, TRUE);
  notifs->box = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_box"));
  g_signal_connect(notifs->button, "clicked", G_CALLBACK(display_notifications), notifs);
  chatty->notifs = notifs;
}
