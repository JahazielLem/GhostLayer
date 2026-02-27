/* src/ui/components - toolbar.c
 *
 * GhostLayer - By astrobyte 25/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include "toolbar.h"
#include "table.h"

typedef struct {
  GtkToolItem *iface;
  GtkToolItem *start_btn;
  GtkToolItem *stop_btn;
  GtkToolItem *scroll_btn;
  gboolean is_connected;
} toolbar_iface_context_t;

static toolbar_iface_context_t iface_context;


static void toolbar_on_scroll(GtkWidget *widget, gpointer user_data) {
  (void)widget;
  (void)user_data;
  table_switch_scroll();
}


GtkWidget * main_layout_toolbar_init(GtkWidget *widget, gpointer user_data) {
  GtkWidget *window = (GtkWidget *) user_data;
  GtkWidget *toolbar = gtk_toolbar_new();

  iface_context.iface = gtk_tool_button_new(NULL, "iFace");
  iface_context.start_btn = gtk_tool_button_new(NULL, "StartIFace");
  iface_context.stop_btn = gtk_tool_button_new(NULL, "StopIFace");
  iface_context.scroll_btn = gtk_tool_button_new(NULL, "Scroll");

  gtk_widget_set_tooltip_text(GTK_WIDGET(iface_context.iface), "iFace Menu");
  gtk_widget_set_tooltip_text(GTK_WIDGET(iface_context.start_btn), "Start Capture");
  gtk_widget_set_tooltip_text(GTK_WIDGET(iface_context.stop_btn), "Stop Capture");
  gtk_widget_set_tooltip_text(GTK_WIDGET(iface_context.scroll_btn), "Enable/Disable Scrolling");

  g_signal_connect(iface_context.scroll_btn, "clicked", G_CALLBACK(toolbar_on_scroll), window);

  gtk_widget_set_sensitive(GTK_WIDGET(iface_context.start_btn), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(iface_context.stop_btn), FALSE);

  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(iface_context.iface), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(iface_context.start_btn), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(iface_context.stop_btn), -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(iface_context.scroll_btn), -1);

  return toolbar;
}
