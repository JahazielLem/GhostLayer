/* src/core - packet.h
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_PACKET_H
#define GHOSTLAYER_PACKET_H

#include <glib.h>
#include "proto.h"

GLPacket *gl_packet_new(const uint8_t *data, size_t length);
void gl_packet_free(GLPacket *pkt);

GLProtoNode *gl_proto_node_new(char *label, guint offset, guint length);
void gl_proto_node_add_child(GLProtoNode *parent, GLProtoNode *child);
void gl_proto_node_free(GLProtoNode *node);

#endif //GHOSTLAYER_PACKET_H
