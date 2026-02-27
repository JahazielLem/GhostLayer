/* src/ui/components - main-layout.h
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_MAIN_LAYOUT_H
#define GHOSTLAYER_MAIN_LAYOUT_H

#include <gtk/gtk.h>

#define APPLICATION_MAIN_NAME "GhostLayer - IoT Packet Analyzer"
#define APPLICATION_WEBSITE "https://pwnsat.org/"
#define APPLICATION_COMMENTS "IoT Packet Analyzer for hackers."
#define DEFAULT_WIDTH (1280)
#define DEFAULT_HEIGHT (720)

void main_layout_init(GtkApplication *app, gpointer user_data);
#endif //GHOSTLAYER_MAIN_LAYOUT_H
