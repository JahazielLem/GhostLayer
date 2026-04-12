/**
 * @file src/pcap_files.c
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
#include <pcap.h>
#include "../include/alerts.h"
#include "../include/app_state.h"

typedef struct {
  int packet_count;
} pcap_callback_data_t;

static void on_packet_read(u_char *user_data, const struct pcap_pkthdr *packet_header, const u_char *packet) {
  pcap_callback_data_t *ctx = (pcap_callback_data_t *)user_data;
  g_print("Capture length: %d - Length %d\n", packet_header->caplen, packet_header->len);
  app_state_new_packet_from_file((uint8_t*)packet, packet_header->len);
  ctx->packet_count++;
}

void pcap_reader_open_file(const char*filename) {
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *handle = pcap_open_offline(filename, errbuf);

  if (handle == NULL) {
    g_printerr("Error Opening PCAP: %s\n", errbuf);
    alert_show_dialog(NULL, ALERT_ERROR, "Error Opening",errbuf );
    return;
  }

  pcap_callback_data_t ctx = {.packet_count = 0 };

  pcap_loop(handle, -1, on_packet_read, (u_char *)&ctx);
  pcap_close(handle);
  g_print("[*] Loaded %d packets from %s\n", ctx.packet_count, filename);
}

void pcap_reader_dialog(GtkButton *button, gpointer user_data) {
  (void)button;

  GtkWidget *dialog = gtk_file_chooser_dialog_new("Open PCAP File",
    GTK_WINDOW(user_data),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL);

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(filter, "*.pcap");
  gtk_file_filter_set_name(filter, "PCAP file (*.pcap)");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    app_state_clear_packet_viewer();
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    pcap_reader_open_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
}
