/**
 * @file include/app_state.h
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
#ifndef GHOSTLAYER_APP_STATE_H
#define GHOSTLAYER_APP_STATE_H

#include "bridge.h"

typedef struct {
  uint32_t frequency;
  uint16_t bandwidth;
  uint16_t spread_factor;
} app_radio_config_t;

gboolean app_state_server_get_state(void);
void app_state_server_set_port(int port);
int app_state_server_get_port(void);
void app_state_server_init(int port);
void app_state_server_set_state(gboolean state);
void app_state_server_cleanup(void);
void app_state_new_packet(char *protocol, char *information, uint8_t *buffer, int length);
void app_state_transmit_packet(void);
void app_state_transmit_packet_with_config(uint8_t *payload, uint16_t payload_length);
#endif //GHOSTLAYER_APP_STATE_H
