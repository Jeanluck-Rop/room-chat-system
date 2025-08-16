#include "gtk_messages.h"

static const char* message_ui = "/org/chat/client/resources/message.ui";

/**
 * Build a GTK widget representing a chat message.
 *
 * @param msg Pointer to the ChatMessage struct containing message data.
 * @return A GtkWidget representing the formatted message. The caller owns a reference.
 **/
GtkWidget*
build_message(ChatMessage *msg)
{
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource(message_ui);
  GtkWidget *root;
  root = GTK_WIDGET(gtk_builder_get_object(builder, "message_root"));
  GtkWidget *msg_box;
  if (msg->type == INFO_MESSAGE) {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "info_message_form"));
    GtkWidget *content_label;
    content_label= GTK_WIDGET(gtk_builder_get_object(builder, "info_label"));
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  } else if (msg->type == NORMAL_MESSAGE) {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "message_form"));
    GtkWidget *sender_label;
    sender_label = GTK_WIDGET(gtk_builder_get_object(builder, "sender_label"));
    GtkWidget *content_label;
    content_label = GTK_WIDGET(gtk_builder_get_object(builder, "content_label"));
    gtk_label_set_text(GTK_LABEL(sender_label), msg->sender);
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  } else {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "own_message_form"));
    GtkWidget *sender_label;
    sender_label = GTK_WIDGET(gtk_builder_get_object(builder, "you_label"));
    GtkWidget *content_label;
    content_label = GTK_WIDGET(gtk_builder_get_object(builder, "your_content_label"));
    gtk_label_set_text(GTK_LABEL(sender_label), msg->sender);
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  }
  g_object_ref(msg_box);
  g_object_unref(builder);
  return msg_box;
}

/**
 * Add a new message to a chat and update the GUI.
 *
 * @param chat Pointer to the Chat struct where the message will be stored.
 * @param chatty Pointer to the ChatData struct managing the GUI.
 * @param type The type of message (OWN_MESSAGE, NORMAL_MESSAGE, INFO_MESSAGE, etc.).
 * @param sender The sender name of the message.
 * @param content The text content of the message.
 **/
void
add_new_message(Chat *chat,
		ChatData *chatty,
		MessageType type,
		const char* sender,
		const char* content)
{
  ChatMessage *msg = g_new0(ChatMessage, 1);
  msg->type = type;
  msg->sender = g_strdup(sender);
  msg->content = g_strdup(content);
  chat->messages = g_list_append(chat->messages, msg);
  //update the gui to show the newest message
  if (chatty->current_chat && g_strcmp0(chatty->current_chat->name, chat->name) == 0) {
    GtkWidget *msg_widget = build_message(msg);
    gtk_box_append(GTK_BOX(chatty->messages_box), msg_widget);
    g_timeout_add(50, scroll_to_bottom, chatty->messages_scroll);
  } else if (chat->row)
    gtk_widget_add_css_class(chat->row, "new-message");
  if (chat->recent_label)
    gtk_label_set_text(GTK_LABEL(chat->recent_label), content);
}

/**
 * Handle sending a message from the input entry.
 * Reads the text from the message entry, sends it to the controller and updates the chat GUI.
 *
 * @param widget The widget triggering this callback (typically the send button).
 * @param user_data Pointer to the ChatData struct managing the GUI and current chat.
 **/
void
send_message(GtkWidget *widget,
	     gpointer user_data)
{
  ChatData *chatty = (ChatData *)user_data;
  GtkEntryBuffer *buffer;
  buffer = gtk_entry_get_buffer(GTK_ENTRY(chatty->message_entry));
  const char *msg_content = gtk_entry_buffer_get_text(buffer);
  if (msg_content && *msg_content != '\0') {
    add_new_message(chatty->current_chat, chatty, OWN_MESSAGE, "You", msg_content);
    if (chatty->current_chat->type == ROOM_CHAT)
      controller_room_message(msg_content, chatty->current_chat->name);
    else if (chatty->current_chat->type == USER_CHAT)
      controller_direct_message(msg_content, chatty->current_chat->name);
    else
      controller_public_message(msg_content);
    gtk_entry_buffer_set_text(buffer, "", -1);
    gtk_widget_set_sensitive(chatty->send_button, FALSE);
  }
  g_idle_add(focus_message_entry, chatty->message_entry);
}
