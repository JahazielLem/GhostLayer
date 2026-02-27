/* src/ui - ui-manager.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "ui-manager.h"

int ui_manager_init(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.jahaziellem.GhostLayer", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(main_layout_init), NULL);
  const int ret = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return ret;
}