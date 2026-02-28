/* src/dissectors/protocols - spp.c
 *
 * GhostLayer - By astrobyte 28/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "protocols.h"
#include <sparkcli/sparkcli.h>

// SPP
#define CCSDS_VERSION 0
// Common
#define MAX_SPP_PACKET_SIZE 64
#define MAX_TC_FRAME_SIZE 128
#define MAX_TM_FRAME_SIZE 128

typedef enum { PACKET_TYPE_TM = 0, PACKET_TYPE_TC = 1 } packet_type_t;

typedef enum {
  GROUPING_FLAG_CONT = 0b00,
  GROUPING_FLAG_START = 0b01,
  GROUPING_FLAG_END = 0b10,
  GROUPING_FLAG_UNSEGMENTED = 0b11
} grouping_flag_t;

typedef struct {
  // Primary Header (6 bytes)
  uint16_t packet_id; // Version(3) + Type(1) + SecHdr(1) + APID(11)
  uint16_t packet_sequence; // Seq flags(2) + Seq count(14)
  uint16_t length; // Packet length = (len(payload) + len(secondary_header)) - 1
} __attribute__((packed)) spp_primary_header_t;

typedef struct {
  spp_primary_header_t header;
  uint8_t data[MAX_SPP_PACKET_SIZE - 6]; // Secondary header + payload
} __attribute__((packed)) spp_packet_t;

static gboolean spp_can_handle(const uint8_t *data, size_t len);
static void spp_dissect_summary(GLPacket *packet);
static void spp_dissect_full(GLPacket *packet);

GLDissector spp_dissector = {
  .name = "SPP",
  .long_name = "CCSDS - Space Packet Protocol",
  .can_handle = spp_can_handle,
  .dissect_summary = spp_dissect_summary,
  .dissect = spp_dissect_full
};


static gboolean spp_can_handle(const uint8_t *data, size_t len) {
  return TRUE;
}

static void spp_dissect_summary(GLPacket *packet) {
  packet->summary_protocol = g_strdup(spp_dissector.name);
  packet->summary_info = g_strdup_printf("Length: %lu", packet->length);
  packet->summary_source = g_strdup("-");
  packet->summary_dest = g_strdup("-");
}

static void spp_dissect_full(GLPacket *packet){
  packet->root = gl_proto_node_new(spp_dissector.long_name, 0, packet->length, "");
  // gl_proto_node_add_child(, root_node);

  spp_packet_t space_packet;

  memset(&space_packet, 0, sizeof(spp_packet_t));
  space_packet.header.packet_id = (packet->data[1] << 8) | packet->data[0];
  space_packet.header.packet_sequence = (packet->data[3] << 8) | packet->data[2];
  space_packet.header.length = (packet->data[5] << 8) | packet->data[4];

  const uint16_t payload_len = space_packet.header.length;

  if (payload_len > MAX_SPP_PACKET_SIZE || payload_len > packet->length - 6) {
    scli_log_error("[Error: Malformed Packet - Payload len error]");
    GLProtoNode *node = gl_proto_node_new("[Error: Malformed Packet - Payload len error]", 0, packet->length, "");
    gl_proto_node_add_child(packet->root, node);
    packet->fully_dissected = TRUE;
    return;
  }

  char buffer[512];
  sprintf(buffer, "Packet Primary Header: 0x%02X", space_packet.header.packet_id);
  GLProtoNode *node_header = gl_proto_node_new(buffer, 0, packet->length, "");
  gl_proto_node_add_child(packet->root, node_header);

  sprintf(buffer, "Packet Sequence Control: 0x%02X", space_packet.header.packet_sequence);
  GLProtoNode *node_seq = gl_proto_node_new(buffer, 0, packet->length, "");
  gl_proto_node_add_child(packet->root, node_seq);

  sprintf(buffer, "[Length: %d bytes]", payload_len);
  GLProtoNode *node_length = gl_proto_node_new(buffer, 0, packet->length, "");
  gl_proto_node_add_child(packet->root, node_length);

  if (payload_len > 0) {
    memcpy(space_packet.data, packet->data, payload_len);
  }
  packet->fully_dissected = TRUE;
}


