/**
 * @file src/gui/fuzzer_table.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-11
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */

#include "main_gui.h"

enum { COLUMN_INDEX = 0, COLUMN_TIMESTAMP, COLUMN_PAYLOAD, COLUMN_DATA, COLUMN_LENGTH, COLUMN_COUNT };

static uint16_t packet_count = 0;
static GtkWidget *treeview = NULL;
static GList *list_packet_buffer = NULL;

static void on_menu_copy_hex_data(GtkMenuItem *item, gpointer user_data) {
  GtkWidget *treeview = GTK_WIDGET(user_data);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gint index;
    gtk_tree_model_get(model, &iter, COLUMN_INDEX, &index, -1);
    const proto_packet_t *pkt = g_list_nth_data(list_packet_buffer, index);
    if (pkt) {
      GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      gtk_clipboard_set_text(clipboard, uint8_buffer_to_hex_string(pkt->buffer, pkt->length), -1);
    }
  }
}

static void on_menu_copy_hexdump_data(GtkMenuItem *item, gpointer user_data) {
  GtkWidget *treeview = GTK_WIDGET(user_data);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gint index;
    gtk_tree_model_get(model, &iter, COLUMN_INDEX, &index, -1);
    proto_packet_t *pkt = g_list_nth_data(list_packet_buffer, index);
    if (pkt) {
      GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      GString *hex_string = generate_hexdump(pkt->buffer, pkt->length);
      gtk_clipboard_set_text(clipboard, (char *) hex_string->str, -1);
      g_string_free(hex_string, TRUE);
    }
  }
}

static gboolean packet_viewer_on_button_press(GtkWidget *treeview, GdkEventButton *event, gpointer user_data) {
  if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
    GtkTreeView *tv = GTK_TREE_VIEW(treeview);
    GtkTreePath *path;
    if (gtk_tree_view_get_path_at_pos(tv, (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {

      GtkTreeSelection *selection = gtk_tree_view_get_selection(tv);
      gtk_tree_selection_unselect_all(selection);
      gtk_tree_selection_select_path(selection, path);
      gtk_tree_path_free(path);

      GtkWidget *menu = gtk_menu_new();
      gtk_widget_set_name(menu, "table_context_menu");

      GtkWidget *menu_copy_hex = gtk_menu_item_new_with_label("Copy Hex Data");
      GtkWidget *menu_copy_hexdump = gtk_menu_item_new_with_label("Copy Hexdump Data");

      gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_copy_hex);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_copy_hexdump);

      g_signal_connect(menu_copy_hex, "activate", G_CALLBACK(on_menu_copy_hex_data), treeview);
      g_signal_connect(menu_copy_hexdump, "activate", G_CALLBACK(on_menu_copy_hexdump_data), treeview);

      gtk_widget_show_all(menu);
      gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *) event);

      return TRUE;
    }
  }
  return FALSE;
}

void fuzzer_gui_packet_viewer_cleanup(void) {
  g_list_free_full(list_packet_buffer, g_free);
  list_packet_buffer = NULL;
  packet_count = 0;

  if (treeview != NULL) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    gtk_list_store_clear(store);
  }
}


GtkWidget *fuzzer_gui_packet_viewer(void) {
  GtkListStore *store = gtk_list_store_new(COLUMN_COUNT,
                                           G_TYPE_INT, // Packet Number
                                           G_TYPE_STRING, // Timestamp
                                           G_TYPE_STRING, // Payload
                                           G_TYPE_STRING, // Hex
                                           G_TYPE_INT // Length
  );
  treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  gtk_widget_set_vexpand(treeview, TRUE);
  gtk_widget_set_hexpand(treeview, TRUE);

  g_object_unref(store);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("No", renderer, "text", COLUMN_INDEX, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Timestamp", renderer, "text", COLUMN_TIMESTAMP, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Payload", renderer, "text", COLUMN_PAYLOAD, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Data (HEX)", renderer, "text", COLUMN_DATA, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Bytes", renderer, "text", COLUMN_LENGTH, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  g_signal_connect(treeview, "button-press-event", G_CALLBACK(packet_viewer_on_button_press), NULL);

  gtk_widget_set_name(treeview, "packet_viewer_window");

  return treeview;
}

void fuzzer_gui_packet_viewer_add_numeric_payload(uint32_t payload, uint8_t *buffer, int length) {
  GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
  GtkTreeIter iter;

  proto_packet_t *pkt = g_new0(proto_packet_t, 1);
  memcpy(pkt->buffer, buffer, length);
  pkt->length = length;

  list_packet_buffer = g_list_append(list_packet_buffer, pkt);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
    COLUMN_INDEX, packet_count++,
    COLUMN_TIMESTAMP, get_timestamp_str(),
    COLUMN_PAYLOAD, g_strdup_printf("0x%04X", payload),
    COLUMN_DATA, uint8_buffer_to_hex_string(buffer, length),
    COLUMN_LENGTH, length, -1);

  GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, NULL, FALSE, 0, 0);
  gtk_tree_path_free(path);
}

void fuzzer_gui_packet_viewer_add_list_payload(char *payload, uint8_t *buffer, int length) {
  GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
  GtkTreeIter iter;

  proto_packet_t *pkt = g_new0(proto_packet_t, 1);
  memcpy(pkt->buffer, buffer, length);
  pkt->length = length;

  list_packet_buffer = g_list_append(list_packet_buffer, pkt);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
    COLUMN_INDEX, packet_count++,
    COLUMN_TIMESTAMP, get_timestamp_str(),
    COLUMN_PAYLOAD, payload,
    COLUMN_DATA, uint8_buffer_to_hex_string(buffer, length),
    COLUMN_LENGTH, length, -1);

  GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, NULL, FALSE, 0, 0);
  gtk_tree_path_free(path);
}
