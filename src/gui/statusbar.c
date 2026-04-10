/**
 * @file src/statusbar.c
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

#include "../../include/main_gui.h"

typedef struct {
  GtkWidget *label_connection;
  GtkWidget *label_packets;
} statusbar_context_t;

static statusbar_context_t statusbar_context;

void statusbar_update_label_connection(const gboolean state) {
  char buffer[64];
  sprintf(buffer, "Connection: %s", state ? "Connected" : "Disconnected");
  gtk_label_set_text(GTK_LABEL(statusbar_context.label_connection), buffer);
}

void statusbar_update_label_packet_count(const uint16_t packet_count) {
  char buffer[64];
  sprintf(buffer, "Packets: %d", packet_count);
  gtk_label_set_text(GTK_LABEL(statusbar_context.label_packets), buffer);
}

GtkWidget *statusbar_create(GtkWidget *widget, gpointer data) {
  (void)widget;
  GtkWidget *window = (GtkWidget *) data;

  GtkWidget *statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_margin_top(statusbar, 4);
  gtk_widget_set_margin_bottom(statusbar, 4);
  gtk_widget_set_margin_start(statusbar, 8);
  gtk_widget_set_margin_end(statusbar, 8);

  statusbar_context.label_connection = gtk_label_new("Connection: None");
  statusbar_context.label_packets = gtk_label_new("Packets: 0");

  g_object_set_data(G_OBJECT(window), "label_connection", statusbar_context.label_connection);
  g_object_set_data(G_OBJECT(window), "label_packets", statusbar_context.label_packets);

  gtk_widget_set_name(statusbar, "statusbar");

  gtk_box_pack_start(GTK_BOX(statusbar), statusbar_context.label_connection, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(statusbar), statusbar_context.label_packets, TRUE, TRUE, 5);

  return statusbar;
}
