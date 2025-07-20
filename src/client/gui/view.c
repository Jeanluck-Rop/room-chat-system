#include "view.h"

static GtkPopover *actions_popover = NULL;

/**
 *
 * Auxiliar functions
 *
 **/

/*     Functions for handle gtk widgets and structs     */

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
static ChatData*
get_chat_data()
{
  GtkWindow *window = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
  return g_object_get_data(G_OBJECT(window), "chat-data");
}

/* */
static Chat*
get_chat(const char *name)
{
  ChatData *chatty = get_chat_data();
  for (GList *l = chatty->chats; l != NULL; l = l->next) {
    Chat *chat = l->data;
    if (g_strcmp0(chat->name, name) == 0) {
      return chat;
    }
  }
  return NULL; // Not found
}

/* */
static void
clear_widget(GtkWidget *widget)
{
  GtkWidget *child;
  GtkWidget *next;
  child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    next = gtk_widget_get_next_sibling(child);
    gtk_widget_unparent(child);
    child = next;
  }
}
///



/*     Functions for handle the user beginning connection     */

/* */
static GtkWidget
*build_message(ChatMessage *msg)
{
 
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/message.ui");
  GtkWidget *root;
  root = GTK_WIDGET(gtk_builder_get_object(builder, "message_root"));
  
  GtkWidget *msg_box;
  if (msg->type == INFO_MESSAGE) {
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

/* */
static void
set_actions_btn(GtkBuilder *builder)
{
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

/* */
static void
set_header(Chat *chat, GtkWidget *header)
{
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/headers.ui");
  GtkWidget *label;
  const char *header_id;
  
  switch (chat->type) {
  case USER_CHAT:
    header_id = "user_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "user_name_label"));
    if (label)
      gtk_label_set_text(GTK_LABEL(label), chat->name);
    break;
  case ROOM_CHAT:
    header_id = "room_header";
    label = GTK_WIDGET(gtk_builder_get_object(builder, "room_name_label"));
    if (label)
      gtk_label_set_text(GTK_LABEL(label), chat->name);
    break;
  default:
    header_id = "public_header"; //this cant happen
    break;
  }
  GtkWidget *custom_header;
  custom_header = GTK_WIDGET(gtk_builder_get_object(builder, header_id));
  
  gtk_list_box_append(GTK_LIST_BOX(header), GTK_WIDGET(custom_header));
  gtk_widget_set_visible(custom_header, TRUE);
  g_signal_connect(header, "row-activated", G_CALLBACK(on_header_activated), builder);
}
///


/*     Load the main page of the chat     */

/* */
static void
load_main_page(Chat *chat,
	       gpointer user_data)
{
  ChatData *chatty = (ChatData *)user_data;
    
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/main_page.ui");
  GtkWidget *row_page;
  row_page = GTK_WIDGET(gtk_builder_get_object(builder, "row_page"));
  GtkWidget *header_left;
  header_left = GTK_WIDGET(gtk_builder_get_object(builder, "header_left"));
  GtkWidget *header_right;
  header_right = GTK_WIDGET(gtk_builder_get_object(builder, "header_right"));
  GtkWidget *messages_box;
  messages_box = GTK_WIDGET(gtk_builder_get_object(builder, "messages_box"));
  GtkWidget *message_entry;
  message_entry = GTK_WIDGET(gtk_builder_get_object(builder, "message_entry"));
  GtkWidget *send_button;
  send_button = GTK_WIDGET(gtk_builder_get_object(builder, "send_button"));
  
  //clean previous content
  clear_widget(chatty->main_content);

  //load header
  set_header(chat, header_left);

  //load user actions button
  set_actions_btn(builder);
  
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

  gtk_box_append(GTK_BOX(chatty->main_content), row_page);
  gtk_widget_set_visible(chatty->main_content, TRUE);
  g_object_unref(builder);
}
///


/*     Functions for handle the chat rows and their messages    */

/* */
static void
on_row_selected(GtkListBox *box,
		GtkListBoxRow *row,
		gpointer user_data)
{
  Chat *chat = g_object_get_data(G_OBJECT(row), "chat-data");
  if (chat != NULL)
    load_main_page(chat, user_data);
}

/* */
void
add_new_message(MessageType type,
		const char* chat_name,
		const char* sender,
		const char* content)
{
  Chat *chat = get_chat(chat_name);
  
  ChatMessage *msg = g_new0(ChatMessage, 1);
  msg->type = type;
  msg->sender = g_strdup(sender);
  msg->content = g_strdup(content);
  //actualizamos el gui para que se muestre el

  chat->messages = g_list_append(chat->messages, msg);
}

/* */
void
new_chat_row(ChatType type,
	     const char* name,
	     const char* msg)
{
  ChatData *chatty = get_chat_data();

  const char* row_id;
  const char* label_id;
  const char* recent_id;

  switch (type) {
  case PUBLIC_CHAT:
    row_id = "public_row";
    label_id = "public_row_label";
    recent_id = "public_recent_label";
    break;
  case USER_CHAT:
    row_id = "user_row";
    label_id = "user_row_label";
    recent_id = "user_recent_label";
    break;
  case ROOM_CHAT:
    row_id = "room_row";
    label_id = "room_row_label";
    recent_id = "room_recent_label";
    break;
  default:
    return; //cant happen
  }

  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/rows.ui");
  GtkWidget *row;
  row = GTK_WIDGET(gtk_builder_get_object(builder, row_id));
  GtkWidget *label;
  label = GTK_WIDGET(gtk_builder_get_object(builder, label_id));
  GtkWidget *recent;
  recent = GTK_WIDGET(gtk_builder_get_object(builder, recent_id));
  
  gtk_label_set_text(GTK_LABEL(label), name);
  gtk_label_set_text(GTK_LABEL(recent), msg);

  Chat *chat = g_new0(Chat, 1);
  chat->name = g_strdup(name);
  chat->type = type;
  chat->messages = NULL;
  chat->row_widget = GTK_WIDGET(row);
  g_object_set_data(G_OBJECT(row), "chat-data", chat);
  
  //insert new chat
  gtk_list_box_append(GTK_LIST_BOX(chatty->chats_list), GTK_WIDGET(row));
  chatty->chats = g_list_append(chatty->chats, chat);
  
  MessageType msgtp = (type == USER_CHAT) ? NORMAL_MESSAGE : INFO_MESSAGE;
  const char *sender = (type == USER_CHAT) ? name : "info";
  
  add_new_message(msgtp, name, sender, msg);
   
  g_object_unref(builder);
}
///


/*     Functions for handle the notifications     */

/* */
void
add_notify(const char *msg)
{
  //get the chat window
  ChatData *chatty = get_chat_data();
  
  chatty->notifs->list = g_list_append(chatty->notifs->list, g_strdup(msg));
  gtk_widget_add_css_class(chatty->notifs->button, "has-notifications");
}

/* */
static void
remove_notify(const char *msg,
	      gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  GList *found = g_list_find_custom(notifs->list, msg, (GCompareFunc)g_strcmp0);
  if (found) {
    g_free(found->data);
    notifs->list = g_list_delete_link(notifs->list, found);
  }
  
  if (!notifs->list)
    gtk_widget_remove_css_class(notifs->button, "has-notifications");
}

/* */
static void
on_notify_clicked(GtkButton *button,
		  gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  const char *msg = gtk_button_get_label(button);
  remove_notify(msg, user_data);
  gtk_popover_popdown(notifs->popover);
}

/* */
static void
display_notifications(GtkButton *button,
		      gpointer user_data)
{
  Notifs *notifs = (Notifs *)user_data;
  clear_widget(notifs->box);   //clear previous childs
  for (GList *l = notifs->list; l; l = l->next) {
    GtkWidget *btn = gtk_button_new_with_label((char*)l->data);
    gtk_widget_add_css_class(btn, "notif-item");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_notify_clicked), notifs);
    gtk_box_append(GTK_BOX(notifs->box), btn);
  }
  
  gtk_popover_popup(notifs->popover);
}

/* */
static void
set_notifs(ChatData *chatty)
{
  Notifs *notifs = g_new(Notifs, 1);
  notifs->button = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_button"));
  notifs->popover = GTK_POPOVER(gtk_builder_get_object(chatty->builder, "notifs_popover"));
  notifs->box = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "notifs_box"));
  g_signal_connect(notifs->button, "clicked", G_CALLBACK(display_notifications), notifs);
  chatty->notifs = notifs;
}
///

