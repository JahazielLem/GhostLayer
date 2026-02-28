/* src/ui/components - table.h
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_TABLE_H
#define GHOSTLAYER_TABLE_H

#include <gtk/gtk.h>
#include "proto.h"

GtkWidget *main_layout_table_init(void);
void table_add_item(GLPacket *item);
uint16_t table_get_item_count(void);
void table_switch_scroll(void);
void table_register_select_cb(on_select_packet_cb select_cb);
#endif //GHOSTLAYER_TABLE_H
