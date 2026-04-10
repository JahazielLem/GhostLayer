/**
 * @file src/plugins/spp_crafter.c
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
#include "../include/plugins.h"
#include <ccsds.h>

GtkWidget *entry_spp_apid;
GtkWidget *combo_type;
GtkWidget *combo_seq_flag;

void plugin_spp_parse_packet(uint8_t *buffer, int length) {
  space_packet_t space_packet;
  if (spp_unpack_packet(&space_packet, buffer, length) == SPP_ERROR_NONE) {
    char text[24];
    snprintf(text, sizeof(text), "%u", spp_get_apid(&space_packet));
    gtk_entry_set_text(GTK_ENTRY(entry_spp_apid), text);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), spp_get_type(&space_packet));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_seq_flag), spp_get_sequence_flags(&space_packet));
  }
}

void plugin_spp_crafter_create(GtkWidget *parent) {
  GtkWidget *grid = gtk_grid_new();

  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

  gtk_widget_set_margin_top(grid, 15);
  gtk_widget_set_margin_bottom(grid, 15);
  gtk_widget_set_margin_start(grid, 15);
  gtk_widget_set_margin_end(grid, 15);

  int current_row = 0;

  GtkWidget *lbl_spp_apid = gtk_label_new("<b><span size='large'>APID</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_spp_apid), TRUE);
  gtk_widget_set_halign(lbl_spp_apid, GTK_ALIGN_START);

  entry_spp_apid = gtk_entry_new();
  gtk_widget_set_hexpand(entry_spp_apid, TRUE);

  gtk_grid_attach(GTK_GRID(grid), lbl_spp_apid, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), entry_spp_apid, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_type = gtk_label_new("<b><span size='large'>Type</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_type), TRUE);
  gtk_widget_set_halign(lbl_type, GTK_ALIGN_START);

  combo_type = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_type), NULL, "TM");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_type), NULL, "TC");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0);

  gtk_grid_attach(GTK_GRID(grid), lbl_type, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_type, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_seq_flag = gtk_label_new("<b><span size='large'>Sequence Flag</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_seq_flag), TRUE);
  gtk_widget_set_halign(lbl_seq_flag, GTK_ALIGN_START);

  combo_seq_flag = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_seq_flag), NULL, "Continuation (0x0)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_seq_flag), NULL, "Start (0x1)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_seq_flag), NULL, "End (0x2)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_seq_flag), NULL, "Unsegmented (0x3)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_seq_flag), 3);

  gtk_grid_attach(GTK_GRID(grid), lbl_seq_flag, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_seq_flag, 1, current_row, 1, 1);

  gtk_box_pack_start(GTK_BOX(parent), grid, FALSE, FALSE, 0);
}