/**
 * @file src/gui/alerts.c
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
#include "alerts.h"

static GtkWidget *infobar_container;
static GtkWidget *infobar_label;

void alert_status_update(GtkStatusbar *statusbar, const char *message) {
  const guint context_id = gtk_statusbar_get_context_id(statusbar, "status_log");
  gtk_statusbar_push(statusbar, context_id, message);
}

GtkWidget* alert_infobar_create(void) {
  GtkWidget *infobar = gtk_info_bar_new();
  gtk_info_bar_set_show_close_button(GTK_INFO_BAR(infobar), TRUE);

  infobar_label = gtk_label_new("");
  GtkWidget *content = gtk_info_bar_get_content_area(GTK_INFO_BAR(infobar));
  gtk_container_add(GTK_CONTAINER(content), infobar_label);

  g_signal_connect(infobar, "response", G_CALLBACK(gtk_widget_hide), NULL);

  return infobar;
}

void alert_notify_infobar(alert_level_t level, const char *message) {
  GtkMessageType type;
  switch (level) {
    case ALERT_SUCCESS: type = GTK_MESSAGE_INFO; break;
    case ALERT_WARNING: type = GTK_MESSAGE_WARNING; break;
    case ALERT_ERROR:   type = GTK_MESSAGE_ERROR;   break;
    default:            type = GTK_MESSAGE_INFO;    break;
  }

  gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar_container), type);
  gtk_label_set_text(GTK_LABEL(infobar_label), message);
  gtk_widget_show_all(infobar_container);
}

void alert_show_dialog(GtkWindow *parent, alert_level_t level, const char *title, const char *message) {
  GtkMessageType type;
  switch (level) {
    case ALERT_ERROR: type = GTK_MESSAGE_ERROR; break;
    case ALERT_WARNING: type = GTK_MESSAGE_WARNING; break;
    case ALERT_SUCCESS: type = GTK_MESSAGE_INFO; break;
    default: type = GTK_MESSAGE_INFO; break;
  }

  GtkWidget *dialog = gtk_message_dialog_new(parent,
    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
    type,
    GTK_BUTTONS_OK,
    "%s", title);

  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
