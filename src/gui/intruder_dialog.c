/**
 * @file src/gui/intruder_dialog.c
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

#include "../include/main_gui.h"
#include "../include/app_state.h"

enum {
  ATTACK_SNIPER = 0,
  ATTACK_PITCHFORK = 1,
};

static GtkWidget *hex_editor;
static GtkWidget *tree_view;
static GtkWidget *entry_add;
static GtkWidget *combo_attack;
static GtkWidget *attack_stack;
static GtkWidget *payload_stack;
static GtkListStore *list_store;
static GtkWidget *intruder_window = NULL;

proto_packet_t *user_packet;

static void intruder_gui_on_reset(void) {
  user_packet = intruder_get_packet_data();

  intruder_gui_hexeditor_update(user_packet->buffer, user_packet->length);
}
static void intruder_gui_on_copy_token(void) {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text(clipboard, FUZZER_TOKEN, -1);
}

static void intruder_gui_on_send(void) {
  const int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_attack));
  if (index == ATTACK_SNIPER){}else{}
  // app_state_transmit_packet();
}

static void intruder_payloads_paste_from_clipboard(GtkListStore *list_store) {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gchar *payload_list = gtk_clipboard_wait_for_text(clipboard);

  if (payload_list == NULL) {
    g_print("Empty or it's not a text");
    return;
  }

  gchar **lines = g_strsplit_set(payload_list, "\r\n", -1);
  for (int i = 0; lines[i] != NULL; i++) {
    gchar *line = g_strstrip(lines[i]);

    if (strlen(line) == 0){ continue;}

    GtkTreeIter iter;
    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter, 0, line, -1);
  }

  g_strfreev(lines);
  g_free(payload_list);
}

static void intruder_gui_on_payload_paste(GtkButton *button, gpointer user_data) {
  (void)button;
  GtkListStore *store = GTK_LIST_STORE(user_data);
  intruder_payloads_paste_from_clipboard(store);
}

static void intruder_gui_on_payload_remove(GtkButton *button, gpointer user_data) {
  (void)button;
  GtkListStore *store = GTK_LIST_STORE(user_data);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gtk_list_store_remove(store, &iter);
  }
}

static void intruder_gui_on_payload_load_from_file(GtkButton *button, gpointer user_data) {
  (void)button;
  GtkListStore *store = GTK_LIST_STORE(user_data);
  GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Payload File",
    GTK_WINDOW(intruder_window),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    GIOChannel *channel = g_io_channel_new_file(filename, "r", NULL);

    if (channel) {
      gchar *line = NULL;
      GError *error = NULL;
      while (g_io_channel_read_line(channel, &line, NULL, NULL, &error) == G_IO_STATUS_NORMAL) {
        const gchar *stripped = g_strstrip(line);
        if (strlen(stripped) > 0) {
          GtkTreeIter iter;
          gtk_list_store_append(store, &iter);
          gtk_list_store_set(store, &iter, 0, stripped, -1);
        }
        g_free(line);
      }
      if (error) g_error_free(error);
      g_io_channel_unref(channel);
    }
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
}

static void intruder_gui_on_payload_clear(GtkWidget *button, gpointer user_data) {
  (void)button;
  GtkListStore *store = GTK_LIST_STORE(user_data);

  GtkWidget *dialog = gtk_dialog_new_with_buttons("Are you sure?",
    GTK_WINDOW(intruder_window),
    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
    "_Cancel", GTK_RESPONSE_REJECT,
    "Accept", GTK_RESPONSE_ACCEPT,
    NULL);

  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(intruder_window));
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    gtk_list_store_clear(store);
  }
  gtk_widget_destroy(dialog);
}

static void intruder_gui_on_payload_add(GtkWidget *button, gpointer user_data) {
  (void)button;
  GtkTreeIter iter;
  GtkListStore *store = GTK_LIST_STORE(user_data);

  const gchar *payload = gtk_entry_get_text(GTK_ENTRY(entry_add));
  if (payload) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, payload, -1);
    gtk_entry_set_text(GTK_ENTRY(entry_add), "");
  }
}

static void intruder_gui_payload_create(GtkWidget *parent) {
  GtkWidget *lbl_payloads = gtk_label_new("<b><span size='medium'>Payload Configuration</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_payloads), TRUE);
  gtk_widget_set_halign(lbl_payloads, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(parent), lbl_payloads, FALSE, FALSE, 0);


  GtkWidget *payload_main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_vexpand(payload_main_hbox, TRUE);

  GtkWidget *payload_btn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *btn_payload_paste = gtk_button_new_with_label("Paste");
  GtkWidget *btn_payload_load = gtk_button_new_with_label("Load...");
  GtkWidget *btn_payload_remove = gtk_button_new_with_label("Remove");
  GtkWidget *btn_payload_clear = gtk_button_new_with_label("Clear");
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), btn_payload_paste, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), btn_payload_load, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), btn_payload_remove, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), btn_payload_clear, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(payload_main_hbox), payload_btn_vbox, FALSE, FALSE, 0);

  GtkWidget *scroll_list = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_list), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll_list, TRUE);

  list_store = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree_view), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
  g_object_unref(list_store);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Payload Item", renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

  gtk_container_add(GTK_CONTAINER(scroll_list), tree_view);
  gtk_box_pack_start(GTK_BOX(payload_main_hbox), scroll_list, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(parent), payload_main_hbox, TRUE, TRUE, 0);

  GtkWidget *add_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  entry_add = gtk_entry_new();
  gtk_widget_set_hexpand(entry_add, TRUE);
  GtkWidget *btn_add = gtk_button_new_with_label("Add");

  gtk_box_pack_start(GTK_BOX(add_hbox), entry_add, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(add_hbox), btn_add, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(parent), add_hbox, FALSE, FALSE, 0);

  g_signal_connect(btn_payload_paste, "clicked", G_CALLBACK(intruder_gui_on_payload_paste), list_store);
  g_signal_connect(btn_payload_load, "clicked", G_CALLBACK(intruder_gui_on_payload_load_from_file), list_store);
  g_signal_connect(btn_payload_remove, "clicked", G_CALLBACK(intruder_gui_on_payload_remove), list_store);
  g_signal_connect(btn_payload_clear, "clicked", G_CALLBACK(intruder_gui_on_payload_clear), list_store);
  g_signal_connect(btn_add, "clicked", G_CALLBACK(intruder_gui_on_payload_add), list_store);
}

static void on_payload_type_changed(GtkComboBox *combo, gpointer data) {
    const gchar *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    if (id) {
        gtk_stack_set_visible_child_name(GTK_STACK(payload_stack), id);
    }
}

GtkWidget *create_pitchfork_config_ui(void) {
  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 10);

  GtkWidget *lbl_title = gtk_label_new("<b><span size='large'>Payloads</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_title), TRUE);
  gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(main_vbox), lbl_title, FALSE, FALSE, 0);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
  gtk_box_pack_start(GTK_BOX(main_vbox), grid, FALSE, FALSE, 5);

  int row = 0;

  GtkWidget *lbl_set = gtk_label_new("Payload set:");
  gtk_widget_set_halign(lbl_set, GTK_ALIGN_START);
  GtkAdjustment *adj_set = gtk_adjustment_new(1, 1, 5, 1, 1, 0);
  GtkWidget *spin_set = gtk_spin_button_new(adj_set, 1, 0);

  gtk_grid_attach(GTK_GRID(grid), lbl_set, 0, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), spin_set, 1, row, 1, 1);
  row++;

  GtkWidget *lbl_type = gtk_label_new("Payload type:");
  gtk_widget_set_halign(lbl_type, GTK_ALIGN_START);
  GtkWidget *combo_type = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_type), "list", "Simple List");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_type), "numbers", "Numbers");
  gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_type), "list");

  gtk_grid_attach(GTK_GRID(grid), lbl_type, 0, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_type, 1, row, 1, 1);

  gtk_box_pack_start(GTK_BOX(main_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  payload_stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(payload_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_box_pack_start(GTK_BOX(main_vbox), payload_stack, TRUE, TRUE, 0);

  GtkWidget *list_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_stack_add_named(GTK_STACK(payload_stack), list_vbox, "list");
  /* Payloads */
  intruder_gui_payload_create(list_vbox);

  GtkWidget *num_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(num_grid), 8);
  gtk_grid_set_column_spacing(GTK_GRID(num_grid), 15);
  gtk_container_set_border_width(GTK_CONTAINER(num_grid), 10);

  const char *labels[] = {"From:", "To:", "Step:"};
  for(int i = 0; i < 3; i++) {
    GtkWidget *lbl = gtk_label_new(labels[i]);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    GtkAdjustment *adj = gtk_adjustment_new(0, 0, 1000000, 1, 10, 0);
    GtkWidget *spin = gtk_spin_button_new(adj, 1, 0);
    gtk_widget_set_hexpand(spin, TRUE);
    gtk_grid_attach(GTK_GRID(num_grid), lbl, 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(num_grid), spin, 1, i, 1, 1);
  }
  gtk_stack_add_named(GTK_STACK(payload_stack), num_grid, "numbers");

  g_signal_connect(combo_type, "changed", G_CALLBACK(on_payload_type_changed), NULL);

  return main_vbox;
}

