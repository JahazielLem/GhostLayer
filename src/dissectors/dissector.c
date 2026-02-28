/* src/dissectors - dissector.c
 *
 * GhostLayer - By astrobyte 27/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "dissector.h"

#include "protocols/protocols.h"

static GList *registered_dissectors = NULL;
extern GLDissector raw_dissector;
extern GLDissector spp_dissector;

void dissector_register(GLDissector *dissector) {
  registered_dissectors = g_list_append(registered_dissectors, dissector);
}

GLDissector *gl_dissector_find(const uint8_t *data, const size_t len) {
  for (const GList *l = registered_dissectors; l != NULL; l = l->next) {
    GLDissector *d = l->data;
    if (d->can_handle && d->can_handle(data, len) && strcmp(d->name, "RAW") != 0){ return d;}
  }
  return NULL;
}

GLDissector *gl_dissector_get(const char *name) {
  for (const GList *l = registered_dissectors; l != NULL; l = l->next) {
    GLDissector *d = l->data;
    if (strcmp(d->name, name) == 0) {return d;}
  }
  return NULL;
}

void gl_dissector_init_core(void) {
  dissector_register(&raw_dissector);
  dissector_register(&spp_dissector);
}