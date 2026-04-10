/**
 * @file src/dissectors/spp.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-10
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#include "../../include/main_gui.h"
#include <ccsds.h>

typedef struct {
  uint16_t value;
  const char *string;
} value_string_map_t;

static const value_string_map_t mapping[] = {
    {100, "Data"},
    {200, "Command"},
    {300, "Sensors"},
    {0xFFF, "IDLE"},
};

static proto_dissector_handle_t handle;
static const int mapping_size = sizeof(mapping) / sizeof(mapping[0]);

const char *map_value_to_string(int value) {
  int left = 0, right = mapping_size - 1;
  while (left <= right) {
    int mid = left + (right - left) / 2;
    if (mapping[mid].value == value) {
      return mapping[mid].string;
    } else if (mapping[mid].value < value) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return "Unknown";
}

static proto_dissector_handle_t *dissector_register_handler(void);
static proto_module_t module_handler = {.name = "SPP", .register_fn = dissector_register_handler};

static int dissector_is_valid(proto_tvbuff_t *tvbuff) {
  space_packet_t space_packet;
  if (spp_unpack_packet(&space_packet, tvbuff->real_data, tvbuff->length) != SPP_ERROR_NONE){ return 0;}
  return 1;
}

static int dissector_dissect(proto_packet_t *packet, GtkTreeIter *root, uint8_t *data) {
  (void)root;
  (void)data;
  char buffer[256];

  GtkTreeIter troot = packet_details_add_field(NULL, "Space Packet Protocol", "", 0, packet->length);

  space_packet_t space_packet;
  if (spp_unpack_packet(&space_packet, packet->buffer, packet->length) != SPP_ERROR_NONE) {
    g_warning("Invalid packet skipping %d \n", packet->length);
    sprintf(buffer, "[Error: Malformed Packet - Payload len error]");
    packet_details_add_field(&troot, "", buffer, 0, packet->length);
    return 0;
  }

  bitfield_t fields_identification[] = {{"Version", 13, 3}, {"Type", 11, 1}, {"SecHeader", 10, 1}, {"APID", 0, 11}};
  sprintf(buffer, "Packet Primary Header: 0x%02x", space_packet.header.identification);
  packet_details_add_field(&troot, buffer, "", 0, 1);
  packet_details_add_bitfield(&troot, space_packet.header.identification, 2, fields_identification, 4, 0, 1);
  bitfield_t fields_sequence_control[] = {
    {"Flags", 14, 2},
    {"Count", 0, 14},
  };
  sprintf(buffer, "Packet Sequence Control: 0x%02x", space_packet.header.sequence);
  packet_details_add_field(&troot, buffer, "", 2, 3);
  packet_details_add_bitfield(&troot, space_packet.header.sequence, 2, fields_sequence_control, 2, 2, 3);

  GtkTreeIter tpdata_field = packet_details_add_field(&troot, "Packer Data Field", "", 0, packet->length);
  snprintf(buffer, sizeof(buffer), "%s",
             uint8_buffer_to_hex_string_separator(packet->buffer + 6, space_packet.header.length, ""));
  packet_details_add_field(&tpdata_field, "Payload:", buffer, 6, packet->length);

  sprintf(buffer, "[Length: %u bytes]", space_packet.header.length);
  packet_details_add_field(&tpdata_field, buffer, "", 4, 5);
  return packet->length;
}

static proto_dissector_handle_t *dissector_register_handler(void) {
  handle = *proto_create_dissector("SPP", "Space Packet Protocol", dissector_is_valid, dissector_dissect);
  return &handle;
}

proto_module_t *dissector_register_spp(void) { return &module_handler; }
