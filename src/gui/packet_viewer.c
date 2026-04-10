/**
 * @file src/gui/packet_viewer.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-09
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */

#include "../../include/main_gui.h"

enum {
  COLUMN_INDEX = 0,
  COLUMN_TIMESTAMP,
  COLUMN_LENGTH,
  COLUMN_PROTOCOL,
  COLUMN_INFORMATION,
  COLUMN_BUFFER_PTR,
  COLUMN_COUNT
};

static uint16_t packet_count = 0;
static GtkWidget *gtk_treeview = NULL;
static GList *list_packet_buffer = NULL;

static packet_viewer_on_select_cb packet_viewer_selected_cb = NULL;

static void packet_viewer_on_row_select(GtkTreeSelection *selection, gpointer user_data) {
  (void)user_data;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gint index;
    gtk_tree_model_get(model, &iter, COLUMN_INDEX, &index, -1);
    proto_packet_t *pkt = g_list_nth_data(list_packet_buffer, index);
    if (pkt) {
      packet_viewer_selected_cb(pkt);
    }
  }
}

static void on_send_to_intruder(GtkMenuItem *item, gpointer user_data) {
  GtkWidget *treeview = GTK_WIDGET(user_data);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gint index;
    gtk_tree_model_get(model, &iter, COLUMN_INDEX, &index, -1);
    proto_packet_t *pkt = g_list_nth_data(list_packet_buffer, index);
    if (pkt) {
      g_print("Proto\n");
      intruder_gui_inspect_packet(pkt);
    }
  }
}

static void on_menu_copy_hex_data(GtkMenuItem *item, gpointer user_data) {
  (void)item;
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
      gtk_clipboard_set_text(clipboard, uint8_buffer_to_hex_string_separator(pkt->buffer, pkt->length, " "), -1);
    }
  }
}

static void on_menu_copy_hexdump_data(GtkMenuItem *item, gpointer user_data) {
  (void)item;
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
  (void)user_data;
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
      GtkWidget *separator = gtk_separator_menu_item_new();
      GtkWidget *menu_intruder = gtk_menu_item_new_with_label("Send to Intruder");

      gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_copy_hex);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_copy_hexdump);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_intruder);

      g_signal_connect(menu_copy_hex, "activate", G_CALLBACK(on_menu_copy_hex_data), treeview);
      g_signal_connect(menu_copy_hexdump, "activate", G_CALLBACK(on_menu_copy_hexdump_data), treeview);
      g_signal_connect(menu_intruder, "activate", G_CALLBACK(on_send_to_intruder), treeview);

      gtk_widget_show_all(menu);
      gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *) event);

      return TRUE;
    }
  }
  return FALSE;
}

void packet_viewer_register_select_cb(packet_viewer_on_select_cb select_cb) { packet_viewer_selected_cb = select_cb; }

GtkWidget *packet_viewer_create(void) {
  GtkListStore *store = gtk_list_store_new(COLUMN_COUNT,
                                           G_TYPE_INT,    // Packet Number
                                           G_TYPE_STRING, // Timestamp
                                           G_TYPE_INT,    // Length
                                           G_TYPE_STRING, // Protocol
                                           G_TYPE_STRING, // Information
                                           G_TYPE_POINTER // Raw buffer
  );

  gtk_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  gtk_widget_set_vexpand(gtk_treeview, TRUE);
  gtk_widget_set_hexpand(gtk_treeview, TRUE);
  gtk_widget_set_name(gtk_treeview, "packet_viewer");

  g_object_unref(store);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("No", renderer, "text", COLUMN_INDEX, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(gtk_treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Timestamp", renderer, "text", COLUMN_TIMESTAMP, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(gtk_treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Length", renderer, "text", COLUMN_LENGTH, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(gtk_treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Protocol", renderer, "text", COLUMN_PROTOCOL, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(gtk_treeview), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Information", renderer, "text", COLUMN_INFORMATION, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(gtk_treeview), column);

  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gtk_treeview));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  g_signal_connect(selection, "changed", G_CALLBACK(packet_viewer_on_row_select), NULL);
  g_signal_connect(gtk_treeview, "button-press-event", G_CALLBACK(packet_viewer_on_button_press), NULL);

  return gtk_treeview;
}

void packet_viewer_add(const char *protocol, const char *information, const uint8_t *buffer, int length) {
  GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gtk_treeview)));
  GtkTreeIter iter;

  proto_packet_t *pkt = g_new0(proto_packet_t, 1);
  memcpy(pkt->buffer, buffer, length);
  pkt->length = length;

  list_packet_buffer = g_list_append(list_packet_buffer, pkt);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, COLUMN_INDEX, packet_count++, COLUMN_TIMESTAMP, get_timestamp_str(),
                     COLUMN_LENGTH, length, COLUMN_PROTOCOL, protocol,
                     COLUMN_INFORMATION, information, COLUMN_BUFFER_PTR, pkt, -1);

  GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(gtk_treeview), path, NULL, FALSE, 0, 0);
  gtk_tree_path_free(path);
}

uint16_t packet_viewer_get_count(void) { return packet_count; }

GList *packet_viewer_get_packet_list(void) { return list_packet_buffer; }

/* @brief Clear records */
void packet_viewer_clear(void) {
  g_list_free_full(list_packet_buffer, g_free);
  list_packet_buffer = NULL;
  packet_count = 0;

  if (gtk_treeview != NULL) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gtk_treeview)));
    gtk_list_store_clear(store);
  }
}
