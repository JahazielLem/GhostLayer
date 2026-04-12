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
#include "../../include/bridge.h"

proto_packet_t *context_packet;

/**@brief Iterates through the Application Process Identifier (APID) range to map active services and identify the logical architecture of the satellite bus.
*/
static void intruder_discovery_attack(void) {
  const gint from   = intruder_gui_get_range_from(ATTACK_SERVICE_DISCOVERY);
  const gint to     = intruder_gui_get_range_to(ATTACK_SERVICE_DISCOVERY);
  const gint steps  = intruder_gui_get_range_steps(ATTACK_SERVICE_DISCOVERY);
  const gint radio_delay = plugin_radio_get_delay();

  g_print("[Discovery ATTACK] Running from %d to %d steps (%d)\n", from, to, steps);
  for (gint i = from; i <= to; i += steps) {
    g_print("Running Attack (%d)\n", i);
    sleep(radio_delay);
  }
}

void intruder_send_attack(const gint attack) {
  switch (attack) {
    case ATTACK_SERVICE_DISCOVERY:
      intruder_discovery_attack();
      break;
    case ATTACK_SEQ_EXHAUSTION:
    case ATTACK_MANUAL_INJECTION:
    default: break;
  }
}

proto_packet_t *intruder_get_packet_data(void) {
  return context_packet;
}

void intruder_inspect_packet(proto_packet_t *packet) {
  if (intruder_gui_get_instance() == NULL) {
    intruder_gui_create();
  }

  context_packet = g_new0(proto_packet_t, 1);
  memcpy(context_packet->buffer, packet->buffer, packet->length);
  context_packet->length = packet->length;

  plugin_spp_parse_packet(context_packet->buffer, context_packet->length);
  intruder_gui_hexeditor_update(context_packet->buffer, context_packet->length);

  gtk_window_present(GTK_WINDOW(intruder_gui_get_instance()));
}
