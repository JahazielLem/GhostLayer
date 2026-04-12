/**
 * @file include/main_gui.h
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
#ifndef GHOSTLAYER_MAIN_GUI_H
#define GHOSTLAYER_MAIN_GUI_H

#include <gtk/gtk.h>
#include "proto.h"
#include "dissectors.h"
#include "intruder.h"
#include "plugins.h"

#define APPLICATION_MAIN_NAME "GhostLayer - IoT Packet Analyzer"
#define APPLICATION_WEBSITE   "https://github.com/JahazielLem/GhostLayer"
#define APPLICATION_COMMENTS  "IoT Packet Analyzer for hackers."

#define APPLICATION_MIN_WIDTH (1280)
#define APPLICATION_MIN_HEIGHT (720)

enum {
  ATTACK_SERVICE_DISCOVERY  = 0, // APID Sweep
  ATTACK_SEQ_EXHAUSTION     = 1, // Counter Fuzzing
  ATTACK_MANUAL_INJECTION   = 2, // Token based
};


typedef void (*packet_viewer_on_select_cb)(proto_packet_t *pkt);

int gui_main(int argc, char *argv[]);

void dissector_parser_register(void);
void register_dissectors(void);

/* GtkWidget */
GtkWidget *statusbar_create(GtkWidget *widget, gpointer data);
GtkWidget *toolbar_create(GtkWidget *widget, gpointer user_data);
GtkWidget *packet_viewer_create(void);
GtkWidget *packet_details_create(void);
GtkWidget *packet_hexdump_create(void);
/* Dialogs */
void about_dialog_create(GtkApplication *app, gpointer user_data);
void iface_dialog_create(GtkWidget *widget, gpointer data);
void packet_sender_dialog_create(GtkWidget *widget, gpointer data);
void intruder_gui_create(void);

/* Handlers */
void statusbar_update_label_connection(gboolean state);
void statusbar_update_label_packet_count(uint16_t packet_count);

GtkWidget *intruder_gui_get_instance(void);
void intruder_gui_delete_instance(void);
void intruder_gui_hexeditor_update(uint8_t *buffer, int length);

gint intruder_gui_get_attack(void);
gint intruder_gui_get_range_from(gint attack_index);
gint intruder_gui_get_range_to(gint attack_index);
gint intruder_gui_get_range_steps(gint attack_index);

void packet_viewer_add(const char *protocol, const char *information, const uint8_t *buffer, int length);
void packet_viewer_register_select_cb(packet_viewer_on_select_cb select_cb);
void packet_viewer_clear(void);
uint16_t packet_viewer_get_count(void);
GList *packet_viewer_get_packet_list(void);

void packet_details_cleanup(void);
void packet_details_clear(void);
void packet_viewer_expand_tree(void);
GtkTreeIter packet_details_add_field(GtkTreeIter *parent, const char *name, const char *value, int start, int end);
GtkTreeIter packet_details_add_bitfield(GtkTreeIter *parent, uint32_t buffer, int total_bytes, bitfield_t *fields,
                                        int num_fields, int start, int end);

void packet_hexdump_apply_hover_tag_start_end(int start_offset, int end_offset);
void packet_hexdump_apply_hover_tag(int field_index);
void packet_hexdump_clear_tag_field(void);
void packet_hexdump_add_field_value(int packet_start, int packet_end);
void packet_hexdump_update(uint8_t *buffer, int length);
#endif //GHOSTLAYER_MAIN_GUI_H
