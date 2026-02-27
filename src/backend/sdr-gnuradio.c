/* src/backend - sdr-gnuradio.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "sdr-gnuradio.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sparkcli/sparkcli.h>

#define BACKLOG 10

static int server_fd = -1;
static int client_fd = -1;
static GIOChannel *tcp_channel = NULL;
static tcp_engine_read_callback tcp_read_callback = NULL;

void tcp_engine_register_read_cb(tcp_engine_read_callback cb) { tcp_read_callback = cb; }

int tcp_engine_get_fd(void) { return client_fd; }

int tcp_engine_is_connected(void) { return client_fd >= 0; }

int tcp_engine_init(int port) {
  struct sockaddr_in address;
  int opt = 1;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    scli_log_error("Socket creation failed");
    return -1;
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    scli_log_error("Error setting socket options");
    close(server_fd);
    return -1;
  }

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
    scli_log_error("Bind failed");
    close(server_fd);
    return -1;
  }

  if (listen(server_fd, BACKLOG) < 0) {
    scli_log_error("Listen failed");
    close(server_fd);
    return -1;
  }

  fcntl(server_fd, F_SETFL, O_NONBLOCK);

  scli_log_info("TCP Server listening on port %d", port);
  return server_fd;
}

static gboolean tcp_server_accept_cb(GIOChannel *source, GIOCondition condition, gpointer user_data) {
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  const int new_fd = accept(server_fd, (struct sockaddr *) &client_addr, &addr_len);

  if (new_fd < 0) {
    scli_log_error("Accept failed");
    return FALSE;
  }

  scli_log_info("New TCP client connected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  client_fd = new_fd;

  tcp_channel = g_io_channel_unix_new(client_fd);
  g_io_channel_set_encoding(tcp_channel, NULL, NULL);
  g_io_channel_set_flags(tcp_channel, G_IO_FLAG_NONBLOCK, NULL);

  g_io_add_watch(tcp_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, tcp_read_callback, NULL);

  return TRUE;
}

void tcp_engine_listen_async(void) {
  if (server_fd < 0)
    return;

  GIOChannel *server_channel = g_io_channel_unix_new(server_fd);
  g_io_channel_set_encoding(server_channel, NULL, NULL);
  g_io_channel_set_flags(server_channel, G_IO_FLAG_NONBLOCK, NULL);

  g_io_add_watch(server_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, tcp_server_accept_cb, NULL);
}

void tcp_engine_send(uint8_t *data, int len) {
  if (client_fd < 0) {
    scli_log_error("No TCP client connected");
    return;
  }
  write(client_fd, data, len);
  write(client_fd, "\n", 1);
}

void tcp_engine_close(void) {
  if (client_fd >= 0) {
    close(client_fd);
    client_fd = -1;
  }
  if (server_fd >= 0) {
    close(server_fd);
    server_fd = -1;
  }
  if (tcp_channel) {
    g_io_channel_unref(tcp_channel);
    tcp_channel = NULL;
  }
}
