/**
 * @file src/gui/main_gui.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-09
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#include "../include/main_gui.h"
#include "../include/style_ccs.h"

typedef struct {
  GtkWidget *toolbar;
  GtkWidget *statusbar;
  GtkWidget *packet_table;
  GtkWidget *packet_details;
  GtkWidget *packet_hexdump;
} window_context_t;

static window_context_t window_context;

static GtkCssProvider *main_gui_create_css_provider(void) {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider, stylesheet, -1, NULL);
  return provider;
}

static void main_gui_menu_create(GtkApplication *app, gpointer user_data) {
  GMenu *menu = g_menu_new();
  GMenu *help_menu = g_menu_new();
  g_menu_append(help_menu, "About", "app.about");
  g_menu_append_section(help_menu, "Help", G_MENU_MODEL(help_menu));

  gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu));
  g_object_unref(help_menu);

  about_dialog_create(app, user_data);
}

static void main_gui_layout_create(GtkApplication *app, gpointer user_data) {
  (void)app;
  GtkWidget *window = (GtkWidget *) user_data;
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  window_context.toolbar = toolbar_create(window, user_data);
  window_context.statusbar = statusbar_create(window, user_data);
  window_context.packet_table = packet_viewer_create();
  window_context.packet_details = packet_details_create();
  window_context.packet_hexdump = packet_hexdump_create();

  /* Header */
  gtk_box_pack_start(GTK_BOX(vbox), window_context.toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style(GTK_TOOLBAR(window_context.toolbar), GTK_TOOLBAR_ICONS);

  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(vbox), split_layout, TRUE, TRUE, 0);

  GtkWidget *container_table = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(container_table, (gint)(APPLICATION_MIN_WIDTH * 0.4), (gint)(APPLICATION_MIN_HEIGHT * 0.5));
  gtk_widget_set_name(container_table, "container_table");

  GtkWidget *container_details = gtk_scrolled_window_new(NULL, NULL);
  GtkWidget *vpaned_info = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(container_details), vpaned_info);

  gtk_paned_pack1(GTK_PANED(split_layout), container_table, FALSE, FALSE);
  gtk_paned_pack2(GTK_PANED(split_layout), container_details, TRUE, FALSE);

  GtkWidget *scroll_details = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroll_details), window_context.packet_details);
  gtk_widget_set_hexpand(scroll_details, FALSE);
  gtk_widget_set_vexpand(scroll_details, TRUE);

  gtk_container_add(GTK_CONTAINER(container_table), window_context.packet_table);
  gtk_paned_pack1(GTK_PANED(vpaned_info), scroll_details, TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(vpaned_info), window_context.packet_hexdump, TRUE, FALSE);
  gtk_box_pack_end(GTK_BOX(vbox), window_context.statusbar, FALSE, FALSE, 0);

  /* Footer */
  gtk_box_pack_end(GTK_BOX(vbox), window_context.statusbar, FALSE, FALSE, 0);
  gtk_widget_show_all(window);
}

static void main_gui_create_main_window(GtkApplication *app, gpointer user_data) {
  (void)user_data;
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen *screen = gdk_display_get_default_screen(display);
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), APPLICATION_MAIN_NAME);
  gtk_window_set_default_size(GTK_WINDOW(window), APPLICATION_MIN_WIDTH, APPLICATION_MIN_HEIGHT);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  GtkCssProvider *provider = main_gui_create_css_provider();
  gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  main_gui_menu_create(app, window);
  main_gui_layout_create(app, window);
  gtk_widget_show_all(window);
}

int gui_main(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.pwnsat.ghostlayer", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(main_gui_create_main_window), NULL);
  const int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
