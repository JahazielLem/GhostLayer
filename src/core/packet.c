/* src/core - packet.c
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include "packet.h"

GLPacket *gl_packet_new(const uint8_t *data, size_t length, uint16_t frame_counter) {
  GLPacket *packet = g_malloc0(sizeof(GLPacket));

  packet->data = g_memdup2(data, length);
  packet->length = length;

  packet->frame_counter = frame_counter;

  packet->summary_source = "-";
  packet->summary_dest = "-";
  packet->summary_protocol = "RAW";
  packet->summary_info = "-";

  char buffer[512];
  sprintf(buffer, "Frame %d: Packet, %zu bytes", packet->frame_counter, packet->length);

  packet->root = gl_proto_node_new(buffer, 0, length, "Value");
  return packet;
}
void gl_packet_free(GLPacket *pkt) {
  if (!pkt) { return; }

  g_free(pkt->data);
  g_free(pkt->summary_protocol);
  g_free(pkt->summary_info);
  g_free(pkt->summary_source);
  g_free(pkt->summary_dest);

  gl_proto_node_free(pkt->root);

  g_free(pkt);
}

GLProtoNode *gl_proto_node_new(char *label, guint offset, guint length, char *value) {
  GLProtoNode *node = g_malloc0(sizeof(GLProtoNode));
  node->label = g_strdup(label);
  node->offset = offset;
  node->length = length;
  node->start = offset;
  node->end = offset + length;
  node->value = g_strdup(value);
  node->malformed = 0;
  return node;
}
void gl_proto_node_add_child(GLProtoNode *parent, GLProtoNode *child) {
  if (!parent) { return; }
  parent->children = g_list_append(parent->children, child);
}
void gl_proto_node_free(GLProtoNode *node) {
  if (!node) { return; }

  g_free(node->label);
  for (GList *l = node->children; l != NULL; l = l->next) {
    gl_proto_node_free(l->data);
  }

  g_list_free(node->children);
  g_free(node);
}