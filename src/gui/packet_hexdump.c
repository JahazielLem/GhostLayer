/**
 * @file src/gui/packet_hexdump.c
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


#include "main_gui.h"


#define HEXDUMP_DATA_LEN_FORMAT (9)

typedef struct {
  int start;
  int end;
} field_range_t;

static field_range_t field_ranges[128];
static uint16_t field_range_count = 0;

typedef struct {
  GtkWidget *textview;
  GtkTextBuffer *text_buffer;
  GtkTextTag *hover_tag;
} hexdump_context_t;

static hexdump_context_t hexdump_ctx;
static uint16_t buffer_len = 0;

static int packet_hexdump_get_ascii_offset_for_byte(const int byte_index) {
  const int line = byte_index / 16;
  const int col = byte_index % 16;

  const int line_offset = line * 73; // len (offset + hex + space + ascii)
  const int ascii_start = 6 + 48 + 2; // 6 offset + 48 hex + 2 space = 56

  return line_offset + ascii_start + col;
}

static int packet_hexdump_get_text_offset_for_byte(const int byte_index) {
  const int line = byte_index / 16;
  const int col = byte_index % 16;

  // Start of line: 4 (offset) + 2 spaces
  const int base_offset = 6;
  int offset_in_line = col * 3;
  if (col >= 8)
    offset_in_line += 1; // space

  return line * 73 + base_offset + offset_in_line;
}


static unsigned long packet_hexdump_get_index_from_offset(const int offset) {
  for (unsigned long i = 0; i < sizeof(field_ranges) / sizeof(field_ranges[0]); i++) {
    if (offset >= field_ranges[i].start && offset <= field_ranges[i].end) {
      return i;
    }
  }
  return -1;
}

static gboolean packet_hexdump_on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
  (void)user_data;
  int x, y;
  GtkTextIter iter;

  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT, (int) event->x, (int) event->y, &x,
                                        &y);
  gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, x, y);

  const int offset = gtk_text_iter_get_offset(&iter);
  const int byte_index = offset / HEXDUMP_DATA_LEN_FORMAT; // 5 char (offset + space) + 3 (chars + space) + space + ascii

  packet_hexdump_apply_hover_tag((int)packet_hexdump_get_index_from_offset(byte_index));
  return FALSE;
}

void packet_hexdump_apply_hover_tag_start_end(const int start_offset, const int end_offset) {
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(hexdump_ctx.text_buffer, &start);
  gtk_text_buffer_get_end_iter(hexdump_ctx.text_buffer, &end);
  gtk_text_buffer_remove_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &start, &end);

  if (start_offset < 0 || end_offset < 0 || end_offset < start_offset)
    return;

  for (int i = start_offset; i <= end_offset; i++) {
    const int offset = packet_hexdump_get_text_offset_for_byte(i);
    GtkTextIter byte_start, byte_end;

    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &byte_start, offset);
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &byte_end, offset + 3);
    gtk_text_buffer_apply_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &byte_start, &byte_end);

    const int ascii_offset = packet_hexdump_get_ascii_offset_for_byte(i);
    GtkTextIter ascii_start, ascii_end;
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &ascii_start, ascii_offset);
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &ascii_end, ascii_offset + 1);
    gtk_text_buffer_apply_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &ascii_start, &ascii_end);
  }
}


void packet_hexdump_apply_hover_tag(const int field_index) {
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter(hexdump_ctx.text_buffer, &start);
  gtk_text_buffer_get_end_iter(hexdump_ctx.text_buffer, &end);

  gtk_text_buffer_remove_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &start, &end);

  if (field_index < 0)
    return;

  gtk_text_buffer_get_start_iter(hexdump_ctx.text_buffer, &start);
  for (int i = field_ranges[field_index].start; i <= field_ranges[field_index].end; i++) {
    const int offset = packet_hexdump_get_text_offset_for_byte(i);
    GtkTextIter byte_start, byte_end;

    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &byte_start, offset);
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &byte_end, offset + 3);
    gtk_text_buffer_apply_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &byte_start, &byte_end);

    const int ascii_offset = packet_hexdump_get_ascii_offset_for_byte(i);
    GtkTextIter ascii_start, ascii_end;
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &ascii_start, ascii_offset);
    gtk_text_buffer_get_iter_at_offset(hexdump_ctx.text_buffer, &ascii_end, ascii_offset + 1);
    gtk_text_buffer_apply_tag(hexdump_ctx.text_buffer, hexdump_ctx.hover_tag, &ascii_start, &ascii_end);
  }
}

void packet_hexdump_clear_tag_field(void) {
  memset(field_ranges, 0, sizeof(field_ranges));
  field_range_count = 0;
}

void packet_hexdump_add_field_value(const int packet_start, const int packet_end) {
  const field_range_t new_field_range = {.start = packet_start, .end = packet_end};
  field_ranges[field_range_count++] = new_field_range;
}

GtkWidget *packet_hexdump_create(void) {
  hexdump_ctx.textview = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hexdump_ctx.textview), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(hexdump_ctx.textview), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hexdump_ctx.textview), GTK_WRAP_WORD);
  gtk_widget_add_events(hexdump_ctx.textview, GDK_POINTER_MOTION_MASK);
  gtk_widget_set_name(hexdump_ctx.textview, "hexdump_textview");

  g_signal_connect(hexdump_ctx.textview, "motion-notify-event", G_CALLBACK(packet_hexdump_on_motion_notify), NULL);

  hexdump_ctx.text_buffer = gtk_text_view_get_buffer((GTK_TEXT_VIEW(hexdump_ctx.textview)));
  hexdump_ctx.hover_tag = gtk_text_buffer_create_tag(hexdump_ctx.text_buffer, "hover", "background", "#94e2d5",
                                                     "foreground", "#1e1e2e", NULL);

  return hexdump_ctx.textview;
}

void packet_hexdump_update(uint8_t *buffer, const int length) {
  gtk_text_buffer_set_text(hexdump_ctx.text_buffer, "", -1);
  buffer_len = length;

  GtkTextIter iter;
  gtk_text_buffer_get_start_iter(hexdump_ctx.text_buffer, &iter);

  GString *hex_string = generate_hexdump(buffer, length);
  // gtk_text_buffer_set_text(hexdump_ctx.text_buffer, hex_string->str, -1);
  gtk_text_buffer_insert(hexdump_ctx.text_buffer, &iter, hex_string->str, -1);
  g_string_free(hex_string, TRUE);
}
