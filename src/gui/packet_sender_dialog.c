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

#include "main_gui.h"
#include "app_state.h"
#include "alerts.h"

typedef struct {
  GtkWidget *hex_editor;
  GtkWidget *output_editor;
  GtkTextBuffer *hex_buffer;
  GtkTextBuffer *output_buffer;
  GtkTextTag *hover_tag;
} hexdump_context_t;

static hexdump_context_t hexdump_ctx;
static GtkWidget *sender_instance = NULL;

static void on_plugin_state_modified(void);
static void packet_sender_on_send(void);

static void on_send_to_intruder(void) {
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(hexdump_ctx.hex_buffer, &start, &end);
  const char *text = gtk_text_buffer_get_text(hexdump_ctx.hex_buffer, &start, &end, FALSE);

  int payload_len = 0;
  uint8_t *payload_raw = ascii_to_uint8_buffer(text, &payload_len);

  space_packet_t *spp_packet = plugin_spp_build_packet(payload_raw, payload_len);
  const uint16_t spp_payload_len = HOST_TO_BE16(spp_packet->header.length) + 1;
  const uint16_t total_size = SPP_PRIMARY_HEADER_LEN + spp_payload_len;

  if (total_size > sizeof(space_packet_t)) {
    g_warning("Invalid packet size: %u", total_size);
    return;
  }
  proto_packet_t *packet = g_new0(proto_packet_t, 1);
  packet->length = total_size;
  memccpy(packet->buffer, (uint8_t*)spp_packet, 0, total_size);
  intruder_inspect_packet(packet);
  if (payload_raw) g_free(payload_raw);
}

static void on_sender_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
  GtkWidget *statusbar = GTK_WIDGET(user_data);
  guint context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "packet_events");

  if (response_id == GTK_RESPONSE_ACCEPT) {
    if (!app_state_server_get_state()) {
      alert_show_dialog(GTK_WINDOW(dialog), ALERT_ERROR, "Connection Error", "Python bridge is offline.");
      gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, "Error: Bridge offline");
    }else {
      packet_sender_on_send();
      gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, "Packet transmitted!");
    }
  }else {
    gtk_widget_destroy(GTK_WIDGET(dialog));
    sender_instance = NULL;
  }
}

static void packet_sender_gui_layout_left_panel(GtkWidget *split_layout) {
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 10);
  GtkWidget *plugin_box = plugin_spp_crafter_create();
  gtk_box_pack_start(GTK_BOX(left_vbox), plugin_box, FALSE, FALSE, 0);
  gtk_paned_pack1(GTK_PANED(split_layout), left_vbox, TRUE, FALSE);
  g_signal_connect(plugin_box, "spp-data-changed", G_CALLBACK(on_plugin_state_modified), NULL);
}

static void packet_sender_gui_layout_right_panel(GtkWidget *split_layout) {
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 10);
  plugin_radio_create(right_vbox);
  gtk_paned_pack2(GTK_PANED(split_layout), right_vbox, TRUE, FALSE);
}

static void packet_sender_gui_on_payload_change(GtkTextBuffer *buffer, gpointer user_data) {
  (void)user_data;
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  const char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  int payload_len = 0;
  uint8_t *payload_raw = ascii_to_uint8_buffer(text, &payload_len);

  space_packet_t *spp_packet = plugin_spp_build_packet(payload_raw, payload_len);

  const uint16_t spp_payload_len = HOST_TO_BE16(spp_packet->header.length);
  const uint16_t total_size = SPP_PRIMARY_HEADER_LEN + spp_payload_len;

  if (total_size > sizeof(space_packet_t) || total_size < SPP_PRIMARY_HEADER_LEN) {
    g_warning("Invalid packet size: %u", total_size);
    return;
  }

  GString *hex_dump = generate_hexdump((uint8_t*)spp_packet, total_size);
  gtk_text_buffer_set_text(hexdump_ctx.output_buffer, hex_dump->str, -1);

  g_string_free(hex_dump, TRUE);
  if (payload_raw) g_free(payload_raw);
  while (gtk_events_pending()) {
    gtk_main_iteration_do(FALSE);
  }
}

static void on_plugin_state_modified(void) {
  packet_sender_gui_on_payload_change(hexdump_ctx.hex_buffer, NULL);
}

static void packet_sender_on_send(void) {
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(hexdump_ctx.hex_buffer, &start, &end);
  const char *text = gtk_text_buffer_get_text(hexdump_ctx.hex_buffer, &start, &end, FALSE);

  int payload_len = 0;
  uint8_t *payload_raw = ascii_to_uint8_buffer(text, &payload_len);

  space_packet_t *spp_packet = plugin_spp_build_packet(payload_raw, payload_len);
  const uint16_t spp_payload_len = HOST_TO_BE16(spp_packet->header.length) + 1;
  const uint16_t total_size = SPP_PRIMARY_HEADER_LEN + spp_payload_len;

  if (total_size > sizeof(space_packet_t)) {
    g_warning("Invalid packet size: %u", total_size);
    return;
  }
  app_state_transmit_packet_with_config((uint8_t*)spp_packet, total_size);
  if (payload_raw) g_free(payload_raw);
}

