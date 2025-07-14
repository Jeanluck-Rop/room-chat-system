#include "view.h"

/**
 * Class constants and variables.
 **/
static GtkWidget *port_entry = NULL;
static GtkWidget *ip_entry = NULL;
static int current_port = 0;

GtkWidget *main_content = NULL;
GtkWidget *chats_list = NULL;
GList *all_chats = NULL;

/**
 *
 * Auxiliar functions.
 *
 **/
/* */
int
get_port()
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(port_entry));
  return atoi(port_text);
}

/* */
const char
*get_ip()
{
  return gtk_editable_get_text(GTK_EDITABLE(ip_entry));
}

/* */
static void
quit(GtkButton *button,
     gpointer user_data)
{
  g_application_quit(g_application_get_default());
}

/* */
static void
on_port_changed(GtkEditable *editable,
		gpointer user_data)
{
  const char* text = gtk_editable_get_text(editable);
  current_port = atoi(text);
}

/* */
static void
clear_widget(GtkWidget *widget)
{
  GtkWidget *child;
  child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    GtkWidget *next;
    next = gtk_widget_get_next_sibling(child);
    gtk_widget_unparent(child);
    child = next;
  }
}

/* */
ChatMessage
*create_message(MessageType type,
		const char *sender,
		const char *content)
{
  ChatMessage *msg = g_new0(ChatMessage, 1);
  msg->type = type;
  msg->sender = g_strdup(sender);
  msg->content = g_strdup(content);
  return msg;
}

/* */
static void
load_css(const char *resource_path)
{
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, resource_path);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
///////


/**
 *
 * Main functions to handle the gui
 *
 **/

/* */
GtkWidget
*build_message(ChatMessage *msg)
{
 
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/message.ui");
  GtkWidget *root;
  root = GTK_WIDGET(gtk_builder_get_object(builder, "message_root"));
  
  GtkWidget *msg_box;
  if (msg->type == MESSAGE_TYPE_INFO) {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "info_message_form"));
    GtkWidget *content_label;
    content_label= GTK_WIDGET(gtk_builder_get_object(builder, "info_label"));
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  } else {
    msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "message_form"));
    GtkWidget *sender_label;
    sender_label = GTK_WIDGET(gtk_builder_get_object(builder, "sender_label"));
    GtkWidget *content_label;
    content_label = GTK_WIDGET(gtk_builder_get_object(builder, "content_label"));
    gtk_label_set_text(GTK_LABEL(sender_label), msg->sender);
    gtk_label_set_text(GTK_LABEL(content_label), msg->content);
  }

  g_object_ref(msg_box);
  g_object_unref(builder);
  return msg_box;
}

/* */
void
load_main_page(Chat *chat)
{
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/main_page.ui");
  GtkWidget *row_page;
  row_page = GTK_WIDGET(gtk_builder_get_object(builder, "row_page"));
  GtkWidget *header_left;
  header_left = GTK_WIDGET(gtk_builder_get_object(builder, "header_left"));
  GtkWidget *messages_box;
  messages_box = GTK_WIDGET(gtk_builder_get_object(builder, "messages_box"));
  GtkWidget *message_entry;
  message_entry = GTK_WIDGET(gtk_builder_get_object(builder, "message_entry"));
  GtkWidget *send_button;
  send_button = GTK_WIDGET(gtk_builder_get_object(builder, "send_button"));
  
  //clean previous content
  clear_widget(main_content);

  //load header
  const char *header_id;
  switch (chat->type) {
    case CHAT_TYPE_PUBLIC:
      header_id = "public_header";
      break;
    case CHAT_TYPE_USER:
      header_id = "user_header";
      break;
    case CHAT_TYPE_ROOM:
      header_id = "room_header";
      break;
    default:
      header_id = "room_header";
      break;
  }
  GtkBuilder *header_builder;
  header_builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
  GtkWidget *custom_header;
  custom_header = GTK_WIDGET(gtk_builder_get_object(header_builder, header_id));
  if (custom_header) {
    gtk_box_append(GTK_BOX(header_left), custom_header);
    gtk_widget_set_visible(custom_header, TRUE);
  }

  //load messages logic
  for (GList *l = chat->messages; l; l = l->next) {
    ChatMessage *msg = l->data;
    GtkWidget *widget = build_message(msg);
    if (GTK_IS_WIDGET(widget))
      gtk_box_append(GTK_BOX(messages_box), widget);
    else
      g_warning("build_message failed for: %s", msg->sender);
  }
  
  //send button logic

  gtk_box_append(GTK_BOX(main_content), row_page);
  gtk_widget_set_visible(main_content, TRUE);
  g_object_unref(builder);
}

