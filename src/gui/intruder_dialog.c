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

static GtkWidget *hex_editor;
static GtkWidget *intruder_window = NULL;

static void intruder_gui_layout_left_panel(GtkWidget *split_layout) {
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 10);

    GtkWidget *top_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Copy Token %s", FUZZER_TOKEN);
    GtkWidget *copy_token_button = gtk_button_new_with_label(buffer);

    GtkWidget *send_button = gtk_button_new_with_label("Send Attack");
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
}

static void intruder_gui_layout_right_panel(GtkWidget *split_layout) {
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 10);


  GtkWidget *attack_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *combo_attack = gtk_combo_box_text_new();

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

  /* Space Packet Configuration */
  GtkWidget *protocol_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

  GtkWidget *lbl_protocol = gtk_label_new("<b><span size='large'>Protocol Configuration</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_protocol), TRUE);
  gtk_widget_set_halign(lbl_protocol, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(protocol_hbox), lbl_protocol, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), protocol_hbox, TRUE, TRUE, 0);

  plugin_spp_crafter_create(right_vbox);

  gtk_box_pack_start(GTK_BOX(right_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  /* Payloads */
  GtkWidget *lbl_payloads = gtk_label_new("<b><span size='large'>Payloads</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_payloads), TRUE);
  gtk_widget_set_halign(lbl_payloads, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(right_vbox), lbl_payloads, FALSE, FALSE, 0);


  GtkWidget *payload_main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_vexpand(payload_main_hbox, TRUE);

  GtkWidget *payload_btn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), gtk_button_new_with_label("Paste"), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), gtk_button_new_with_label("Load..."), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), gtk_button_new_with_label("Remove"), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), gtk_button_new_with_label("Clear"), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(payload_btn_vbox), gtk_button_new_with_label("Deduplicate"), FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(payload_main_hbox), payload_btn_vbox, FALSE, FALSE, 0);


  GtkWidget *scroll_list = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_list), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scroll_list, TRUE);


  GtkListStore *list_store = gtk_list_store_new(1, G_TYPE_STRING);
  GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
  g_object_unref(list_store);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Payload Item", renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

  gtk_container_add(GTK_CONTAINER(scroll_list), tree_view);
  gtk_box_pack_start(GTK_BOX(payload_main_hbox), scroll_list, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), payload_main_hbox, TRUE, TRUE, 0);


  GtkWidget *add_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget *entry_add = gtk_entry_new();
  gtk_widget_set_hexpand(entry_add, TRUE);
  GtkWidget *btn_add = gtk_button_new_with_label("Add");

  gtk_box_pack_start(GTK_BOX(add_hbox), entry_add, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(add_hbox), btn_add, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), add_hbox, FALSE, FALSE, 0);


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
  gtk_window_set_title(GTK_WINDOW(intruder_window), "SPP Intruder");
  gtk_window_set_default_size(GTK_WINDOW(intruder_window), (gint)(APPLICATION_MIN_WIDTH * 1.2), (gint)(APPLICATION_MIN_HEIGHT * 1.2));
  gtk_window_set_position(GTK_WINDOW(intruder_window), GTK_WIN_POS_CENTER);

  g_signal_connect(intruder_window, "destroy", G_CALLBACK(intruder_gui_delete_instance), NULL);

  GtkWidget *main_layout = intruder_gui_layout_create();
  gtk_container_add(GTK_CONTAINER(intruder_window), main_layout);

  gtk_widget_set_name(intruder_window, "intruder_window");
  gtk_widget_show_all(intruder_window);

  gtk_paned_set_position(GTK_PANED(main_layout), (gint)(APPLICATION_MIN_WIDTH * 1.2) / 2);
}

GtkWidget *intruder_gui_get_instance(void) { return intruder_window; }

void intruder_gui_delete_instance(void) { intruder_window = NULL; }

void intruder_gui_hexeditor_update(uint8_t *buffer, const int length) {
  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hex_editor));
  gtk_text_buffer_set_text(text_buffer, "", -1);
  const char *hex_string = uint8_buffer_to_hex_string(buffer, length);
  gtk_text_buffer_set_text(text_buffer, hex_string, -1);
}