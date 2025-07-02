#include "view.h"

static void activate(GtkApplication *app)
{
  GtkWidget *window = gtk_application_window_new (app);
  GtkWidget *label = gtk_label_new ("Hello World");
  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
  gtk_window_set_child (GTK_WINDOW (window), label);
  gtk_window_present (GTK_WINDOW (window));
}


void launch_gui(char* server_ip, int port)
{
  g_autoptr(AdwApplication) app = NULL;
  app = adw_application_new("org.chat.client", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
  g_application_run(G_APPLICATION (app), port, server_ip);
}
