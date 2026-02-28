/* src - proto.h
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_PROTO_H
#define GHOSTLAYER_PROTO_H

#include <gtk/gtk.h>
#include <stdint.h>

typedef struct _GLProtoNode GLProtoNode;

struct _GLProtoNode {
  char *value;
  gchar *label;
  guint offset;
  guint length;
  int start;
  int end;
  uint8_t malformed;
  GList *children;
};

typedef struct {
  int length;
  uint8_t buffer[512];
} proto_packet_t;

typedef struct {
  int length;
  uint8_t *buffer;
  char *source;
  char *dest;
  char *info;
  char *protocol;
} proto_table_item_t;

typedef struct {
  uint8_t *data;
  size_t length;

  uint16_t frame_counter;

  gchar *summary_protocol;
  gchar *summary_info;
  gchar *summary_source;
  gchar *summary_dest;

  gboolean fully_dissected;

  GLProtoNode *root;
} GLPacket;

typedef void (*on_select_packet_cb)(GLPacket *pkt);

#endif //GHOSTLAYER_PROTO_H
