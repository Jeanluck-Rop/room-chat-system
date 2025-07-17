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

static GtkWidget *notifs_button = NULL;
static GtkPopover *notifs_popover = NULL;
static GtkWidget *notifs_box = NULL;
static GList *notifications = NULL;

static GtkPopover *actions_popover = NULL;

/**
 *
 * Auxiliar functions
 *
 **/

/*     Functions for handle gtk widgets     */
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
///


/*     Functions for handle the user beginning connection     */

/* */
static int
get_port()
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(port_entry));
  return atoi(port_text);
}

/* */
static const char
*get_ip()
{
  return gtk_editable_get_text(GTK_EDITABLE(ip_entry));
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
quit(GtkButton *button,
     gpointer user_data)
{
  g_application_quit(g_application_get_default());
}
///


/*     Functions for handle the user beginning connection     */

/* */
static ChatMessage
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
static GtkWidget
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
///


/*     Functions for handle the notifications     */

/* */
void
add_notify(const char *msg)
{
  notifications = g_list_append(notifications, g_strdup(msg));
  gtk_widget_add_css_class(notifs_button, "has-notifications");
}

/* */
static void
remove_notify(const char *msg)
{
  GList *found = g_list_find_custom(notifications, msg, (GCompareFunc)g_strcmp0);
  if (found) {
    g_free(found->data);
    notifications = g_list_delete_link(notifications, found);
  }
  
  if (!notifications)
    gtk_widget_remove_css_class(notifs_button, "has-notifications");
}

/* */
static void
on_notify_clicked(GtkButton *button,
		  gpointer user_data)
{
  const char *msg = gtk_button_get_label(button);
  remove_notify(msg);
  gtk_popover_popdown(notifs_popover);
}

/* */
static void
display_notifications(GtkButton *button,
		      gpointer user_data)
{
  clear_widget(notifs_box);   //clear previous childs
  for (GList *l = notifications; l; l = l->next) {
    GtkWidget *btn = gtk_button_new_with_label((char*)l->data);
    gtk_widget_add_css_class(btn, "notif-item");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_notify_clicked), NULL);
    gtk_box_append(GTK_BOX(notifs_box), btn);
  }
  
  gtk_popover_popup(notifs_popover);
}
///


/*     Functions for handle the chat rows    */

/* */
Chat
*new_chat_row(ChatType type,
	      const char *name)
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
///


/*     Functions for handle the each chat header information     */

/* */
static gboolean
on_dialog_close(GtkWindow *window,
		gpointer user_data)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  return TRUE;
}

/* */
static void
on_send_button_clicked(GtkButton *button,
                       gpointer user_data)
{
  const char *username = user_data;
  g_print("Send message to: %s\n", username);

  GtkWidget *widget = GTK_WIDGET(button);
  GtkWidget *ancestor = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
  if (ancestor)
    gtk_widget_set_visible(ancestor, FALSE);

  g_free(user_data);
}

/* */
static void
populate_user_list(GtkBuilder *builder,
		   GtkListBox *list_box,
                   char **usernames,
                   GCallback send_callback)
{
  gtk_list_box_remove_all(list_box);
  
  for (int i = 0; usernames[i] != NULL; i++) {
    GtkBuilder *row_builder;
    row_builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
    GtkWidget *template_row;
    template_row = GTK_WIDGET(gtk_builder_get_object(row_builder, "user_list_item"));
    
    if (!template_row)
      continue;
    
    gtk_widget_set_visible(template_row, TRUE);
    GtkWidget *box;
    box = gtk_widget_get_first_child(template_row);
    GtkWidget *username_label;
    username_label = gtk_widget_get_first_child(box);
    GtkWidget *send_button;
    send_button = gtk_widget_get_next_sibling(username_label);
    
    gtk_label_set_text(GTK_LABEL(username_label), usernames[i]);
    g_signal_connect(send_button, "clicked", send_callback, g_strdup(usernames[i]));
    gtk_list_box_append(list_box, template_row);
    g_object_unref(row_builder);
  }
  
  gtk_widget_set_visible(GTK_WIDGET(list_box), TRUE);
}

