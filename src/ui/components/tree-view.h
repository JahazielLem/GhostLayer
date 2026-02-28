/* src/ui/components - tree-view.h
 *
 * GhostLayer - By astrobyte 28/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef GHOSTLAYER_TREE_VIEW_H
#define GHOSTLAYER_TREE_VIEW_H

#include <gtk/gtk.h>
#include "proto.h"

GtkWidget *main_layout_treeview_init(void);
void main_layout_treeview_clear_records(void);
void main_layout_treeview_expand(void);
void treeview_add_proto_node(GtkTreeIter *parent_iter, GLProtoNode *node);
#endif //GHOSTLAYER_TREE_VIEW_H
