/**
 * @file include/plugins.h
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
#ifndef GHOSTLAYER_PLUGINS_H
#define GHOSTLAYER_PLUGINS_H

GtkWidget *plugin_spp_crafter_create(void);
void plugin_spp_parse_packet(uint8_t *buffer, int length);

void plugin_radio_create(GtkWidget *parent);
uint32_t plugin_radio_get_frequency(void);
uint16_t plugin_radio_get_bandwidth(void);
uint16_t plugin_radio_get_spread_factor(void);
#endif //GHOSTLAYER_PLUGINS_H
