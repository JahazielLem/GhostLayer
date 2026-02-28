/* src - main.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "ui/ui-manager.h"
#include "core/core.h"
#include "dissectors/dissector.h"

int main(const int argc, char *argv[]){
  gl_dissector_init_core();
  core_init();
  return ui_manager_init(argc, argv);
}
