/**
 * @file include/proto.h
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-09
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#ifndef GHOSTLAYER_PROTO_H
#define GHOSTLAYER_PROTO_H
#include <gtk/gtk.h>
#include <stdint.h>

#define PROTO_MAX_DISSECTORS 64
#define FUZZER_TOKEN "§FUZZ§"

typedef struct {
  int length;
  uint8_t buffer[512];
} proto_packet_t;

typedef struct {
  const char *name;
  int bit_offset;
  int bit_size;
} bitfield_t;

typedef struct {
  uint8_t *real_data;
  /* Amount of data that was reported */
  int reported_length;
  /* Amount of data that's available from the capture */
  int length;
  int offset;
  // Metadata
  char *timestamp;
} proto_tvbuff_t;

/*
 * Dissector that returns:
 *
 *	The amount of data in the protocol's PDU, if it was able to
 *	dissect all the data;
 *
 */
typedef int (*proto_dissector_t)(proto_packet_t *packet, GtkTreeIter *root, uint8_t *data);
typedef struct {
  char *name; /* dissector name */
  char *description; /* dissector description */
  int dissector_id; /* dissector id*/
  int (*is_valid)(proto_tvbuff_t *tvbuff);
  proto_dissector_t dissect_data;
} proto_dissector_handle_t;

typedef struct {
  const char *name; /* dissector name */
  proto_dissector_handle_t *(*register_fn)(void);
} proto_module_t;

proto_dissector_handle_t *proto_create_dissector(const char *name, const char *description,
                                                 int (*is_valid)(proto_tvbuff_t *tvbuff),
                                                 proto_dissector_t dissect_data);
proto_dissector_handle_t *proto_register_dissector(proto_dissector_handle_t *dissector);

/* Utilities */
char *bitfield_string(uint32_t value, int total_bytes, bitfield_t *fields, int num_fields);
char *uint8_buffer_to_hex_string(const uint8_t *buffer, int length);
char *uint8_buffer_to_hex_string_separator(const uint8_t *buffer, int length, const char *separator);
GString *generate_hexdump(uint8_t *buffer, int length);
uint8_t *hex_string_to_uint8_buffer_token(const char *hex_data, int *out_length);
uint8_t *hex_string_to_uint8_buffer(const char *hex_data, int *out_length);
char *get_timestamp_str(void);
char* validate_and_convert_to_hex(const char *input);
uint8_t *ascii_to_uint8_buffer(const char *input, int *out_length);
#endif //GHOSTLAYER_PROTO_H
