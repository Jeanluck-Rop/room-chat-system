#include "view.h"

extern GResource *resources_get_resource(void);

static const char *status[] = { "Active", "Away", "Busy" };

/* */
static void
display_chats(GtkWidget *listbox)
{
  GtkBuilder *row_builder = gtk_builder_new_from_resource("/org/chat/client/resources/rows.ui");

  for (int i = 0; i < 18; ++i) {
    const char *template_id;

    // Fake data for the moment
    if (i % 3 == 0)
      template_id = "row_type_1";
    else if (i % 3 == 1)
      template_id = "row_type_2";
    else
      template_id = "row_type_3";

    GtkWidget *row = GTK_WIDGET(gtk_builder_get_object(row_builder, template_id));
    if (!row)
      continue;

    GtkWidget *row_clone = g_object_new(G_OBJECT_TYPE(row), NULL);

    if (i % 3 == 0)
      g_object_set(row_clone, "title", "PUBLIC CHAT", NULL);
    else if (i % 3 == 1) {
      gchar *user = g_strdup_printf("user_%d", i);
      gchar *stat = g_strdup_printf("STATUS: %s", status[i % 3]);
      g_object_set(row_clone, "title", user, "subtitle", stat, NULL);
      g_free(user);
      g_free(stat);
    } else {
      gchar *room = g_strdup_printf("ROOM CHAT: Room_%d", i);
      g_object_set(row_clone, "title", room, NULL);
      g_free(room);
    }

    gtk_list_box_append(GTK_LIST_BOX(listbox), row_clone);
  }

  g_object_unref(row_builder);
}

/* */
static void
enter_chat(GtkButton *button,
	      gpointer user_data)
{
  GtkWidget *toolbar_view = GTK_WIDGET(user_data);

  GtkBuilder *chat_builder = gtk_builder_new_from_resource("/org/chat/client/resources/chat.ui");
  GtkWidget *chat = GTK_WIDGET(gtk_builder_get_object(chat_builder, "chat"));

  adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), chat);

  GtkWidget *chats_list;
  chats_list = GTK_WIDGET(gtk_builder_get_object(chat_builder, "chats_list"));
  display_chats(chats_list);

  g_object_unref(chat_builder);
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
enter_server(GtkWidget *toolbar_view)
{
  GtkBuilder *home_builder;
  home_builder = gtk_builder_new_from_resource("/org/chat/client/resources/home.ui");
  GtkWidget *home;
  home = GTK_WIDGET(gtk_builder_get_object(home_builder, "home"));
  GtkWidget *btn_connect;
  btn_connect = GTK_WIDGET(gtk_builder_get_object(home_builder, "connect"));
  GtkWidget *btn_quit;
  btn_quit = GTK_WIDGET(gtk_builder_get_object(home_builder, "quit"));
  
  g_signal_connect(btn_connect, "clicked", G_CALLBACK(enter_chat), toolbar_view);
  g_signal_connect(btn_quit, "clicked", G_CALLBACK(quit), NULL);
  adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), home);
  g_object_unref(home_builder);
}

/* */
static void
activate(GtkApplication *app)
{
  g_resources_register(resources_get_resource());
  
  GtkBuilder *builder;
  builder = gtk_builder_new_from_resource("/org/chat/client/resources/window.ui");
  GtkWindow *window;
  window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
  gtk_window_set_application(window, app);
  GtkWidget *toolbar_view;
  toolbar_view = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar_view"));
  
  enter_server(toolbar_view);
  gtk_window_present(window);
  g_object_unref(builder);
}

/* void launch_gui(char* server_ip, int port) */
void
launch_gui()
{
  g_autoptr(AdwApplication) app = NULL;
  app = adw_application_new("org.chat.client", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
  g_application_run(G_APPLICATION (app), 0, NULL);
}
