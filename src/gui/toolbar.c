/**
 * @file src/gui/toolbar.c
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
#include "pcap_loader.h"

typedef struct {
  GtkToolItem *iface;
  GtkToolItem *handle_connection;
  GtkToolItem *intruder;
  GtkToolItem *packet_sender;
  GtkToolItem *export_file;
  GtkToolItem *load_pcap;
  GtkToolItem *save_pcap;
  GtkToolItem *builder;
} toolbar_context_t;

static toolbar_context_t toolbar_context;

static void toolbar_on_iface_handler(void) {
  const gboolean state = !app_state_server_get_state();
  const char *icon = state ? "media-playback-stop" : "media-playback-start";

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.handle_connection), icon);
  statusbar_update_label_connection(state);
  app_state_server_set_state(state);
  if (state) {
    app_state_server_init(app_state_server_get_port());
  }else {
    app_state_server_cleanup();
  }
}

static void toolbar_on_iface(GtkWidget *widget, gpointer user_data) {
  iface_dialog_create(widget, user_data);
}

static void toolbar_on_packet_sender(GtkWidget *widget, gpointer user_data) {
  packet_sender_dialog_create(widget, user_data);
}

static void toolbar_on_intruder(void) {
  intruder_gui_create();
}

static void toolbar_on_load_pcap(GtkWidget *widget, gpointer user_data) {
  pcap_reader_open_dialog(widget, user_data);
}

static void toolbar_on_save_pcap(GtkWidget *widget, gpointer user_data) {
  pcap_reader_save_dialog(widget, user_data);
}

static void toolbar_on_builder(GtkWidget *widget, gpointer user_data) {
  (void)widget;
  generator_gui_create(user_data);
}


GtkWidget *toolbar_create(GtkWidget *widget) {
  GtkWidget *window = (GtkWidget *) widget;
  GtkWidget *toolbar = gtk_toolbar_new();

  toolbar_context.iface = gtk_tool_button_new(NULL, "iFace");
  toolbar_context.handle_connection = gtk_tool_button_new(NULL, "Connection Handler");
  toolbar_context.load_pcap = gtk_tool_button_new(NULL, "Load PCAP");
  toolbar_context.save_pcap = gtk_tool_button_new(NULL, "Save PCAP");
  toolbar_context.intruder = gtk_tool_button_new(NULL, "Intruder");
  toolbar_context.builder = gtk_tool_button_new(NULL, "Advance Builder");
  toolbar_context.packet_sender = gtk_tool_button_new(NULL, "Packet Sender");

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.iface), "network-transmit-symbolic");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.iface), "Interface Configuration");
  g_signal_connect(toolbar_context.iface, "clicked", G_CALLBACK(toolbar_on_iface), window);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.handle_connection), "media-playback-start");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.handle_connection), "Connection state handler");
  gtk_widget_set_sensitive(GTK_WIDGET(toolbar_context.handle_connection), TRUE);
  g_signal_connect(toolbar_context.handle_connection, "clicked", G_CALLBACK(toolbar_on_iface_handler), window);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.load_pcap), "document-open");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.load_pcap), "Load PCAP");
  g_signal_connect(toolbar_context.load_pcap, "clicked", G_CALLBACK(toolbar_on_load_pcap), window);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.save_pcap), "document-save");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.save_pcap), "Save PCAP");
  g_signal_connect(toolbar_context.save_pcap, "clicked", G_CALLBACK(toolbar_on_save_pcap), window);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.packet_sender), "mail-send-symbolic");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.packet_sender), "CCSDS Packet Sender");
  g_signal_connect(toolbar_context.packet_sender, "clicked", G_CALLBACK(toolbar_on_packet_sender), window);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.intruder), "media-playlist-repeat-symbolic");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.intruder), "Intruder");
  g_signal_connect(toolbar_context.intruder, "clicked", G_CALLBACK(toolbar_on_intruder), NULL);

  gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolbar_context.builder), "mail-send-symbolic");
  gtk_widget_set_tooltip_text(GTK_WIDGET(toolbar_context.builder), "Advance Builder");
  g_signal_connect(toolbar_context.builder, "clicked", G_CALLBACK(toolbar_on_builder), window);

  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.iface), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.handle_connection), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar),  gtk_separator_tool_item_new(), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.load_pcap), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.save_pcap), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar),  gtk_separator_tool_item_new(), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.packet_sender), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.intruder), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(toolbar_context.builder), -1);

  return toolbar;
}
