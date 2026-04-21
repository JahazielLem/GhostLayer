/**
 * @file src/gui/iface_dialog.c
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
#include "main_gui.h"
#include "app_state.h"

void iface_dialog_create(GtkWidget *widget, gpointer data) {
  (void)widget;
  GtkWidget *window = (GtkWidget *) data;
  GtkWidget *dialog = gtk_dialog_new_with_buttons("iFace Configuration", GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "_Close",
                                                  GTK_RESPONSE_REJECT, "_Save", GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(dialog), (gint)(APPLICATION_MIN_WIDTH * 0.2), (gint)(APPLICATION_MIN_HEIGHT * 0.2));

  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  GtkWidget *port_label = gtk_label_new("Host Port:");
  gtk_grid_attach(GTK_GRID(grid), port_label, 1, 0, 1, 1);

  GtkWidget *port_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), port_entry, 3, 0, 1, 1);
  char port_str[24];
  snprintf(port_str, sizeof(port_str), "%d", BRIDGE_DEFAULT_PORT);
  gtk_entry_set_text(GTK_ENTRY(port_entry), port_str);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    const char *host_port = gtk_entry_get_text(GTK_ENTRY(port_entry));
    char *end;
    const long port = strtol(host_port, &end, 10);
    app_state_server_set_port((int)port);
  }

  gtk_widget_destroy(dialog);
}
