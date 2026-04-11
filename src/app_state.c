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

static int server_port = BRIDGE_DEFAULT_PORT;
static gboolean is_connected = FALSE;

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

void app_state_new_packet(const char *protocol, const char *information, const uint8_t *buffer, const int length) {
  packet_viewer_add(protocol, information, buffer, length);
  statusbar_update_label_packet_count(packet_viewer_get_count());
}

void app_state_transmit_packet(void) {
  int offset = 0;
  uint8_t buffer[64];

  const uint32_t freq = plugin_radio_get_frequency();
  const uint16_t bw = plugin_radio_get_bandwidth();
  const uint16_t sf = plugin_radio_get_spread_factor();

  buffer[offset++] = 0x64;
  buffer[offset++] = 0x83;

  buffer[offset++] = (uint8_t)((freq >> 24) & 0xFF);
  buffer[offset++] = (uint8_t)((freq >> 16) & 0xFF);
  buffer[offset++] = (uint8_t)((freq >> 8)  & 0xFF);
  buffer[offset++] = (uint8_t)(freq & 0xFF);

  buffer[offset++] = (uint8_t)((bw >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(bw & 0xFF);

  buffer[offset++] = (uint8_t)((sf >> 8) & 0xFF);
  buffer[offset++] = (uint8_t)(sf & 0xFF);

  buffer[offset++] = 0x64;
  buffer[offset++] = 0x69;
  buffer[offset++] = '\0';

  bridge_engine_send(buffer, offset);
}