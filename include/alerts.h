/**
 * @file include/alerts.h
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-11
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#ifndef GHOSTLAYER_ALERTS_H
#define GHOSTLAYER_ALERTS_H

#include <gtk/gtk.h>

typedef enum {
  ALERT_INFO,
  ALERT_SUCCESS,
  ALERT_WARNING,
  ALERT_ERROR,
} alert_level_t;

void alert_show_dialog(GtkWindow *parent, alert_level_t level, const char *title, const char *message);
GtkWidget* alert_infobar_create(void);
void alert_notify_infobar(alert_level_t level, const char *message);
void alert_status_update(GtkStatusbar *statusbar, const char *message);
#endif //GHOSTLAYER_ALERTS_H