/* */
static void
on_row_selected(GtkListBox *box,
		GtkListBoxRow *row,
		gpointer user_data)
{
  Chat *chat = g_object_get_data(G_OBJECT(row), "chat-data");
  if (chat != NULL)
    load_main_page(chat);
}

/* */
Chat
*new_chat_row(const char *name,
	     ChatType type)
{
  const char *row_id = NULL;
  const char *label_id = NULL;
  switch (type) {
    case CHAT_TYPE_PUBLIC:
      row_id = "public_row";
      label_id = "public_row_label";
      break;
    case CHAT_TYPE_USER:
      row_id = "user_row";
      label_id = "user_row_label";
      break;
    case CHAT_TYPE_ROOM:
      row_id = "room_row";
      label_id = "room_row_label";
      break;
  }

  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/rows.ui");
  GtkWidget *row;
  row = GTK_WIDGET(gtk_builder_get_object(builder, row_id));
  GtkWidget *label;
  label = GTK_WIDGET(gtk_builder_get_object(builder, label_id));
  
  gtk_label_set_text(GTK_LABEL(label), name);
  Chat *chat = g_new0(Chat, 1);
  chat->name = g_strdup(name);
  chat->type = type;
  chat->messages = NULL;
  chat->row_widget = GTK_WIDGET(row);
  g_object_set_data(G_OBJECT(row), "chat-data", chat);

  //insert new chat
  gtk_list_box_append(GTK_LIST_BOX(chats_list), GTK_WIDGET(row));
  all_chats = g_list_append(all_chats, chat);
  g_object_unref(builder);
  return chat;
}

