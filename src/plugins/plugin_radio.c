/**
 * @file src/plugins/plugin_radio.c
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
#include <arpa/inet.h>

GtkWidget *spin_delay;
GtkWidget *spin_frequency;
GtkWidget *combo_bandwidth;
GtkWidget *combo_spread_factor;

uint32_t plugin_radio_get_frequency(void) {
  const double frequency = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_frequency));
  return (uint32_t)(frequency * 100.0f);
}

uint16_t plugin_radio_get_bandwidth(void) {
  gchar *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_bandwidth));
  if (!text) return 0;
  char *end;
  const float bandwidth = strtof(text, &end);
  g_free(text);
  return (uint16_t)(bandwidth * 100.0f);
}

uint16_t plugin_radio_get_spread_factor(void) {
  gchar *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_spread_factor));
  if (!text) return 0;
  char *end;
  const long sf = strtol(text, &end, 10);
  g_free(text);
  return (uint16_t)sf;
}

gint plugin_radio_get_delay(void) {
  return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_delay));
}

void plugin_radio_create(GtkWidget *parent) {
  GtkWidget *protocol_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

  GtkWidget *lbl_protocol = gtk_label_new("<b><span size='large'>Radio Transmission Configuration</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_protocol), TRUE);
  gtk_widget_set_halign(lbl_protocol, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(protocol_hbox), lbl_protocol, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(parent), protocol_hbox, FALSE, FALSE, 0);

  GtkWidget *grid = gtk_grid_new();

  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

  gtk_widget_set_margin_top(grid, 15);
  gtk_widget_set_margin_bottom(grid, 15);
  gtk_widget_set_margin_start(grid, 15);
  gtk_widget_set_margin_end(grid, 15);

  int current_row = 0;

  GtkWidget *lbl_frequency = gtk_label_new("<b><span size='large'>Frequency (MHz)</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_frequency), TRUE);
  gtk_widget_set_halign(lbl_frequency, GTK_ALIGN_START);

  GtkAdjustment *adj_freq = gtk_adjustment_new(916.0, 70.0, 1000.0, 1, 1.0, 0.0);
  spin_frequency = gtk_spin_button_new(adj_freq, 1.0, 2);
  gtk_widget_set_hexpand(spin_frequency, TRUE);

  gtk_grid_attach(GTK_GRID(grid), lbl_frequency, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), spin_frequency, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_bandwidth = gtk_label_new("<b><span size='large'>Bandwidth (kHz)</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_bandwidth), TRUE);
  gtk_widget_set_halign(lbl_bandwidth, GTK_ALIGN_START);

  combo_bandwidth = gtk_combo_box_text_new();

  gtk_grid_attach(GTK_GRID(grid), lbl_bandwidth, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_bandwidth, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_spread_factor = gtk_label_new("<b><span size='large'>Spread Factor</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_spread_factor), TRUE);
  gtk_widget_set_halign(lbl_spread_factor, GTK_ALIGN_START);

  combo_spread_factor = gtk_combo_box_text_new();

  gtk_grid_attach(GTK_GRID(grid), lbl_spread_factor, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_spread_factor, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_delay = gtk_label_new("<b><span size='large'>Delay (seconds)</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_delay), TRUE);
  gtk_widget_set_halign(lbl_delay, GTK_ALIGN_START);

  GtkAdjustment *adj_delay = gtk_adjustment_new(5, 0, 100, 1, 1.0, 0.0);
  spin_delay = gtk_spin_button_new(adj_delay, 1.0, 2);
  gtk_widget_set_hexpand(spin_delay, TRUE);

  gtk_grid_attach(GTK_GRID(grid), lbl_delay, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), spin_delay, 1, current_row, 1, 1);
  current_row++;

  gtk_box_pack_start(GTK_BOX(parent), grid, FALSE, FALSE, 0);

  const double bandwidths[] = { 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500 };
  for (size_t i = 0; i < G_N_ELEMENTS(bandwidths); i++) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.02f kHz", bandwidths[i]);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_bandwidth), NULL, buf);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_bandwidth), 8);

  for (int i = 5; i <= 12; i++) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", i);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spread_factor), NULL, buf);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spread_factor), 2);
}
