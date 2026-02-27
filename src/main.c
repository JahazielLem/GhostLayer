/* src - main.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "ui/ui-manager.h"
#include "core/core.h"

int main(const int argc, char *argv[]){
  core_init();
  return ui_manager_init(argc, argv);
}
