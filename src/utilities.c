/**
 * @file src/utilities.c
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

#include "../include/proto.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** @brief Generate hexdump format. Use free(str) after use **/
char *bitfield_string(const uint32_t value, const int total_bytes, bitfield_t *fields, const int num_fields) {
  const int total_bits = total_bytes * 8;
  const int buf_size = 1024;
  char *output = (char *) malloc(buf_size);
  if (!output) {
    g_printerr("OUT OF MEMORY\n");
    return NULL;
  }
  output[0] = '\0';
  // Lines per field
  for (int f = 0; f < num_fields; f++) {
    const bitfield_t *field = &fields[f];
    const int start_bit = total_bits - field->bit_offset - field->bit_size;
    const int end_bit = total_bits - field->bit_offset - 1;

    for (int i = 0; i < total_bits; i++) {
      char ch[3] = ". ";
      if (i >= start_bit && i <= end_bit) {
        const int bit_index = total_bits - 1 - i;
        const int bit = (value >> bit_index) & 0x1;
        snprintf(ch, sizeof(ch), "%d ", bit);
      }
      strcat(output, ch);
      if ((i + 1) % 4 == 0) {
        strcat(output, " ");
      }
    }
    strcat(output, "= ");
    strcat(output, field->name);
    strcat(output, "\n");
  }
  return output;
}

char *uint8_buffer_to_hex_string(const uint8_t *buffer, int length) {
  if (buffer == NULL || length <= 0)
    return NULL;

  GString *gstr = g_string_new(NULL);
  for (int i = 0; i < length; i++) {
    if (i > 0)
      g_string_append(gstr, " ");
    g_string_append_printf(gstr, "%02x", buffer[i]);
  }

  char *result = g_strdup(gstr->str);
  g_string_free(gstr, TRUE);
  return result;
}

char *uint8_buffer_to_hex_string_separator(const uint8_t *buffer, int length, const char *separator) {
  if (buffer == NULL || length <= 0)
    return NULL;

  GString *gstr = g_string_new(NULL);
  for (int i = 0; i < length; i++) {
    if (i > 0)
      g_string_append(gstr, separator);
    g_string_append_printf(gstr, "%02x", buffer[i]);
  }

  char *result = g_strdup(gstr->str);
  g_string_free(gstr, TRUE);
  return result;
}

/** @brief Generate hexdump format. Use g_string_free(str, TRUE) after use **/
GString *generate_hexdump(uint8_t *buffer, int length) {
  GString *hex_string = g_string_new(NULL);
  for (int i = 0; i < length; i++) {
    if (i % 16 == 0) {
      if (i > 0)
        g_string_append(hex_string, "\n");
      g_string_append_printf(hex_string, "%04x  ", i);
    }

    g_string_append_printf(hex_string, "%02x ", buffer[i]);

    if ((i + 1) % 8 == 0 && (i + 1) % 16 != 0) {
      g_string_append(hex_string, " ");
    }

    if ((i + 1) % 16 == 0 || i == length - 1) {
      int start_idx = (i / 16) * 16;
      int bytes_in_line = (i + 1) - start_idx;
      if (i == length - 1 && (i + 1) % 16 != 0) {
        bytes_in_line = i + 1 - start_idx;
      }

      for (int j = bytes_in_line; j < 16; j++) {
        if (j % 8 == 0 && j > 0)
          g_string_append(hex_string, " ");
        g_string_append(hex_string, "   ");
      }
      g_string_append(hex_string, " ");
      for (int j = start_idx; j < start_idx + bytes_in_line; j++) {
        unsigned char c = buffer[j];
        if (c >= 32 && c <= 126) {
          g_string_append_c(hex_string, c);
        } else {
          g_string_append_c(hex_string, '.');
        }
      }
      for (int j = bytes_in_line; j < 16; j++) {
        g_string_append_c(hex_string, '.');
      }
    }
  }
  if (length > 0) {
    g_string_append(hex_string, "\n");
  }
  return hex_string;
}

