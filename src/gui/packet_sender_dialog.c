/**
 * @file src/gui/packet_sender_dialog.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-11
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */

#include "../include/main_gui.h"
#include "../include/app_state.h"

static void packet_sender_gui_layout_left_panel(GtkWidget *split_layout) {
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 10);
  gtk_box_pack_start(GTK_BOX(left_vbox), plugin_spp_crafter_create(), FALSE, FALSE, 0);
  gtk_paned_pack1(GTK_PANED(split_layout), left_vbox, TRUE, FALSE);
}

static void packet_sender_gui_layout_right_panel(GtkWidget *split_layout) {
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 10);
  plugin_radio_create(right_vbox);
  gtk_paned_pack2(GTK_PANED(split_layout), right_vbox, TRUE, FALSE);
}

void packet_sender_dialog_create(GtkWidget *widget, gpointer data) {
  (void)widget;
  GtkWidget *window = (GtkWidget *) data;
  GtkWidget *dialog = gtk_dialog_new_with_buttons("CCSDS Packet Sender", GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "_Close",
                                                  GTK_RESPONSE_REJECT, "_Send", GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(dialog), (gint)(APPLICATION_MIN_WIDTH * 0.5), (gint)(APPLICATION_MIN_HEIGHT * 0.5));

  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(content_area), split_layout);

  packet_sender_gui_layout_left_panel(split_layout);
  packet_sender_gui_layout_right_panel(split_layout);

  gtk_container_add(GTK_CONTAINER(content_area), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  GtkWidget *payload_layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(content_area), payload_layout);

  GtkWidget *label_moded = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_moded), "<b><span size='large'>Packet Base (Hex)</span></b>");
  gtk_widget_set_halign(label_moded, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(payload_layout), label_moded, FALSE, FALSE, 0);

  GtkWidget *scroll_hex = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_hex), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scroll_hex, TRUE);
  gtk_widget_set_hexpand(scroll_hex, TRUE);

  GtkWidget *hex_editor = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hex_editor), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hex_editor), GTK_WRAP_WORD);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(hex_editor), TRUE);
  gtk_container_add(GTK_CONTAINER(scroll_hex), hex_editor);

  gtk_box_pack_start(GTK_BOX(payload_layout), scroll_hex, TRUE, TRUE, 0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
  }

  gtk_widget_destroy(dialog);
}