/*static void fake_chat_info(Chat *public_chat) {
  Chat *user_chat = new_chat_row(USER_CHAT, "Alice");
  Chat *room_chat = new_chat_row(ROOM_CHAT, "Room A");
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
  
  add_notify("Nueva solicitud de mensaje");
  add_notify("Alicen se unió al chat");
  add_notify("Alice te invito al cuarto [Pumitas]");
  } */

/* Enter the chat */

/* */
static void
enter_chat()
{
  ChatData *chatty = g_new0(ChatData, 1); //allocate memory for the ChatData struct
  chatty->app = GTK_APPLICATION(g_application_get_default());

  //Load the css
  load_css("/org/chat/client/resources/css/chat.css");
  load_css("/org/chat/client/resources/css/rows.css");
  load_css("/org/chat/client/resources/css/headers.css");
  load_css("/org/chat/client/resources/css/main_page.css");
  load_css("/org/chat/client/resources/css/notifies.css");

  //Define the chat builder
  chatty->builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  
  //
  chatty->window = GTK_WINDOW(gtk_builder_get_object(chatty->builder, "chat_window"));
  chatty->chats_list = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "chats_list"));
  chatty->main_content = GTK_WIDGET(gtk_builder_get_object(chatty->builder, "main_content"));

  //
  gtk_window_set_application(chatty->window, chatty->app);
  g_object_set_data(G_OBJECT(chatty->window), "chat-data", chatty);
  
  //
  set_notifs(chatty);
  
  //
  new_chat_row(PUBLIC_CHAT, "Public Chat", "Welcome to the Public Chat!");
  Chat *public_chat = get_chat("Public Chat");
  
  //
  g_signal_connect(chatty->chats_list, "row-activated", G_CALLBACK(on_row_selected), chatty);
    
  //
  load_main_page(public_chat, chatty);

  //testing fake_chat_info(public_chat);

  //
  gtk_window_present(chatty->window);
  g_object_unref(chatty->builder);
}
///


