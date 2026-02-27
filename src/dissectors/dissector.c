/* src/dissectors - dissector.c
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "dissector.h"

static GList *registered_dissectors = NULL;

void dissector_register(GLDissector *dissector) {
  registered_dissectors = g_list_append(registered_dissectors, dissector);
}

GLDissector *gl_dissector_find(const uint8_t *data, const size_t len) {
  for (const GList *l = registered_dissectors; l != NULL; l = l->next) {
    GLDissector *d = l->data;
    if (d->can_handle && d->can_handle(data, len)){ return d;}
  }
  return NULL;
}