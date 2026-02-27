/* src/backend - sdr-gnuradio.h
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_SDR_GNURADIO_H
#define GHOSTLAYER_SDR_GNURADIO_H

#include <glib.h>

typedef gboolean (*tcp_engine_read_callback)(GIOChannel *source, GIOCondition condition, gpointer user_data);

void tcp_engine_register_read_cb(tcp_engine_read_callback cb);
int tcp_engine_get_fd(void);
int tcp_engine_is_connected(void);
int tcp_engine_init(int port);
void tcp_engine_listen_async(void);
void tcp_engine_send(uint8_t *data, int len);
void tcp_engine_close(void);

#endif //GHOSTLAYER_SDR_GNURADIO_H
