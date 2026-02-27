/* src/dissectors/protocols - raw.c
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include "protocols.h"

gboolean raw_can_handle(const uint8_t *data, size_t len) {
  return TRUE;
}

void raw_dissect_summary(GLPacket *packet) {
  packet->summary_protocol = g_strdup("RAW");
  packet->summary_info = g_strdup_printf("Length: %lu", packet->length);
  packet->summary_source = g_strdup("-");
  packet->summary_dest = g_strdup("-");
}

void raw_dissect_full(GLPacket *pkt){
  GLProtoNode *node =
      gl_proto_node_new("Raw Payload", 0, pkt->length);

  gl_proto_node_add_child(pkt->root, node);
  pkt->fully_dissected = TRUE;
}