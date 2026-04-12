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
#include "main_gui.h"
#include "plugins.h"

enum {
  SPP_CHANGED_SIGNAL,
  LAST_SIGNAL
};

GtkWidget *spin_spp_apid;
GtkWidget *spin_spp_counter;
GtkWidget *combo_spp_type;
GtkWidget *combo_spp_sechdr_flag;
GtkWidget *combo_spp_seq_flag;

static space_packet_t space_packet;
static guint spp_signals[LAST_SIGNAL] = { 0 };

static void plugin_spp_on_change(GtkWidget *widget, gpointer user_data) {
  (void)widget;
  GtkWidget *plugin_main_container = GTK_WIDGET(user_data);
  g_signal_emit_by_name(plugin_main_container, "spp-data-changed");
}

void plugin_spp_parse_packet(uint8_t *buffer, int length) {
  if (spp_unpack_packet(&space_packet, buffer, length) == SPP_ERROR_NONE) {
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_spp_apid), spp_get_apid(&space_packet));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_spp_counter), spp_get_sequence_count(&space_packet));

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spp_type), spp_get_type(&space_packet));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spp_seq_flag), spp_get_sequence_flags(&space_packet));
  }
}

space_packet_t *plugin_spp_build_packet(uint8_t*buffer, uint16_t length) {
  memset(&space_packet, 0, sizeof(space_packet_t));
  spp_apid_context_t counter = {0};

  const uint16_t apid = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_spp_apid));
  const uint8_t type = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_type));
  const uint8_t sec_hdr = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_sechdr_flag));
  const uint8_t seq_flag = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_seq_flag));
  const uint16_t seq_counter = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_spp_counter));

  uint8_t dummy_payload[1] = {0x00};
  uint8_t *ptr = (buffer != NULL && length > 0) ? buffer : dummy_payload;
  const uint16_t len = (buffer != NULL && length > 0) ? length : 1;

  counter.apid = apid;
  // TODO: Add sec header logic
  if (type == SPP_PTYPE_TM) {
    counter.tm = seq_counter;
    spp_tm_build_packet(&space_packet, seq_flag, sec_hdr, 0, ptr, len, &counter);
  }else {
    counter.tc = seq_counter;
    spp_tc_build_packet(&space_packet, seq_flag, sec_hdr, 0, ptr, len, &counter);
  }
  return &space_packet;
}

GtkWidget *plugin_spp_crafter_create(void) {
  GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  if (spp_signals[SPP_CHANGED_SIGNAL] == 0) {
    spp_signals[SPP_CHANGED_SIGNAL] = g_signal_new("spp-data-changed",
        G_TYPE_FROM_CLASS(G_OBJECT_GET_CLASS(main_container)),
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0);
  }


  GtkWidget *protocol_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
  GtkWidget *lbl_protocol = gtk_label_new("<b><span size='large'>Protocol Configuration</span></b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_protocol), TRUE);
  gtk_widget_set_halign(lbl_protocol, GTK_ALIGN_START);

  gtk_box_pack_start(GTK_BOX(protocol_hbox), lbl_protocol, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_container), protocol_hbox, FALSE, FALSE, 5);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

  gtk_widget_set_margin_top(grid, 10);
  gtk_widget_set_margin_bottom(grid, 10);
  gtk_widget_set_margin_start(grid, 15);
  gtk_widget_set_margin_end(grid, 15);

  int current_row = 0;

  GtkWidget *lbl_spp_apid = gtk_label_new("<b>APID</b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_spp_apid), TRUE);
  gtk_widget_set_halign(lbl_spp_apid, GTK_ALIGN_START);

  GtkAdjustment *adj_apid = gtk_adjustment_new(0, 0, 5000, 1, 1.0, 0.0);
  spin_spp_apid = gtk_spin_button_new(adj_apid, 1.0, 0);
  gtk_widget_set_hexpand(spin_spp_apid, TRUE);

  gtk_grid_attach(GTK_GRID(grid), lbl_spp_apid, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), spin_spp_apid, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_type = gtk_label_new("<b>Type</b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_type), TRUE);
  gtk_widget_set_halign(lbl_type, GTK_ALIGN_START);

  combo_spp_type = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_type), NULL, "TM");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_type), NULL, "TC");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spp_type), 0);
  gtk_grid_attach(GTK_GRID(grid), lbl_type, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_spp_type, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_sechdr_flag = gtk_label_new("<b>Secondary Header Flag</b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_sechdr_flag), TRUE);
  gtk_widget_set_halign(lbl_sechdr_flag, GTK_ALIGN_START);

  combo_spp_sechdr_flag = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_sechdr_flag), NULL, "No Present (0x0)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_sechdr_flag), NULL, "Present (0x1)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spp_sechdr_flag), 0);
  gtk_grid_attach(GTK_GRID(grid), lbl_sechdr_flag, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_spp_sechdr_flag, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_seq_flag = gtk_label_new("<b>Sequence Flag</b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_seq_flag), TRUE);
  gtk_widget_set_halign(lbl_seq_flag, GTK_ALIGN_START);

  combo_spp_seq_flag = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_seq_flag), NULL, "Continuation (0x0)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_seq_flag), NULL, "Start (0x1)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_seq_flag), NULL, "End (0x2)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_spp_seq_flag), NULL, "Unsegmented (0x3)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spp_seq_flag), 3);
  gtk_grid_attach(GTK_GRID(grid), lbl_seq_flag, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo_spp_seq_flag, 1, current_row, 1, 1);
  current_row++;

  GtkWidget *lbl_spp_seq_count = gtk_label_new("<b>Sequence Counter</b>");
  gtk_label_set_use_markup(GTK_LABEL(lbl_spp_seq_count), TRUE);
  gtk_widget_set_halign(lbl_spp_seq_count, GTK_ALIGN_START);

  GtkAdjustment *adj_seq_counter = gtk_adjustment_new(0, 0, 1000000, 1, 1.0, 0.0);
  spin_spp_counter = gtk_spin_button_new(adj_seq_counter, 1.0, 0);
  gtk_widget_set_hexpand(spin_spp_counter, TRUE);

  gtk_grid_attach(GTK_GRID(grid), lbl_spp_seq_count, 0, current_row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), spin_spp_counter, 1, current_row, 1, 1);


  gtk_box_pack_start(GTK_BOX(main_container), grid, FALSE, FALSE, 0);

  g_signal_connect(spin_spp_apid, "value-changed", G_CALLBACK(plugin_spp_on_change), main_container);
  g_signal_connect(spin_spp_counter, "value-changed", G_CALLBACK(plugin_spp_on_change), main_container);
  g_signal_connect(combo_spp_type, "changed", G_CALLBACK(plugin_spp_on_change), main_container);
  g_signal_connect(combo_spp_seq_flag, "changed", G_CALLBACK(plugin_spp_on_change), main_container);
  g_signal_connect(combo_spp_sechdr_flag, "changed", G_CALLBACK(plugin_spp_on_change), main_container);

  return main_container;
}

gint plugin_spp_get_apid(void){ return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_spp_apid)); }

gint plugin_spp_get_seq_counter(void){ return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_spp_counter)); }

gint plugin_spp_get_type(void){ return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_type)); }

gint plugin_spp_get_sechdr(void){ return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_sechdr_flag)); }

gint plugin_spp_get_seq_flag(void){ return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_spp_seq_flag)); }
