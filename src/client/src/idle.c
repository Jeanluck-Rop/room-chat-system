#include "idle.h"

/* */
static void
free_users_and_statuses(char **users, char **statuses)
{
  if (users) {
    for (int i = 0; users[i] != NULL; i++)
      g_free(users[i]);
    g_free(users);
  }
  if (statuses) {
    for (int i = 0; statuses[i] != NULL; i++)
      g_free(statuses[i]);
    g_free(statuses);
  }
}

/* */
gboolean
alert_dialog_idle(gpointer call_data)
{
  DialogIdle *data = (DialogIdle *)call_data;
  alert_dialog(data->detail, data->type);
  g_free(data->detail);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
update_status_idle(gpointer call_data)
{
  StatusIdle *data = (StatusIdle *)call_data;
  update_user_status(data->user_name, data->status);
  g_free(data->user_name);
  g_free(data->status);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
update_count_idle(gpointer call_data)
{
  CountIdle *data = (CountIdle *)call_data;
  update_count(data->chat_name, data->users_count);
  g_free(data->chat_name);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
room_users_idle(gpointer call_data)
{
  RoomUsersListIdle *data = (RoomUsersListIdle *)call_data;
  show_room_users(data->room_name, data->users, data->statuses);
  g_free(data->room_name);
  free_users_and_statuses(data->users, data->statuses);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
show_users_idle(gpointer call_data)
{
  ChatUsersListIdle *data = (ChatUsersListIdle *)call_data;
  if (current_users_request == USERS_REQUEST_INVITE)
    show_invitation_window(data->users, data->statuses);
  else
    show_chat_users(data->users, data->statuses);
  free_users_and_statuses(data->users, data->statuses);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
message_received_idle(gpointer call_data)
{
  MessageIdle *data = (MessageIdle *)call_data;
  message_received(data->chat_name, data->sender, data->content, data->chat_type, data->msg_type);
  g_free(data->chat_name);
  if (data->sender)
    g_free(data->sender);
  if (data->content)
    g_free(data->content);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
new_notify_idle(gpointer call_data)
{
  NotifyIdle *data = (NotifyIdle *)call_data;
  add_new_notify(data->msg, data->room_name, data->type);
  g_free(data->msg);
  g_free(data->room_name);
  g_free(data);
  return G_SOURCE_REMOVE;
}

/* */
gboolean
enter_chat_idle(gpointer call_data)
{
  enter_chat();
  return G_SOURCE_REMOVE;
}

/* */
gboolean
init_alert_dialog_idle(gpointer call_data)
{
  DialogIdle *data = (DialogIdle *)call_data;
  init_alert_dialog(data->detail, data->type);
  g_free(data->detail);
  g_free(data);
  return G_SOURCE_REMOVE;
}
