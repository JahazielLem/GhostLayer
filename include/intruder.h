/**
 * @file include/intruder.h
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
#ifndef GHOSTLAYER_INTRUDER_H
#define GHOSTLAYER_INTRUDER_H

/* Intruder Control */
void intruder_inspect_packet(proto_packet_t *packet);
proto_packet_t *intruder_get_packet_data(void);

#endif //GHOSTLAYER_INTRUDER_H