/* */
void free_user_list(char **users) {
  if (!users)
    return;
  
  for (int i = 0; users[i]; i++) {
    if (users[i])
      g_free(users[i]);
  }
  
  g_free(users);
}

/* */
static void
on_header_activated(GtkListBox *box,
                    GtkListBoxRow *row,
                    gpointer user_data)
{
  GtkBuilder *builder = GTK_BUILDER(user_data);
  const char *name = gtk_widget_get_name(GTK_WIDGET(row));
  const char *window_id;
  GtkListBox *list_box;
  char **users;
  GCallback send_callback = G_CALLBACK(on_send_button_clicked);

  static char *room_fake_users[] = { "alice", "charlie", "diana", NULL };
  static char *public_fake_users[] = { "alice", "bob", "charlie", "diana", "eve", NULL };

  if (g_strcmp0(name, "user_header") == 0)
    window_id = "user_info_window";
  else if (g_strcmp0(name, "room_header") == 0) {
    window_id = "room_info_window";
    list_box = GTK_LIST_BOX(gtk_builder_get_object(builder, "room_users_list"));
    GtkWidget *room_label;
    room_label = GTK_WIDGET(gtk_builder_get_object(builder, "room_name_label"));
    const char *room_name = gtk_label_get_text(GTK_LABEL(room_label));
    users = room_fake_users;
    //users = get_room_users(room_name); 
  } else if (g_strcmp0(name, "public_header") == 0) {
    window_id = "public_info_window";
    list_box = GTK_LIST_BOX(gtk_builder_get_object(builder, "public_users_list"));
    users = public_fake_users;
    //users = get_public_users();
  }
  
  GtkWindow *info_window = GTK_WINDOW(gtk_builder_get_object(builder, window_id));
  if (!info_window)
    return;
  
  if (list_box && users) {
    populate_user_list(builder, list_box, users, send_callback);
    ///free_user_list(users);
  }

  gtk_window_set_modal(info_window, TRUE);
  gtk_window_set_transient_for(info_window, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(box))));
  g_signal_connect(info_window, "close-request", G_CALLBACK(on_dialog_close), NULL);
  gtk_window_present(info_window);
}
///


/*     Functions for handle the actions button and delegate each chat action     */

/* */
static void
change_status_clicked(GtkButton *button,
		      gpointer user_data)
{
  printf("Change status clicked\n");
  gtk_popover_popdown(actions_popover);
}

/* */
static void
new_room_clicked(GtkButton *button,
		 gpointer user_data)
{
  printf("New room clicked\n");
  gtk_popover_popdown(actions_popover);
}

/* */
static void
invite_users_clicked(GtkButton *button,
		     gpointer user_data)
{
  printf("Invite users clicked\n");
  gtk_popover_popdown(actions_popover);
}

/* */
static void
leave_room_clicked(GtkButton *button,
		   gpointer user_data)
{
  printf("Leave room clicked\n");
  gtk_popover_popdown(actions_popover);
}

/* */
static void
disconnect_user_clicked(GtkButton *button,
			gpointer user_data)
{
  printf("Disconnect user clicked\n");
  gtk_popover_popdown(actions_popover);
  g_application_quit(g_application_get_default());
}

/* */
static void
display_user_actions(GtkButton *button,
		     gpointer user_data)
{
  GtkPopover *popover = GTK_POPOVER(user_data);
  gtk_popover_popup(popover);
}
///


/**
 *
 * Main functions to handle the gui
 *
 **/