/* */
static void
enter_chat(GtkButton *button,
	   gpointer user_data)
{
  int port = get_port();
  const char* ip = get_ip();
  if (port < 1024 || port > 49151) {
    printf("Port number must be between 1024 and 49151.\n");
    return;
  }
  printf("- Port: [%d] \n- IP: [%s]\n", port, ip);
  
  GtkWindow *current_window;
  current_window = GTK_WINDOW(user_data);
  GtkApplication *app;
  app = GTK_APPLICATION(g_application_get_default());
  
  load_css("/org/chat/client/resources/css/chat.css");
  load_css("/org/chat/client/resources/css/rows.css");
  load_css("/org/chat/client/resources/css/headers.css");
  load_css("/org/chat/client/resources/css/main_page.css");
  
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  GtkWindow *chat_window;
  chat_window = GTK_WINDOW(gtk_builder_get_object(builder, "chat_window"));

  main_content = GTK_WIDGET(gtk_builder_get_object(builder, "main_content"));
  chats_list = GTK_WIDGET(gtk_builder_get_object(builder, "chats_list"));

  Chat *public_chat = new_chat_row("Public Chat", CHAT_TYPE_PUBLIC);

  ///testing
  Chat *user_chat = new_chat_row("Alice", CHAT_TYPE_USER);
  Chat *room_chat = new_chat_row("Room A", CHAT_TYPE_ROOM);
  ChatMessage *inf1 = create_message(MESSAGE_TYPE_INFO, "info", "Bienvenido al chat público");
  public_chat->messages = g_list_append(public_chat->messages, inf1);
  ChatMessage *m1 = create_message(MESSAGE_TYPE_NORMAL, "Alice", "¡Hola a todos!");
  public_chat->messages = g_list_append(public_chat->messages, m1);
  ChatMessage *m2 = create_message(MESSAGE_TYPE_NORMAL, "Bob", "¿Cómo están?");
  public_chat->messages = g_list_append(public_chat->messages, m2);
  ChatMessage *m3 = create_message(MESSAGE_TYPE_NORMAL, "Carmen", "¡Qué bueno ver tanta gente aquí!");
  public_chat->messages = g_list_append(public_chat->messages, m3);
  ChatMessage *m4 = create_message(MESSAGE_TYPE_NORMAL, "David", "¿Alguien ha visto las noticias de hoy?");
  public_chat->messages = g_list_append(public_chat->messages, m4);
  ChatMessage *inf2 = create_message(MESSAGE_TYPE_INFO, "info", "Te has unido a la sala: Room A");
  room_chat->messages = g_list_append(room_chat->messages, inf2);
  ChatMessage *r1 = create_message(MESSAGE_TYPE_NORMAL, "Marco", "Todo bien por aquí.");
  room_chat->messages = g_list_append(room_chat->messages, r1);
  ChatMessage *r2 = create_message(MESSAGE_TYPE_NORMAL, "Mafer", "¡Hola a todos los de Room A!");
  room_chat->messages = g_list_append(room_chat->messages, r2);
  ChatMessage *r3 = create_message(MESSAGE_TYPE_NORMAL, "Nora", "¿Qué proyectos están haciendo?");
  room_chat->messages = g_list_append(room_chat->messages, r3);
  ChatMessage *r4 = create_message(MESSAGE_TYPE_NORMAL, "Leo", "Yo estoy trabajando en una app de recetas :)");
  room_chat->messages = g_list_append(room_chat->messages, r4);
  ChatMessage *inf3 = create_message(MESSAGE_TYPE_INFO, "info", "Alice te ha enviado una solicitud de mensaje");
  user_chat->messages = g_list_append(user_chat->messages, inf3);
  ChatMessage *u1 = create_message(MESSAGE_TYPE_NORMAL, "Alice", "¡Hola! ¿Cómo estás?");
  user_chat->messages = g_list_append(user_chat->messages, u1);
  ChatMessage *u2 = create_message(MESSAGE_TYPE_NORMAL, "Tú", "¡Hola Alice! Bien, ¿y tú?");
  user_chat->messages = g_list_append(user_chat->messages, u2);
  ChatMessage *u3 = create_message(MESSAGE_TYPE_NORMAL, "Alice", "Estaba viendo si querías colaborar en un proyecto.");
  user_chat->messages = g_list_append(user_chat->messages, u3);
  ChatMessage *u4 = create_message(MESSAGE_TYPE_NORMAL, "Tú", "¡Claro! ¿Qué tienes en mente?");
  user_chat->messages = g_list_append(user_chat->messages, u4);
  ///
  
  load_main_page(public_chat);
    
  g_signal_connect(chats_list, "row-activated", G_CALLBACK(on_row_selected), NULL);
  
  gtk_window_set_application(chat_window, app);
  gtk_widget_set_visible(GTK_WIDGET(current_window), FALSE);
  gtk_window_destroy(current_window); 
  gtk_window_present(chat_window);
  g_object_unref(builder);
}

/* */
static void
activate(GtkApplication *app,
	 gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  current_port = data->port;

  load_css("/org/chat/client/resources/css/start.css");
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/start.ui");
  GtkWindow *window;
  window = GTK_WINDOW(gtk_builder_get_object(builder, "start_window"));
  gtk_window_set_application(window, app);

  //set given port and ip 
  port_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port_entry"));
  ip_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ip_entry"));
  g_signal_connect(port_entry, "changed", G_CALLBACK(on_port_changed), NULL);
  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%d", data->port);
  gtk_editable_set_text(GTK_EDITABLE(port_entry), port_str);
  gtk_editable_set_text(GTK_EDITABLE(ip_entry), data->server_ip);
  
  GtkWidget *btn_connect;
  btn_connect = GTK_WIDGET(gtk_builder_get_object(builder, "connect_button"));
  g_signal_connect(btn_connect, "clicked", G_CALLBACK(enter_chat), window);
  GtkWidget *btn_quit;
  btn_quit = GTK_WIDGET(gtk_builder_get_object(builder, "quit_button"));
  g_signal_connect(btn_quit, "clicked", G_CALLBACK(quit), NULL);
    
  gtk_window_present(window);
  g_object_unref(builder);
}

/* */
void
launch_gui(char* server_ip,
	   int port)
{
  StartData *data = g_new(StartData, 1);
  data->server_ip = g_strdup(server_ip);
  data->port = port;

  GtkApplication *app;
  app = gtk_application_new("org.chat.client", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect_data(app, "activate", G_CALLBACK (activate), data, (GClosureNotify)g_free, G_CONNECT_AFTER);
  g_application_run(G_APPLICATION (app), 0, NULL);
}
