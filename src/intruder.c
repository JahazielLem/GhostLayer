/**
 * @file src/intruder.c
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
#include "../../include/plugins.h"

void intruder_gui_inspect_packet(proto_packet_t *packet) {
  (void)packet;

  if (intruder_gui_get_instance() == NULL) {
    intruder_gui_create();
  }

  plugin_spp_parse_packet(packet->buffer, packet->length);
  intruder_gui_hexeditor_update(packet->buffer, packet->length);

  gtk_window_present(GTK_WINDOW(intruder_gui_get_instance()));
}