static void intruder_gui_layout_left_panel(GtkWidget *split_layout) {
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 10);

  GtkWidget *top_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

  GtkWidget *reset_button = gtk_button_new_with_label("Reset");
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Copy Token %s", FUZZER_TOKEN);
  GtkWidget *copy_token_button = gtk_button_new_with_label(buffer);

  GtkWidget *send_button = gtk_button_new_with_label("Send");
  GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_hexpand(spacer, TRUE);

  gtk_box_pack_start(GTK_BOX(top_hbox), reset_button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(top_hbox), copy_token_button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(top_hbox), spacer, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(top_hbox), send_button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(left_vbox), top_hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(left_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  GtkWidget *label_moded = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_moded), "<b><span size='large'>Packet Base (Hex)</span></b>");
  gtk_widget_set_halign(label_moded, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(left_vbox), label_moded, FALSE, FALSE, 0);

  GtkWidget *scroll_hex = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_hex), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scroll_hex, TRUE);
  gtk_widget_set_hexpand(scroll_hex, TRUE);

  hex_editor = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hex_editor), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hex_editor), GTK_WRAP_WORD);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(hex_editor), TRUE);
  gtk_container_add(GTK_CONTAINER(scroll_hex), hex_editor);

  gtk_box_pack_start(GTK_BOX(left_vbox), scroll_hex, TRUE, TRUE, 0);

  gtk_paned_pack1(GTK_PANED(split_layout), left_vbox, TRUE, FALSE);

  g_signal_connect(reset_button, "clicked", G_CALLBACK(intruder_gui_on_reset), NULL);
  g_signal_connect(copy_token_button, "clicked", G_CALLBACK(intruder_gui_on_copy_token), NULL);
  g_signal_connect(send_button, "clicked", G_CALLBACK(intruder_gui_on_send), NULL);
}

