/* src/ui/components - main-layout.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "main-layout.h"
#include "toolbar.h"
#include "table.h"

typedef struct {
  GtkWidget *toolbar;
  GtkWidget *table;
} main_layout_context_t;

static main_layout_context_t main_layout_context;

static void main_layout_about_on_activate(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  GtkWidget *about_dialog = gtk_about_dialog_new();

  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_MAIN_NAME);
  // gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_VERSION);
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_WEBSITE);
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_COMMENTS);
  gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_GPL_3_0);

  gtk_dialog_run(GTK_DIALOG(about_dialog));
  gtk_widget_destroy(about_dialog);
}

static void main_layout_menu(GtkApplication *app, gpointer user_data) {
  GMenu *menu = g_menu_new();
  GMenu *help_menu = g_menu_new();

  g_menu_append(help_menu, "About", "app.about");
  g_menu_append_section(help_menu, "Help", G_MENU_MODEL(help_menu));

  gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu));
  g_object_unref(help_menu);

  GSimpleAction *about_action = g_simple_action_new("about", NULL);
  g_signal_connect(about_action, "activate", G_CALLBACK(main_layout_about_on_activate), NULL);
  g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about_action));
}

static GtkWidget *main_layout_create_table_container() {
  GtkWidget *container = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(container, (DEFAULT_WIDTH * 0.4), (DEFAULT_HEIGHT * 0.5));
  return container;
}

static GtkWidget *main_layout_create_details_container() {
  GtkWidget *container = gtk_scrolled_window_new(NULL, NULL);
  GtkWidget *vpaned_info = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(container), vpaned_info);
  return container;
}

static void main_layout_create_containers(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = user_data;
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  main_layout_context.toolbar = main_layout_toolbar_init(window, user_data);
  gtk_box_pack_start(GTK_BOX(vbox), main_layout_context.toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style(GTK_TOOLBAR(main_layout_context.toolbar), GTK_TOOLBAR_ICONS);

  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(vbox), split_layout, TRUE, TRUE, 0);

  GtkWidget *container_table = main_layout_create_table_container();
  GtkWidget *container_details = main_layout_create_details_container();

  gtk_paned_pack1(GTK_PANED(split_layout), container_table, FALSE, FALSE);
  gtk_paned_pack2(GTK_PANED(split_layout), container_details, TRUE, FALSE);

  main_layout_context.table = main_layout_table_init();

  GtkWidget *scroll_details = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(container_table), main_layout_context.table);
}

void main_layout_init(GtkApplication *app, gpointer user_data) {
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen *screen = gdk_display_get_default_screen(display);
  GtkWidget *base_window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(base_window), APPLICATION_MAIN_NAME);
  gtk_window_set_default_size(GTK_WINDOW(base_window), DEFAULT_WIDTH, DEFAULT_HEIGHT);
  gtk_window_set_position(GTK_WINDOW(base_window), GTK_WIN_POS_CENTER);

  main_layout_menu(app, base_window);
  main_layout_create_containers(app, base_window);

  gtk_widget_show_all(base_window);
}