/* src/dissectors/protocols - protocols.h
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_PROTOCOLS_H
#define GHOSTLAYER_PROTOCOLS_H

#include <glib.h>
#include "proto.h"
#include "core/packet.h"
#include "dissectors/dissector.h"

void raw_dissect_full(GLPacket *pkt);
void raw_dissect_summary(GLPacket *packet);
gboolean raw_can_handle(const uint8_t *data, size_t len);
#endif //GHOSTLAYER_PROTOCOLS_H
