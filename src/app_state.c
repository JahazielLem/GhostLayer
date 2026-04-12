/**
 * @file src/app_state.c
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

#include "../include/main_gui.h"
#include "../include/bridge.h"
#include "../include/plugins.h"
#include "../include/app_state.h"
#include "../include/dissectors.h"

static int server_port = BRIDGE_DEFAULT_PORT;
static gboolean is_connected = FALSE;

static app_radio_config_t radio_config;

void app_state_server_set_state(const gboolean state) {
  is_connected = state;
  statusbar_update_label_connection(state);
}

gboolean app_state_server_get_state(void) {
  return is_connected;
}

void app_state_server_set_port(const int port) {
  server_port = port;
}

int app_state_server_get_port(void) {
  return server_port;
}

void app_state_server_init(const int port) {
  if (bridge_engine_server_fd() < 0) {
    bridge_engine_init(port);
    bridge_engine_listen_async();
  }
}

void app_state_server_cleanup(void) {
  if (bridge_engine_server_fd() >= 0){ bridge_engine_close(); }
}

void app_state_new_packet(char *protocol, char *information, uint8_t *buffer, const int length) {
  packet_viewer_add(protocol, information, buffer, length);
  statusbar_update_label_packet_count(packet_viewer_get_count());
}

void app_state_new_packet_from_file(uint8_t *buffer, const gsize buffer_len) {
  dissector_packet_parser_from_file(buffer, buffer_len);
}

void app_state_clear_packet_viewer(void) {
  packet_viewer_clear();
}

void app_state_transmit_packet(void) {
  int offset = 0;
  uint8_t buffer[64];

  radio_config.frequency = plugin_radio_get_frequency();
  radio_config.bandwidth = plugin_radio_get_bandwidth();
  radio_config.spread_factor = plugin_radio_get_spread_factor();

  buffer[offset++] = 0x64;
  buffer[offset++] = 0x83;

  buffer[offset++] = (uint8_t)((radio_config.frequency >> 24) & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.frequency >> 16) & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.frequency >> 8)  & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.frequency & 0xFF);

  buffer[offset++] = (uint8_t)((radio_config.bandwidth >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.bandwidth & 0xFF);

  buffer[offset++] = (uint8_t)((radio_config.spread_factor >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.spread_factor & 0xFF);

  buffer[offset++] = 0x64;
  buffer[offset++] = 0x69;
  buffer[offset++] = '\0';

  bridge_engine_send(buffer, offset);
}

void app_state_transmit_packet_with_config(uint8_t *payload, uint16_t payload_length) {
  int offset = 0;
  uint8_t buffer[(13 + 2 + payload_length)];

  radio_config.frequency = plugin_radio_get_frequency();
  radio_config.bandwidth = plugin_radio_get_bandwidth();
  radio_config.spread_factor = plugin_radio_get_spread_factor();

  /* Header */
  buffer[offset++] = 0x63;
  buffer[offset++] = 0x83;
  /* Radio Configuration */
  buffer[offset++] = (uint8_t)((radio_config.frequency >> 24) & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.frequency >> 16) & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.frequency >> 8)  & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.frequency & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.bandwidth >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.bandwidth & 0xFF);
  buffer[offset++] = (uint8_t)((radio_config.spread_factor >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(radio_config.spread_factor & 0xFF);

  /* Payload */
  buffer[offset++] = (uint8_t)((payload_length >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(payload_length & 0xFF);

  memcpy(buffer + offset, payload, payload_length);
  offset += payload_length;

  /* Tail */
  buffer[offset++] = 0x64;
  buffer[offset++] = 0x69;
  buffer[offset++] = '\0';

  bridge_engine_send(buffer, offset);
}
