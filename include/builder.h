/**
 * @file include/builder.h
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-16
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#ifndef GHOSTLAYER_BUILDER_H
#define GHOSTLAYER_BUILDER_H

#include <gtk/gtk.h>

typedef enum {
  GEN_RANGE,
  GEN_LIST,
  GEN_BOF_PATTERN
} generator_type_t;

typedef struct _payload_generator {
  generator_type_t type;
  int current_index;
  int total_items;

  uint8_t* (*get_next)(struct _payload_generator *self, int *out_len);
  void (*reset)(struct _payload_generator *self);

  void *specific_ctx;
} payload_generator_t;

typedef enum {
  STRATEGY_SNIPER,      // Un marcador a la vez
  STRATEGY_FORK,        // Marcadores en paralelo (1 a 1)
  STRATEGY_CLUSTER_BOMB // Todas las combinaciones posibles
} attack_strategy_t;

typedef struct {
  char *marker_name;
  generator_type_t gen_type;
  void *gen_ctx;
} injection_point_t;

typedef struct {
  uint8_t *raw_template;
  size_t template_len;
  GList *injection_points;
  attack_strategy_t strategy;
  int total_iterations;
} intruder_campaign_t;

void generator_gui_create(GtkWindow *parent);

#endif //GHOSTLAYER_BUILDER_H
