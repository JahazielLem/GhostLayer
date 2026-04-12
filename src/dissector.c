/**
 * @file src/dissector.c
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
#include "bridge.h"
#include "app_state.h"

static proto_dissector_handle_t proto_dissector_table[PROTO_MAX_DISSECTORS];
static int proto_dissector_count = 0;
static proto_dissector_handle_t *dissector_radio = NULL;

static void tvbuff_free(proto_tvbuff_t *tvb) {
  if (tvb) {
    if (tvb->real_data) {
      free(tvb->real_data);
    }
    free(tvb);
  }
}

static proto_tvbuff_t * tvbuff_init(uint8_t *data, uint16_t len) {
  int offset = 2; // Header
  proto_tvbuff_t *meta_buff = calloc(1, sizeof(proto_tvbuff_t));
  meta_buff->reported_length = len - 4; // Header + Tail
  meta_buff->timestamp = get_timestamp_str();

  meta_buff->length = data[offset++];

  meta_buff->real_data = malloc(meta_buff->length);
  if (meta_buff->real_data == NULL) {
    g_print("Memory allocation failed!\n");
    return NULL;
  }

  memcpy(meta_buff->real_data, data + offset, meta_buff->length);
  return meta_buff;
}

static proto_tvbuff_t * tvbuff_init_from_file(uint8_t *data, uint16_t len) {
  proto_tvbuff_t *meta_buff = calloc(1, sizeof(proto_tvbuff_t));
  meta_buff->reported_length = len;
  meta_buff->timestamp = get_timestamp_str();
  meta_buff->length = len;
  meta_buff->real_data = malloc(meta_buff->length);
  if (meta_buff->real_data == NULL) {
    g_print("Memory allocation failed!\n");
    return NULL;
  }
  memcpy(meta_buff->real_data, data, meta_buff->length);
  return meta_buff;
}

static proto_dissector_handle_t *proto_get_dissector_by_name(const char *name) {
  for (int i = 0; i < proto_dissector_count; i++) {
    if (strcmp(name, proto_dissector_table[i].name) == 0) {
      return &proto_dissector_table[i];
    }
  }
  return NULL;
}

static void dissector_packet_details(proto_packet_t *pkt) {
  if (dissector_radio == NULL) {
    dissector_radio = proto_get_dissector_by_name("SPP");
  }

  if (dissector_radio != NULL) {
    g_print("%s\n", uint8_buffer_to_hex_string(pkt->buffer,pkt->length));
    packet_details_clear();
    dissector_radio->dissect_data(pkt, NULL, pkt->buffer);
    packet_hexdump_update(pkt->buffer,pkt->length);
    packet_viewer_expand_tree();
  }
}

static void dissector_packet_parser(uint8_t *buffer, const gsize buffer_len) {
  if (dissector_radio == NULL) {
    dissector_radio = proto_get_dissector_by_name("SPP");
  }

  proto_tvbuff_t *tv_buffer = tvbuff_init(buffer, (uint16_t)buffer_len);

  if (dissector_radio != NULL) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    if (dissector_radio->is_valid(tv_buffer)) {
      app_state_new_packet(dissector_radio->name, dissector_radio->description, tv_buffer->real_data, tv_buffer->length, &current_time);
    }else {
      app_state_new_packet("RAW", "", tv_buffer->real_data, tv_buffer->length, &current_time);
    }
  }

  tvbuff_free(tv_buffer);
}

void dissector_packet_parser_from_file(uint8_t *buffer, const gsize buffer_len, struct timeval *timestamp) {
  if (dissector_radio == NULL) {
    dissector_radio = proto_get_dissector_by_name("SPP");
  }

  proto_tvbuff_t *tv_buffer = tvbuff_init_from_file(buffer, (uint16_t)buffer_len);

  if (dissector_radio != NULL) {
    if (dissector_radio->is_valid(tv_buffer)) {
      app_state_new_packet(dissector_radio->name, dissector_radio->description, tv_buffer->real_data, tv_buffer->length, timestamp);
    }else {
      app_state_new_packet("RAW", "", tv_buffer->real_data, tv_buffer->length, timestamp);
    }
  }

  tvbuff_free(tv_buffer);
}

static gboolean dissector_raw_data_cb(GIOChannel *source, GIOCondition condition, gpointer user_data) {
  (void)user_data;
  if (condition & (G_IO_HUP | G_IO_ERR)) {
    g_print("Client disconnected or error\n");
    return FALSE;
  }

  uint8_t buffer[1024];
  gsize len = 0;
  GError *error = NULL;

  GIOStatus status = g_io_channel_read_chars(source, (gchar *) buffer, sizeof(buffer) -1, &len, &error);
  if (status == G_IO_STATUS_NORMAL && len > 0) {
    buffer[len] = '\0';
    if (buffer[0] == BRIDGE_MAGIC_HEADER_1 && buffer[1] == BRIDGE_MAGIC_HEADER_2 && buffer[len - 2] == BRIDGE_MAGIC_TAIL_1 && buffer[len - 1] == BRIDGE_MAGIC_TAIL_2) {
      dissector_packet_parser(buffer, len);
    }
  } else if (status == G_IO_STATUS_ERROR && error) {
    g_printerr("TCP read error: %s\n", error->message);
    g_error_free(error);
  }
  return TRUE;
}

proto_dissector_handle_t *proto_create_dissector(const char *name, const char *description,
                                                 int (*is_valid)(proto_tvbuff_t *tvbuff),
                                                 proto_dissector_t dissect_data) {
  proto_dissector_handle_t *handle = calloc(1, sizeof(proto_dissector_handle_t));
  handle->name = strdup(name);
  handle->description = strdup(description);
  handle->is_valid = is_valid;
  handle->dissect_data = dissect_data;
  handle->dissector_id = 0;
  return handle;
}

proto_dissector_handle_t *proto_register_dissector(proto_dissector_handle_t *dissector) {
  if (proto_dissector_count > PROTO_MAX_DISSECTORS) {
    g_print("Dissector limit reached\n");
    return NULL;
  }
  proto_dissector_table[proto_dissector_count].name = dissector->name;
  proto_dissector_table[proto_dissector_count].description = dissector->description;
  proto_dissector_table[proto_dissector_count].is_valid = dissector->is_valid;
  proto_dissector_table[proto_dissector_count].dissect_data = dissector->dissect_data;
  proto_dissector_table[proto_dissector_count].dissector_id = proto_dissector_count;

  proto_dissector_count++;
  return proto_dissector_table;
}

void register_dissectors(void) {
  proto_module_t *module = dissector_register_lora();
  if (proto_register_dissector(module->register_fn()) != NULL) {
    g_print("Registered dissector: %s\n", module->name);
  }

  /* ADD CUSTOM DISSECTOR HERE */
  module = dissector_register_spp();
  if (proto_register_dissector(module->register_fn()) != NULL) {
    g_print("Registered dissector: %s\n", module->name);
  }
}

void dissector_parser_register(void) {
  bridge_engine_register_read_cb(dissector_raw_data_cb);
  packet_viewer_register_select_cb(dissector_packet_details);
}
