/**
 * @file include/bridge.h
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
#ifndef GHOSTLAYER_BRIDGE_H
#define GHOSTLAYER_BRIDGE_H

#include <glib.h>

#define BRIDGE_MAGIC_HEADER_1 0x64
#define BRIDGE_MAGIC_HEADER_2 0x83
#define BRIDGE_MAGIC_TAIL_1   0x64
#define BRIDGE_MAGIC_TAIL_2   0x69
#define BRIDGE_DEFAULT_PORT 5008

typedef gboolean (*tcp_engine_read_callback)(GIOChannel *source, GIOCondition condition, gpointer user_data);

void bridge_engine_register_read_cb(tcp_engine_read_callback const cb);
int bridge_engine_get_fd(void);
int bridge_engine_is_connected(void);
int bridge_engine_server_fd(void);
void bridge_engine_listen_async(void);
void bridge_engine_send(const uint8_t *data, const int len);
void bridge_engine_close(void);
int bridge_engine_init(int port);

#endif //GHOSTLAYER_BRIDGE_H
