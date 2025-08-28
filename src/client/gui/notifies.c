#include "notifies.h"

static const char* invitation_ui = "/org/chat/client/resources/invitation.ui";

/**
 * Remove a notification from the list and free its memory.
 *
 * @param msg The message of the notification to remove.
 * @param user_data Pointer to an InviteData struct containing notification list.
 **/
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

/**
 * Handle the event when an invitation is accepted.
 *
 * @param button The accept button clicked.
 * @param user_data Pointer to an InviteData struct containing the notification.
 **/
static void
on_invite_accepted(GObject *source_object,
		   GAsyncResult *res,
		   gpointer user_data)
{
  GtkAlertDialog *dialog = GTK_ALERT_DIALOG(source_object);
  int response = gtk_alert_dialog_choose_finish(dialog, res, NULL);
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  if (response == 1) {
    controller_join_room(notif->room_name);
    remove_notify(notif->message, data);
  }
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/**
 * Display a popover asking the user to confirm joining a room.
 *
 * @param button The notification button clicked.
 * @param user_data Pointer to an InviteData struct containing the notification.
 **/
static void
on_invitation_clicked(GtkButton *button,
		      gpointer user_data)
{
  InviteData *data = (InviteData *)user_data;
  Notify *notif = data->notif;
  Notifs *notifs = data->notifs;
  gtk_popover_popdown(notifs->popover);
  ChatData *chatty = get_chat_data();
  GtkWindow *parent = chatty->window;
  GtkAlertDialog *alert;
  const char *message = "Invitation";
  const char *detail = g_strdup_printf("Are you sure you want to join to the room [%s]?", notif->room_name);
  const char *buttons[] = { "Cancel", "OK", NULL };
  alert = gtk_alert_dialog_new(message);
  gtk_alert_dialog_set_detail(alert, detail);
  gtk_alert_dialog_set_buttons(alert, buttons);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_choose(alert, parent, NULL, on_invite_accepted, data);
}

/**
 * Handle the event when a normal notification is clicked.
 *
 * @param button The notification button clicked.
 * @param user_data Pointer to a Notifs struct containing the notifications list.
 **/
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

/**
 * Handle the event when a normal notification is clicked.
 *
 * @param button The notification button clicked.
 * @param user_data Pointer to a Notifs struct containing the notifications list.
 **/
static void
display_notifications(GtkButton *button,
		      gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  GtkWidget *listbox = notifs->listbox;
  clear_widget(notifs->listbox); //clear previous childs
  if (!notifs->list) {
    GtkWidget *label = gtk_label_new("No notifications yet");
    gtk_widget_add_css_class(label, "no-notifs-label");
    gtk_box_append(GTK_BOX(listbox), label);
  } else {
    for (GList *l = notifs->list; l; l = l->next) {
      Notify *notif = l->data;
      GtkWidget *row = gtk_button_new_with_label(notif->message);
      gtk_widget_add_css_class(row, "notif-item");
      gtk_widget_add_css_class(row, "notif-label");
      if (notif->type == NORMAL_NOTIF)
        g_signal_connect(row, "clicked", G_CALLBACK(on_normal_notify_clicked), notifs);
      else if (notif->type == INVITE_NOTIF) {
	InviteData *data = g_new0(InviteData, 1);
        data->notif = notif;
        data->notifs = notifs;
        g_signal_connect_data(row, "clicked", G_CALLBACK(on_invitation_clicked), data, (GClosureNotify)g_free, 0);
      }
      gtk_box_append(GTK_BOX(listbox), row);
    }
  }
  gtk_popover_popup(notifs->popover);
  ChatData *chatty = get_chat_data();
  g_idle_add(focus_message_entry, chatty->message_entry);
}

/**
 * Initialize and attach notification popover functionality to the chat interface.
 *
 * @param chatty Pointer to the main chat data structure.
 **/
void
set_notifs(ChatData *chatty)
{
  Notifs *notifs = g_new0(Notifs, 1);
  notifs->button = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_button"));
  notifs->popover = GTK_POPOVER(gtk_builder_get_object(chatty->builder, "notifs_popover"));
  notifs->listbox = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_list"));
  gtk_popover_set_has_arrow(notifs->popover, FALSE);
  gtk_popover_set_autohide(notifs->popover, TRUE);
  g_signal_connect(notifs->button, "clicked", G_CALLBACK(display_notifications), notifs);
  chatty->notifs = notifs;
}