/* */
static void
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
  GtkBuilder *header_builder;
  header_builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
  GtkWidget *label;
  const char *header_id;
  switch (chat->type) {
  case CHAT_TYPE_USER:
    header_id = "user_header";
    label = GTK_WIDGET(gtk_builder_get_object(header_builder, "user_name_label"));
    if (label)
      gtk_label_set_text(GTK_LABEL(label), chat->name);
    break;
  case CHAT_TYPE_ROOM:
    header_id = "room_header";
    label = GTK_WIDGET(gtk_builder_get_object(header_builder, "room_name_label"));
    if (label)
      gtk_label_set_text(GTK_LABEL(label), chat->name);
    break;
  default:
    header_id = "public_header"; //this cant happen
    break;
  }
  GtkWidget *custom_header;
  custom_header = GTK_WIDGET(gtk_builder_get_object(header_builder, header_id));
  if (!header_builder)
    return;
  gtk_list_box_append(GTK_LIST_BOX(header_left), GTK_WIDGET(custom_header));
  gtk_widget_set_visible(custom_header, TRUE);
  g_signal_connect(header_left, "row-activated", G_CALLBACK(on_header_activated), header_builder);

  //user actions button
  GtkWidget *header_right;
  header_right = GTK_WIDGET(gtk_builder_get_object(builder, "header_right"));
  GtkWidget *actions_button;
  actions_button = GTK_WIDGET(gtk_builder_get_object(builder, "info_button"));
  actions_popover = GTK_POPOVER(gtk_builder_get_object(builder, "chat_actions_popover"));
  g_signal_connect(actions_button, "clicked", G_CALLBACK(display_user_actions), actions_popover);

  GtkWidget *disconnect_button;
  disconnect_button = GTK_WIDGET(gtk_builder_get_object(builder, "disconnect_button"));
  g_signal_connect(disconnect_button, "clicked", G_CALLBACK(disconnect_user_clicked), NULL);
  
  GtkWidget *change_status_btn = GTK_WIDGET(gtk_builder_get_object(builder, "change_status"));
  g_signal_connect(change_status_btn, "clicked", G_CALLBACK(change_status_clicked), NULL);
  GtkWidget *new_room_btn = GTK_WIDGET(gtk_builder_get_object(builder, "new_room"));
  g_signal_connect(new_room_btn, "clicked", G_CALLBACK(new_room_clicked), NULL);
  GtkWidget *invite_users_btn = GTK_WIDGET(gtk_builder_get_object(builder, "invite_users"));
  g_signal_connect(invite_users_btn, "clicked", G_CALLBACK(invite_users_clicked), NULL);
  GtkWidget *leave_room_btn = GTK_WIDGET(gtk_builder_get_object(builder, "leave_room"));
  g_signal_connect(leave_room_btn, "clicked", G_CALLBACK(leave_room_clicked), NULL);
  GtkWidget *disconnect_user_btn = GTK_WIDGET(gtk_builder_get_object(builder, "disconnect_user"));
  g_signal_connect(disconnect_user_btn, "clicked", G_CALLBACK(disconnect_user_clicked), NULL);
  
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
///



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
  load_css("/org/chat/client/resources/css/notifies.css");
  
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  GtkWindow *chat_window;
  chat_window = GTK_WINDOW(gtk_builder_get_object(builder, "chat_window"));

  main_content = GTK_WIDGET(gtk_builder_get_object(builder, "main_content"));
  chats_list = GTK_WIDGET(gtk_builder_get_object(builder, "chats_list"));

  Chat *public_chat = new_chat_row(CHAT_TYPE_PUBLIC, "Public Chat");

  ///testing
  Chat *user_chat = new_chat_row(CHAT_TYPE_USER, "Alice");
  Chat *room_chat = new_chat_row(CHAT_TYPE_ROOM, "Room A");
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

  notifs_button = GTK_WIDGET(gtk_builder_get_object(builder, "notifs_button"));
  notifs_popover = GTK_POPOVER(gtk_builder_get_object(builder, "notifs_popover"));
  notifs_box = GTK_WIDGET(gtk_builder_get_object(builder, "notifs_box"));
  g_signal_connect(notifs_button, "clicked", G_CALLBACK(display_notifications), NULL);

  //testing
  add_notify("Nueva solicitud de mensaje");
  add_notify("Alicen se unió al chat");
  add_notify("Alice te invito al cuarto [Pumitas]");
  ///
  
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