uint8_t *hex_string_to_uint8_buffer_token(const char *hex_data, int *out_length) {
  if (!hex_data) {
    *out_length = 0;
    return NULL;
  }

  const size_t totalLen = strlen(hex_data);
  const char *fuzz_token = FUZZER_TOKEN;
  size_t fuzz_token_len = strlen(fuzz_token);
  int byte_count = 0;

  for (size_t i = 0; i < totalLen; i++) {
    if (hex_data[i] != ' ') {
      // Check start of token
      if (i <= totalLen - fuzz_token_len && strncmp(&hex_data[i], fuzz_token, fuzz_token_len) == 0) {
        i += fuzz_token_len - 1;
        continue;
      }
      if (hex_data[i] != ' ' && isxdigit((unsigned char) hex_data[i])) {
        byte_count++;
      }
    }
  }
  byte_count = byte_count / 2;
  if (byte_count == 0) {
    *out_length = 0;
    return NULL;
  }

  uint8_t *buffer = (uint8_t *) malloc(byte_count * sizeof(uint8_t));
  if (!buffer) {
    *out_length = 0;
    return NULL;
  }

  const char *ptr = hex_data;
  int index = 0;
  while (*ptr && index < byte_count) {
    if (strncmp(ptr, fuzz_token, fuzz_token_len) == 0) {
      ptr += fuzz_token_len;
      while (*ptr == ' ')
        ptr++;
      continue;
    }

    if (isxdigit((unsigned char) ptr[0]) && isxdigit((unsigned char) ptr[1])) {
      char hex_byte[3] = {ptr[0], ptr[1], '\0'};
      buffer[index] = (uint8_t) strtol(hex_byte, NULL, 16);
      index++;
      ptr += 2;
    } else {
      ptr++;
    }
    while (*ptr == ' ')
      ptr++;
  }

  *out_length = byte_count;
  return buffer;
}

uint8_t *hex_string_to_uint8_buffer(const char *hex_data, int *out_length) {
  if (!hex_data) {
    *out_length = 0;
    return NULL;
  }

  const size_t len = strlen(hex_data);
  int byte_count = 0;
  for (size_t i = 0; i < len; i++) {
    if (hex_data[i] != ' ')
      byte_count++;
  }
  byte_count = byte_count / 2;
  if (byte_count == 0) {
    *out_length = 0;
    return NULL;
  }

  uint8_t *buffer = (uint8_t *) malloc(byte_count * sizeof(uint8_t));
  if (!buffer) {
    *out_length = 0;
    return NULL;
  }

  const char *ptr = hex_data;
  int index = 0;
  while (*ptr && index < byte_count) {
    const char hex_byte[3] = {ptr[0], ptr[1], '\0'};
    buffer[index] = (uint8_t) strtol(hex_byte, NULL, 16);
    index++;
    ptr += 2;
    while (*ptr == ' ')
      ptr++;
  }

  *out_length = byte_count;
  return buffer;
}

char *get_timestamp_str(void) {
  static char timestamp[16];
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  const struct tm *tm_info = localtime(&ts.tv_sec);

  snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d.%02ld", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           ts.tv_nsec / 1000);
  return timestamp;
}

char* validate_and_convert_to_hex(const char *input) {
  if (!input || !*input) return NULL;

  const char *p = input;
  while (isspace((unsigned char)*p)) p++;
  if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;

  gboolean is_hex = TRUE;
  int digit_count = 0;

  for (const char *tmp = p; *tmp; tmp++) {
    if (isspace((unsigned char)*tmp)) continue;
    if (!isxdigit((unsigned char)*tmp)) {
      is_hex = FALSE;
      break;
    }
    digit_count++;
  }

  if (is_hex && digit_count > 0) {
    return g_strdup(p);
  }

  const size_t len = strlen(input);
  char *hex_out = g_malloc(len * 2 + 1);
  for (size_t i = 0; i < len; i++) {
    sprintf(hex_out + (i * 2), "%02X", (unsigned char)input[i]);
  }
  hex_out[len * 2] = '\0';

  return hex_out;
}

/* Manual free needed */
uint8_t *ascii_to_uint8_buffer(const char *input, int *out_length) {
  if (!input || !*input) {
    *out_length = 0;
    return NULL;
  }
  const size_t len = strlen(input);
  *out_length = (int)len;

  uint8_t *buffer = g_malloc(len);
  memcpy(buffer, input, len);
  return buffer;
}
