/**
 * @file src/bridge.c
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

#include <arpa/inet.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bridge.h"

#define BACKLOG 10

typedef struct {
  int server_fd;
  int client_fd;
} server_context_t;

static GIOChannel *tcp_channel = NULL;
static server_context_t server_context = {.server_fd = -1, .client_fd = -1};
static tcp_engine_read_callback tcp_read_callback = NULL;

static gboolean bridge_server_accept_cb(GIOChannel *source, GIOCondition condition, gpointer user_data) {
  (void)source;
  (void)condition;
  (void)user_data;

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  int new_fd = accept(server_context.server_fd, (struct sockaddr *) &client_addr, &addr_len);

  if (new_fd < 0) {
    g_printerr("Accept failed\n");
    return FALSE;
  }

  g_print("New TCP client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  server_context.client_fd = new_fd;

  tcp_channel = g_io_channel_unix_new(server_context.client_fd);
  g_io_channel_set_encoding(tcp_channel, NULL, NULL);
  g_io_channel_set_flags(tcp_channel, G_IO_FLAG_NONBLOCK, NULL);

  g_io_add_watch(tcp_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, tcp_read_callback, NULL);

  return TRUE;
}

void bridge_engine_register_read_cb(tcp_engine_read_callback const cb) { tcp_read_callback = cb; }

int bridge_engine_get_fd(void) { return server_context.client_fd; }

int bridge_engine_is_connected(void) { return server_context.client_fd >= 0; }

int bridge_engine_server_fd(void) { return server_context.server_fd;}

void bridge_engine_listen_async(void) {
  if (server_context.server_fd < 0)
    return;

  GIOChannel *server_channel = g_io_channel_unix_new(server_context.server_fd);
  g_io_channel_set_encoding(server_channel, NULL, NULL);
  g_io_channel_set_flags(server_channel, G_IO_FLAG_NONBLOCK, NULL);

  g_io_add_watch(server_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, bridge_server_accept_cb, NULL);
}

void bridge_engine_send(const uint8_t *data, const int len) {
  if (server_context.client_fd < 0) {
    g_printerr("No TCP client connected\n");
    return;
  }
  write(server_context.client_fd, data, len);
  write(server_context.client_fd, "\n", 1);
}

void bridge_engine_close(void) {
  if (server_context.client_fd >= 0) {
    close(server_context.client_fd);
    server_context.client_fd = -1;
  }
  if (server_context.server_fd >= 0) {
    close(server_context.server_fd);
    server_context.server_fd = -1;
  }
  if (tcp_channel) {
    g_io_channel_unref(tcp_channel);
    tcp_channel = NULL;
  }

  g_print("TCP Server connection closed\n");
}

int bridge_engine_init(int port) {
  if (server_context.server_fd >= 0){return server_context.server_fd; }

  struct sockaddr_in address;
  int opt = 1;

  if ((server_context.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    g_printerr("Socket creation failed\n");
    return -1;
  }

  if (setsockopt(server_context.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    g_printerr("Error setting socket options\n");
    close(server_context.server_fd);
    return -1;
  }

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_context.server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
    g_printerr("Bind failed\n");
    close(server_context.server_fd);
    return -1;
  }

  if (listen(server_context.server_fd, BACKLOG) < 0) {
    g_printerr("Listen failed\n");
    close(server_context.server_fd);
    return -1;
  }

  fcntl(server_context.server_fd, F_SETFL, O_NONBLOCK);

  g_print("TCP Server listening on port %d\n", port);
  return server_context.server_fd;
}
