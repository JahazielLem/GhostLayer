/**
 * @file include/dissectors.h
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
#ifndef GHOSTLAYER_DISSECTORS_H
#define GHOSTLAYER_DISSECTORS_H

#include "proto.h"

proto_module_t *dissector_register_lora(void);
proto_module_t *dissector_register_spp(void);

#endif //GHOSTLAYER_DISSECTORS_H