static void intruder_gui_layout_right_panel(GtkWidget *split_layout) {
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 10);

  GtkWidget *attack_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  combo_attack = gtk_combo_box_text_new();

  GtkWidget *lbl_attack = gtk_label_new("<b><span size='large'>Attacks</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_attack), TRUE);
  gtk_widget_set_halign(lbl_attack, GTK_ALIGN_START);

  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_attack), "Sniper (Manual Token)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_attack), "Pitchfork (Targeted Payload)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_attack), 0);

  gtk_box_pack_start(GTK_BOX(attack_hbox), lbl_attack, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(attack_hbox), combo_attack, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), attack_hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(right_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  gtk_box_pack_start(GTK_BOX(right_vbox), plugin_spp_crafter_create(), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(right_vbox), create_pitchfork_config_ui(), TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), attack_stack, TRUE, TRUE, 0);
  gtk_paned_pack2(GTK_PANED(split_layout), right_vbox, TRUE, FALSE);

  gtk_box_pack_start(GTK_BOX(right_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  plugin_radio_create(right_vbox);

  gtk_paned_pack2(GTK_PANED(split_layout), right_vbox, TRUE, FALSE);
}

static GtkWidget *intruder_gui_layout_create(void) {
  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

  intruder_gui_layout_left_panel(split_layout);
  intruder_gui_layout_right_panel(split_layout);

  return split_layout;
}

void intruder_gui_create(void) {
  if (intruder_window != NULL) {
    gtk_window_present(GTK_WINDOW(intruder_window));
    return;
  }

  intruder_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(intruder_window), "Intruder");
  gtk_window_set_default_size(GTK_WINDOW(intruder_window), (gint)(APPLICATION_MIN_WIDTH * 0.9), (gint)(APPLICATION_MIN_HEIGHT * 0.9));
  gtk_window_set_position(GTK_WINDOW(intruder_window), GTK_WIN_POS_CENTER);

  g_signal_connect(intruder_window, "destroy", G_CALLBACK(intruder_gui_delete_instance), NULL);

  GtkWidget *main_layout = intruder_gui_layout_create();
  gtk_container_add(GTK_CONTAINER(intruder_window), main_layout);

  gtk_widget_set_name(intruder_window, "intruder_window");
  gtk_widget_show_all(intruder_window);

  gtk_paned_set_position(GTK_PANED(main_layout), (gint)(APPLICATION_MIN_WIDTH * 0.9) / 2);
}

GtkWidget *intruder_gui_get_instance(void) { return intruder_window; }

void intruder_gui_delete_instance(void) { intruder_window = NULL; }

void intruder_gui_hexeditor_update(uint8_t *buffer, const int length) {
  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hex_editor));
  gtk_text_buffer_set_text(text_buffer, "", -1);
  const char *hex_string = uint8_buffer_to_hex_string(buffer, length);
  gtk_text_buffer_set_text(text_buffer, hex_string, -1);
}
