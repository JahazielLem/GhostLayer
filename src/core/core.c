/* src/core - core.c
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "core.h"
#include "backend/backend.h"
#include "ui/components/table.h"
#include "ui/components/tree-view.h"
#include "dissectors/dissector.h"
#include "dissectors/protocols/protocols.h"
#include <sparkcli/sparkcli.h>


#define DEFAULT_PORT 9090

static gboolean core_parser_data_cb(GIOChannel *source, GIOCondition condition, gpointer user_data) {
  if (condition & (G_IO_HUP | G_IO_ERR)) {
    scli_log_error("Client disconnected or error");
    return FALSE;
  }

  uint8_t buffer[1024];
  gsize len = 0;
  GError *error = NULL;

  GIOStatus status = g_io_channel_read_chars(source, (gchar *)buffer, sizeof(buffer) - 1, &len, &error);
  if (status == G_IO_STATUS_NORMAL && len > 0) {
    buffer[len] = '\0';
    if (buffer[0] == 0x64 && buffer[1] == 0x83 && buffer[len - 2] == 0x64 && buffer[len - 1] == 0x69) {

      GLPacket *item = gl_packet_new(buffer + 2, len -4, table_get_item_count());

      GLDissector *raw_dissector = gl_dissector_get("SPP");
      if (raw_dissector != NULL) {
        raw_dissector->dissect_summary(item);
      }
      scli_log_info("Packet received. Length %d:", item->length);
      scli_hexdump16(item->data, item->length);
      table_add_item(item);
    }
  } else if (status == G_IO_STATUS_ERROR && error) {
    scli_log_error("TCP read error: %s", error->message);
    g_error_free(error);
  }

  return TRUE;
}

static void core_dissect_data_cb(GLPacket *packet) {
  scli_log_info("Packet to dissect. Length %d:", packet->length);
  scli_hexdump16(packet->data, packet->length);

  if (!packet) {return;}

  main_layout_treeview_clear_records();

  if (!packet->fully_dissected) {
    GLDissector *d = gl_dissector_find(packet->data, packet->length);
    if (d && d->dissect) {
      d->dissect(packet);
    }
  }

  if (!packet->root){return;}

  treeview_add_proto_node(NULL, packet->root);

  main_layout_treeview_expand();
}

void core_init(void) {
  tcp_engine_init(DEFAULT_PORT);
  tcp_engine_listen_async();
  tcp_engine_register_read_cb(core_parser_data_cb);
  table_register_select_cb(core_dissect_data_cb);
}