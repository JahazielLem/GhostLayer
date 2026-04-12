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
#include "alerts.h"
#include "app_state.h"
#include "proto.h"

typedef struct {
  int packet_count;
} pcap_callback_data_t;

static void on_packet_read(u_char *user_data, const struct pcap_pkthdr *packet_header, const u_char *packet) {
  pcap_callback_data_t *ctx = (pcap_callback_data_t *)user_data;
  g_print("Capture length: %d - Length %d\n", packet_header->caplen, packet_header->len);
  struct timeval ts_copy = packet_header->ts;
  app_state_new_packet_from_file((uint8_t*)packet, packet_header->len, &ts_copy);
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

void pcap_reader_open_dialog(GtkButton *button, gpointer user_data) {
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

void pcap_reader_save_dialog(GtkButton *button, gpointer user_data) {
  (void)button;

  GtkWidget *dialog = gtk_file_chooser_dialog_new("Save PCAP File",
    GTK_WINDOW(user_data),
    GTK_FILE_CHOOSER_ACTION_SAVE,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Save", GTK_RESPONSE_ACCEPT,
    NULL);

  gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "ghost_capture.pcap");

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(filter, "*.pcap");
  gtk_file_filter_set_name(filter, "PCAP file (*.pcap)");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    GList *list = app_sate_get_packet_list();
    pcap_t *pd = pcap_open_dead(148, 65535);
    if (pd != NULL) {
      pcap_dumper_t *pdumper = pcap_dump_open(pd, filename);
      if (pdumper != NULL) {
        struct timeval ts;
        gettimeofday(&ts, NULL);
        for (GList *l = list; l != NULL; l = l->next) {
          proto_packet_t *pkt = (proto_packet_t *)l->data;

          struct pcap_pkthdr hdr;
          hdr.ts = pkt->timestamp;
          hdr.caplen = pkt->length;
          hdr.len = pkt->length;

          pcap_dump((u_char *)pdumper, &hdr, pkt->buffer);
        }
        pcap_dump_close(pdumper);
        g_print("Successfully save %d packets to %s\n", g_list_length(list), filename);

        alert_show_dialog(GTK_WINDOW(user_data), ALERT_SUCCESS, "Successfully saved", filename);

      }else {
        g_printerr("[!] Error opening PCAP dumper: %s\n", pcap_geterr(pd));
      }
      pcap_close(pd);
      } else{
      g_printerr("Error creating dead pcap handle\n");
    }
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
}

