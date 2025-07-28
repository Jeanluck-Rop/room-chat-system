#include "idle.h"

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
message_received_idle(gpointer call_data)
{
  MessageIdle *data = (MessageIdle *)call_data;
  message_received(data->chat_name, data->sender, data->content, data->chat_type, data->msg_type);
  g_free(data->chat_name);
  g_free(data->sender);
  g_free(data->content);
  g_free(data);
  return G_SOURCE_REMOVE;
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
