/**
 * @file src/gui/about_dialog.c
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
#include "../include/version.h.in"

static void about_dialog_on_activate(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  (void)action;
  (void)parameter;
  (void)user_data;

  GtkWidget *about_dialog = gtk_about_dialog_new();

  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_MAIN_NAME);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), GHOSTLAYER_VERSION);
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_WEBSITE);
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), APPLICATION_COMMENTS);
  gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_GPL_3_0);
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog), NULL);

  gtk_widget_set_name(about_dialog, "about_dialog");

  gtk_dialog_run(GTK_DIALOG(about_dialog));
  gtk_widget_destroy(about_dialog);
}

void about_dialog_create(GtkApplication *app, gpointer user_data) {
  (void)user_data;
  GSimpleAction *about_action = g_simple_action_new("about", NULL);
  g_signal_connect(about_action, "activate", G_CALLBACK(about_dialog_on_activate), NULL);
  g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about_action));
}
