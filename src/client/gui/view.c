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
  load_main_page(public_chat);
  //testing
  new_chat_row("Room A", CHAT_TYPE_ROOM);
  new_chat_row("Alice", CHAT_TYPE_USER);

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
