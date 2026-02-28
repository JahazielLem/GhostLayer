/* src/ui/components - tree-view.c
 *
 * GhostLayer - By astrobyte 28/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "tree-view.h"

enum {
  COLUMN_FIELD_NAME,
  COLUMN_FIELD_VALUE,
  COLUMN_BG_COLOR,
  COLUMN_TXT_COLOR,
  COLUMN_START,
  COLUMN_END,
  COLUMN_COUNT,
};

typedef struct {
  GtkTreeStore *treestore;
  GtkWidget *treeview;
} treeview_context_t;

static treeview_context_t treeview_context;
static const char *txt_color = "#4c4f69";


void treeview_add_proto_node(GtkTreeIter *parent_iter, GLProtoNode *node) {
  GtkTreeIter iter;
  const char *bg_color = (parent_iter == NULL) ? "#e6e9ef" : "#dce0e8";

  gtk_tree_store_append(treeview_context.treestore, &iter, parent_iter);
  gtk_tree_store_set(treeview_context.treestore, &iter,
    COLUMN_FIELD_NAME, node->label,
    COLUMN_FIELD_VALUE, node->value ? node->value : "",
    COLUMN_BG_COLOR, bg_color,
    COLUMN_TXT_COLOR, txt_color,
    COLUMN_START, node->start,
    COLUMN_END, node->end, -1);

  for (GList *l = node->children; l != NULL; l = l->next) {
    GLProtoNode *child = l->data;
    treeview_add_proto_node(&iter, child);
  }
}

GtkWidget *main_layout_treeview_init(void) {
  treeview_context.treestore = gtk_tree_store_new(COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
                                 G_TYPE_INT);
  treeview_context.treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(treeview_context.treestore));
  g_object_ref(treeview_context.treestore);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

  GtkTreeViewColumn *col_name =
      gtk_tree_view_column_new_with_attributes("", renderer, "text", COLUMN_FIELD_NAME, "cell-background",
                                               COLUMN_BG_COLOR, "foreground", COLUMN_TXT_COLOR, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_context.treeview), col_name);

  gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview_context.treeview));
  // g_signal_connect(treeview_context.treeview, "cursor-changed", G_CALLBACK(packet_details_on_selection), NULL);
  return treeview_context.treeview;
}

void main_layout_treeview_clear_records(void) { gtk_tree_store_clear(treeview_context.treestore); }
void main_layout_treeview_expand(void) { gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview_context.treeview)); }