void packet_sender_dialog_create(GtkWidget *widget, gpointer data) {
  (void)widget;
  GtkWidget *parent_window = (GtkWidget *) data;
  if (sender_instance != NULL) {
    gtk_window_present(GTK_WINDOW(sender_instance));
    return;
  }

  sender_instance = gtk_dialog_new_with_buttons("CCSDS Packet Sender", NULL,
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  "_Close", GTK_RESPONSE_REJECT,
                                                  "_Send", GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_modal(GTK_WINDOW(sender_instance), FALSE);
  gtk_window_set_position(GTK_WINDOW(sender_instance), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(sender_instance), (gint)(APPLICATION_MIN_WIDTH * 0.8), (gint)(APPLICATION_MIN_HEIGHT * 0.5));

  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(sender_instance));
  gtk_widget_set_margin_top(content_area, 10);
  gtk_widget_set_margin_bottom(content_area, 10);
  gtk_widget_set_margin_start(content_area, 15);
  gtk_widget_set_margin_end(content_area, 15);

  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(content_area), split_layout);

  packet_sender_gui_layout_left_panel(split_layout);
  packet_sender_gui_layout_right_panel(split_layout);

  gtk_container_add(GTK_CONTAINER(content_area), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  GtkWidget *split_layout_hex = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(content_area), split_layout_hex);;

  GtkWidget *payload_layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(payload_layout, 10);
  gtk_widget_set_margin_bottom(payload_layout, 10);
  gtk_widget_set_margin_start(payload_layout, 15);
  gtk_widget_set_margin_end(payload_layout, 15);
  gtk_paned_pack1(GTK_PANED(split_layout_hex), payload_layout, TRUE, FALSE);

  GtkWidget *label_moded = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_moded), "<b><span size='large'>Payload</span></b>");
  gtk_widget_set_halign(label_moded, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(payload_layout), label_moded, FALSE, FALSE, 0);

  GtkWidget *scroll_hex = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_hex), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scroll_hex, TRUE);
  gtk_widget_set_hexpand(scroll_hex, TRUE);

  hexdump_ctx.hex_editor = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hexdump_ctx.hex_editor), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hexdump_ctx.hex_editor), GTK_WRAP_WORD);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(hexdump_ctx.hex_editor), TRUE);
  gtk_container_add(GTK_CONTAINER(scroll_hex), hexdump_ctx.hex_editor);
  hexdump_ctx.hex_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexdump_ctx.hex_editor));

  gtk_box_pack_start(GTK_BOX(payload_layout), scroll_hex, TRUE, TRUE, 0);

  GtkWidget *output_layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(output_layout, 10);
  gtk_widget_set_margin_bottom(output_layout, 10);
  gtk_widget_set_margin_start(output_layout, 15);
  gtk_widget_set_margin_end(output_layout, 15);
  gtk_paned_pack2(GTK_PANED(split_layout_hex), output_layout, TRUE, FALSE);

  GtkWidget *label_output = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_output), "<b><span size='large'>Output Packet (Hex)</span></b>");
  gtk_widget_set_halign(label_output, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(output_layout), label_output, FALSE, FALSE, 0);

  GtkWidget *scroll_output_hex = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_output_hex), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scroll_output_hex, TRUE);
  gtk_widget_set_hexpand(scroll_output_hex, TRUE);

  hexdump_ctx.output_editor = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hexdump_ctx.output_editor), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hexdump_ctx.output_editor), GTK_WRAP_WORD);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(hexdump_ctx.output_editor), TRUE);
  gtk_container_add(GTK_CONTAINER(scroll_output_hex), hexdump_ctx.output_editor);
  hexdump_ctx.output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexdump_ctx.output_editor));

  gtk_box_pack_start(GTK_BOX(output_layout), scroll_output_hex, TRUE, TRUE, 0);

  g_signal_connect(hexdump_ctx.hex_buffer, "changed", G_CALLBACK(packet_sender_gui_on_payload_change), NULL);
  packet_sender_gui_on_payload_change(hexdump_ctx.hex_buffer, NULL);

  gtk_container_add(GTK_CONTAINER(content_area), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  GtkWidget *footer_layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_margin_top(footer_layout, 10);
  gtk_widget_set_margin_bottom(footer_layout, 10);
  gtk_widget_set_margin_start(footer_layout, 15);
  gtk_widget_set_margin_end(footer_layout, 15);
  gtk_container_add(GTK_CONTAINER(content_area), footer_layout);

  GtkWidget *statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(footer_layout), statusbar, FALSE, FALSE, 2);

  GtkWidget *btn_add = gtk_button_new_with_label("Send to Intruder");
  gtk_box_pack_end(GTK_BOX(footer_layout), btn_add, FALSE, FALSE, 2);

  g_signal_connect(sender_instance, "response", G_CALLBACK(on_sender_response), statusbar);
  g_signal_connect_swapped(parent_window, "destroy", G_CALLBACK(gtk_widget_destroy), sender_instance);
  g_signal_connect(btn_add, "clicked", G_CALLBACK(on_send_to_intruder), NULL);

  gtk_widget_show_all(sender_instance);
}