/*     Functions for handle the user initial connection     */

/* */
static int
get_port(StartData *data)
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(data->port_entry));
  return atoi(port_text);
}

/* */
static const char*
get_ip(StartData *data)
{
  return gtk_editable_get_text(GTK_EDITABLE(data->ip_entry));
}

/* */
static void
on_port_changed(GtkEditable *editable,
	        gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  const char* text = gtk_editable_get_text(editable);
  data->port = atoi(text);
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
validate_data(GtkButton *button,
	      gpointer user_data)
{
  StartData *data = (StartData *)user_data;
  int port = get_port(data);
  const char* ip = get_ip(data);
  
  if (port < 1024 || port > 49151) {
    printf("Port number must be between 1024 and 49151.\n");
    return;
  }
  printf("- Port: [%d] \n- IP: [%s]\n", port, ip);
  
  GtkWindow *current_window = data->window;
  gtk_widget_set_visible(GTK_WIDGET(current_window), FALSE);
  gtk_window_destroy(current_window);
  g_free(user_data); //free de StartData struct, we no longer need it
  enter_chat();
}

/* */
static void
activate(GtkApplication *app,
	 gpointer user_data)
{
  //parse the gpointer to the StartData struct
  StartData *data = (StartData *)user_data;
  load_css("/org/chat/client/resources/css/start.css");
  
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/start.ui");
  GtkWindow *window;
  window = GTK_WINDOW(gtk_builder_get_object(builder, "start_window"));
  gtk_window_set_application(window, app);

  //reference the initial widgets
  data->window = window;
  data->port_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port_entry"));
  data->ip_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ip_entry"));
  g_signal_connect(data->port_entry, "changed", G_CALLBACK(on_port_changed), data);

  //set given port and ip 
  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%d", data->port);
  gtk_editable_set_text(GTK_EDITABLE(data->port_entry), port_str);
  gtk_editable_set_text(GTK_EDITABLE(data->ip_entry), data->server_ip);

  //conect to the server
  GtkWidget *btn_connect;
  btn_connect = GTK_WIDGET(gtk_builder_get_object(builder, "connect_button"));
  g_signal_connect(btn_connect, "clicked", G_CALLBACK(validate_data), data);

  //cancel and quit the program
  GtkWidget *btn_quit;
  btn_quit = GTK_WIDGET(gtk_builder_get_object(builder, "quit_button"));
  g_signal_connect(btn_quit, "clicked", G_CALLBACK(quit), NULL);

  //show the window
  gtk_window_present(window);
  g_object_unref(builder);
}

/* */
void
launch_gui(int port,
	   char* server_ip)
{
  //allocate memory to the user initial data struct
  StartData *data = g_new0(StartData, 1);
  data->server_ip = g_strdup(server_ip);
  data->port = port;

  //Create the gtk app and connect it with the active signal
  GtkApplication *app;
  app = gtk_application_new("org.chat.client", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect_data(app, "activate", G_CALLBACK (activate), data, (GClosureNotify)g_free, G_CONNECT_AFTER);
  //Run the gtk application
  g_application_run(G_APPLICATION (app), 0, NULL);
}
