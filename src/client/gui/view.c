#include "view.h"

/**
 * Class constants and variables.
 **/
static GtkWidget *port_entry = NULL;
static GtkWidget *ip_entry = NULL;
static int current_port = 0;

/**
 * Auxiliar functions.
 **/

/* */
int
get_port()
{
  const char* port_text = gtk_editable_get_text(GTK_EDITABLE(port_entry));
  return atoi(port_text);
}

/* */
const char*
get_ip()
{
  return gtk_editable_get_text(GTK_EDITABLE(ip_entry));
}


/**
 * Main functions to handle the gui
 **/


/* */
static void
enter_chat(GtkButton *button,
	   gpointer user_data)
{
  GtkWindow *current_window = GTK_WINDOW(user_data);
  GtkApplication *app = GTK_APPLICATION(g_application_get_default());
  
  int port = get_port();
  const char* ip = get_ip();
  if (port < 1024 || port > 49151) {
    printf("Port number must be between 1024 and 49151.\n");
    return;
  }
  printf("- Port: [%d] \n- IP: [%s]\n", port, ip);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/org/chat/client/resources/css/chat.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  GtkWindow *chat_window;
  chat_window = GTK_WINDOW(gtk_builder_get_object(builder, "chat_window"));
  gtk_window_set_application(chat_window, app);
  gtk_widget_set_visible(GTK_WIDGET(current_window), FALSE);
  //gtk_window_maximize(chat_window);
  gtk_window_destroy(current_window);
  
  gtk_window_present(chat_window);
  g_object_unref(builder);
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
activate(GtkApplication *app,
	 gpointer user_data)
{
  //g_resources_register(resources_get_resource());
  StartData *data = (StartData *)user_data;
  current_port = data->port;
  
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/org/chat/client/resources/css/start.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/start.ui");
  GtkWindow *window;
  window = GTK_WINDOW(gtk_builder_get_object(builder, "start_window"));
  gtk_window_set_application(window, app);

  port_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port_entry"));
  ip_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ip_entry"));
  g_signal_connect(port_entry, "changed", G_CALLBACK(on_port_changed), NULL);
  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%d", data->port);
  gtk_editable_set_text(GTK_EDITABLE(ip_entry), data->server_ip);
  gtk_editable_set_text(GTK_EDITABLE(port_entry), port_str);
  
  GtkWidget *btn_connect;
  btn_connect = GTK_WIDGET(gtk_builder_get_object(builder, "connect_button"));
  GtkWidget *btn_quit;
  btn_quit = GTK_WIDGET(gtk_builder_get_object(builder, "quit_button"));
  g_signal_connect(btn_connect, "clicked", G_CALLBACK(enter_chat), window);
  g_signal_connect(btn_quit, "clicked", G_CALLBACK(quit), NULL);
  
  gtk_window_present(window);
  g_object_unref(builder);
}

//launch_gui()
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
