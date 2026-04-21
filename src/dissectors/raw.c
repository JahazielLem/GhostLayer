/**
 * @file src/dissectors/raw.c
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

static proto_dissector_handle_t handle;
static proto_dissector_handle_t *dissector_register_handler(void);
static proto_module_t module_handler = {.name = "LoRa", .register_fn = dissector_register_handler};

static int dissector_is_valid(proto_tvbuff_t *tvbuff) {
  (void)tvbuff;
  return 1;
}

static int dissector_dissect(proto_packet_t *packet, GtkTreeIter *root, uint8_t *data) {
  (void)data;
  char buffer[256];

  GtkTreeIter troot = packet_details_add_field(root, handle.name, "", 0, packet->length);

  snprintf(buffer, sizeof(buffer), "%s", uint8_buffer_to_hex_string_separator(packet->buffer, packet->length, ""));
  packet_details_add_field(&troot, "Data:", buffer, 0, packet->length);
  packet_hexdump_add_field_value(0, packet->length);

  snprintf(buffer, sizeof(buffer), "[Length: %d bytes]", packet->length);
  packet_details_add_field(&troot, buffer, "", -1, packet->length);
  return packet->length;
}

static proto_dissector_handle_t *dissector_register_handler(void) {
  handle = *proto_create_dissector("LoRa", "LoRa Packet Data", dissector_is_valid, dissector_dissect);
  return &handle;
}

proto_module_t *dissector_register_lora(void) { return &module_handler; }
