/**
 * @file src/gui/packet_tree.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-10
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */

#include "../../include/main_gui.h"

enum {
  COLUMN_FIELD_NAME,
  COLUMN_FIELD_VALUE,
  COLUMN_BG_COLOR,
  COLUMN_TXT_COLOR,
  COLUMN_START,
  COLUMN_END,
  COLUMN_COUNT,
};
static GtkTreeStore *treestore = NULL;
static GtkWidget *treeview = NULL;

static const char *txt_color = "#11111b";

static void packet_details_on_selection(GtkTreeView *tree, gpointer user_data) {
  (void)user_data;
  GtkTreeSelection *selection = gtk_tree_view_get_selection(tree);
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    int start = -1;
    int end = -1;
    gtk_tree_model_get(model, &iter, COLUMN_START, &start, -1);
    gtk_tree_model_get(model, &iter, COLUMN_END, &end, -1);
    if (start >= 0 && end >= 0) {
      packet_hexdump_apply_hover_tag_start_end(start, end);
    }
  }
}

void packet_details_cleanup(void) {
  if (treestore) {
    g_object_ref(treestore);
    treestore = NULL;
  }
}

/* @brief Clear the records */
void packet_details_clear(void) { gtk_tree_store_clear(treestore); }

void packet_viewer_expand_tree(void) { gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview)); }

GtkWidget *packet_details_create(void) {
  treestore = gtk_tree_store_new(COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
                                 G_TYPE_INT);
  treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(treestore));
  g_object_ref(treestore);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

  GtkTreeViewColumn *col_name =
      gtk_tree_view_column_new_with_attributes("", renderer, "text", COLUMN_FIELD_NAME, "cell-background",
                                               COLUMN_BG_COLOR, "foreground", COLUMN_TXT_COLOR, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_name);

  // GtkTreeViewColumn *col_value =
  //     gtk_tree_view_column_new_with_attributes("", renderer, "text", COLUMN_FIELD_VALUE, "cell-background",
  //                                              COLUMN_BG_COLOR, "foreground", COLUMN_TXT_COLOR, NULL);
  // gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_value);

  gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview));
  g_signal_connect(treeview, "cursor-changed", G_CALLBACK(packet_details_on_selection), NULL);
  return treeview;
}

GtkTreeIter packet_details_add_field(GtkTreeIter *parent, const char *name, const char *value, const int start,
                                     const int end) {
  GtkTreeIter iter;
  const char *bg_color = (parent == NULL) ? "#dce0e8" : "#ccd0da";
  char buffer[512];
  sprintf(buffer, "%s %s", name, value);

  gtk_tree_store_append(treestore, &iter, parent);
  gtk_tree_store_set(treestore, &iter, COLUMN_FIELD_NAME, buffer, COLUMN_BG_COLOR, bg_color, COLUMN_TXT_COLOR,
                     txt_color, COLUMN_START, start, COLUMN_END, end, -1);
  return iter;
}

GtkTreeIter packet_details_add_bitfield(GtkTreeIter *parent, uint32_t buffer, int total_bytes, bitfield_t *fields,
                                        int num_fields, const int start, const int end) {
  GtkTreeIter iter;
  const char *bg_color = (parent == NULL) ? "#dce0e8" : "#ccd0da";
  char *value = bitfield_string(buffer, total_bytes, fields, num_fields);
  if (value == NULL) {
    value = "[Error with bitfield]";
  }
  gtk_tree_store_append(treestore, &iter, parent);
  gtk_tree_store_set(treestore, &iter, COLUMN_FIELD_NAME, value, COLUMN_BG_COLOR, bg_color, COLUMN_TXT_COLOR, txt_color,
                     COLUMN_START, start, COLUMN_END, end, -1);
  return iter;
}
