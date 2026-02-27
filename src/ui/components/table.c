/* src/ui/components - table.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "table.h"

enum {
  COLUMN_INDEX = 0,
  COLUMN_TIMESTAMP,
  COLUMN_SOURCE,
  COLUMN_DESTINATION,
  COLUMN_LENGTH,
  COLUMN_PROTOCOL,
  COLUMN_BUFFER_PTR,
  COLUMN_COUNT
};

typedef struct {
  GtkWidget *treeview;
  GtkWidget *filter;
  uint16_t count;
  GList *list;
  uint8_t scroll;
} table_context_t;

static table_context_t table_context;

static char *get_timestamp_str(void) {
  static char timestamp[16];
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm *tm_info = localtime(&ts.tv_sec);

  snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d.%02ld", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           ts.tv_nsec / 1000);
  return timestamp;
}


GtkWidget *main_layout_table_init(void) {
  table_context.count = 0;
  table_context.scroll = 1;
  GtkListStore *store = gtk_list_store_new(COLUMN_COUNT,
   G_TYPE_INT, // Packet Number
   G_TYPE_STRING, // Timestamp
   G_TYPE_STRING, // Source
   G_TYPE_STRING, // Destination
   G_TYPE_INT, // Length
   G_TYPE_STRING, // Protocol
   G_TYPE_POINTER // Raw buffer
  );
  table_context.treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  gtk_widget_set_vexpand(table_context.treeview, TRUE);
  gtk_widget_set_hexpand(table_context.treeview, TRUE);

  g_object_unref(store);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("No", renderer, "text", COLUMN_INDEX, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Timestamp", renderer, "text", COLUMN_TIMESTAMP, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Source", renderer, "text", COLUMN_SOURCE, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Destination", renderer, "text", COLUMN_DESTINATION, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Length", renderer, "text", COLUMN_LENGTH, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Protocol", renderer, "text", COLUMN_PROTOCOL, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(table_context.treeview), column);

  // GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(table_context.treeview));
  // gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  return table_context.treeview;
}

void table_add_item(GLPacket *packet) {
  GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(table_context.treeview)));
  GtkTreeIter iter;

  table_context.list = g_list_append(table_context.list, packet);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
      COLUMN_INDEX, table_context.count++,
      COLUMN_TIMESTAMP, get_timestamp_str(),
      COLUMN_SOURCE, packet->summary_source,
      COLUMN_DESTINATION, packet->summary_dest,
      COLUMN_LENGTH, packet->length,
      COLUMN_PROTOCOL, packet->summary_protocol,
      COLUMN_BUFFER_PTR, packet,
      -1);

  if (table_context.scroll == 1) {
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(table_context.treeview), path, NULL, FALSE, 0, 0);
    gtk_tree_path_free(path);
  }
}

uint16_t table_get_item_count(void) { return table_context.count; }

void table_switch_scroll(void) { table_context.scroll = !table_context.scroll; }