/* src/dissectors - dissector.h
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_DISSECTOR_H
#define GHOSTLAYER_DISSECTOR_H

#include <glib.h>
#include "proto.h"
#include "core/packet.h"

typedef struct _GLDissector GLDissector;

struct _GLDissector {
  const char *name;
  const char *long_name;

  gboolean (*can_handle)(const uint8_t *data, size_t len);
  void (*dissect_summary)(GLPacket *packet);
  void (*dissect)(GLPacket *packet);
};

void gl_dissector_register(GLDissector *dissector);
GLDissector *gl_dissector_find(const uint8_t *data, size_t len);
GLDissector *gl_dissector_get(const char *name);
void gl_dissector_init_core(void);

#endif //GHOSTLAYER_DISSECTOR_H
