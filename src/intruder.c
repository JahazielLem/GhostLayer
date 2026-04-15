/**
 * @file src/intruder.c
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
#include "alerts.h"
#include "main_gui.h"
#include "plugins.h"
#include "bridge.h"
#include "app_state.h"

typedef struct {
  gint attack_type;
  gint from;
  gint to;
  gint steps;
  gint current;
  gint timeout;
  gboolean running;

  // Shared context for packet building
  uint8_t *base_payload;
  int base_payload_len;
  spp_apid_context_t spp_context;

  // Specific context for Payload List attack
  GList *payload_list;
  GList *current_node;
  guint total_payloads;
  int current_payload_idx;
} fuzzer_context_t;

static fuzzer_context_t intruder_fuzz_ctx;
static GtkWidget *packet_viewer_window = NULL;
static GtkWidget *progress_bar = NULL;
static proto_packet_t *context_packet;

static void intruder_fuzzer_cleanup(void) {
  if (intruder_fuzz_ctx.base_payload) {
    g_free(intruder_fuzz_ctx.base_payload);
    intruder_fuzz_ctx.base_payload = NULL;
  }

  if (intruder_fuzz_ctx.payload_list) {
    for (GList *l = intruder_fuzz_ctx.payload_list; l != NULL; l = l->next) {
      g_free(l->data);
    }
    g_list_free(intruder_fuzz_ctx.payload_list);
    intruder_fuzz_ctx.payload_list = NULL;
  }
}

static int intruder_build_packet_to_send(uint8_t *buffer, uint16_t buffer_length, spp_apid_context_t *context) {
  space_packet_t new_spp;
  int ret = SPP_ERROR_NONE;

  uint8_t dummy_payload[1] = {0x00};
  const uint8_t *ptr = (buffer != NULL && buffer_length > 0) ? buffer : dummy_payload;
  const uint16_t len = (buffer != NULL && buffer_length > 0) ? buffer_length : 1;

  if (plugin_spp_get_type() == SPP_PTYPE_TM) {
    ret = spp_tm_build_packet(&new_spp, plugin_spp_get_seq_flag(),
      plugin_spp_get_sechdr(), 0,
      ptr, len, context);
  }else {
    ret = spp_tc_build_packet(&new_spp, plugin_spp_get_seq_flag(),
    plugin_spp_get_sechdr(), 0,
    ptr, len, context);

  }
  if (ret != SPP_ERROR_NONE) {
    g_print("SPP error: %d\n", ret);
    return ret;
  }

  const uint16_t spp_payload_len = HOST_TO_BE16(new_spp.header.length) + 1;
  const uint16_t total_size = SPP_PRIMARY_HEADER_LEN + spp_payload_len;
  app_state_transmit_packet_with_config((uint8_t*)&new_spp, total_size);
  return ret;
}

static void intruder_setup_discovery_attack(void) {
  intruder_fuzzer_cleanup(); // Ensure clean state

  intruder_fuzz_ctx.attack_type = ATTACK_SERVICE_DISCOVERY;
  intruder_fuzz_ctx.from = intruder_gui_get_range_from(ATTACK_SERVICE_DISCOVERY);
  intruder_fuzz_ctx.to = intruder_gui_get_range_to(ATTACK_SERVICE_DISCOVERY);
  intruder_fuzz_ctx.steps = intruder_gui_get_range_steps(ATTACK_SERVICE_DISCOVERY);
  if (intruder_fuzz_ctx.steps <= 0) intruder_fuzz_ctx.steps = 1; // Prevent infinite loop

  intruder_fuzz_ctx.current = intruder_fuzz_ctx.from;
  intruder_fuzz_ctx.timeout = plugin_radio_get_delay();

  const char *text = intruder_gui_get_data();
  intruder_fuzz_ctx.base_payload = ascii_to_uint8_buffer(text, &intruder_fuzz_ctx.base_payload_len);

  memset(&intruder_fuzz_ctx.spp_context, 0, sizeof(spp_apid_context_t));
  intruder_fuzz_ctx.spp_context.tm = plugin_spp_get_seq_counter();
  intruder_fuzz_ctx.spp_context.tc = plugin_spp_get_seq_counter();
}

static void intruder_setup_exhaustion_attack(void) {
  intruder_fuzzer_cleanup();

  intruder_fuzz_ctx.attack_type = ATTACK_SEQ_EXHAUSTION;
  intruder_fuzz_ctx.from = intruder_gui_get_range_from(ATTACK_SEQ_EXHAUSTION);
  intruder_fuzz_ctx.to = intruder_gui_get_range_to(ATTACK_SEQ_EXHAUSTION);
  intruder_fuzz_ctx.steps = intruder_gui_get_range_steps(ATTACK_SEQ_EXHAUSTION);
  if (intruder_fuzz_ctx.steps <= 0) intruder_fuzz_ctx.steps = 1;

  intruder_fuzz_ctx.current = intruder_fuzz_ctx.from;
  intruder_fuzz_ctx.timeout = plugin_radio_get_delay();

  const char *text = intruder_gui_get_data();
  intruder_fuzz_ctx.base_payload = ascii_to_uint8_buffer(text, &intruder_fuzz_ctx.base_payload_len);

  memset(&intruder_fuzz_ctx.spp_context, 0, sizeof(spp_apid_context_t));
  intruder_fuzz_ctx.spp_context.apid = (uint16_t)plugin_spp_get_apid();
}

static void intruder_setup_payload_attack(void) {
  intruder_fuzzer_cleanup();

  intruder_fuzz_ctx.attack_type = ATTACK_MANUAL_INJECTION;
  intruder_fuzz_ctx.payload_list = intruder_gui_get_payload_list();
  intruder_fuzz_ctx.current_node = intruder_fuzz_ctx.payload_list;
  intruder_fuzz_ctx.total_payloads = g_list_length(intruder_fuzz_ctx.payload_list);
  intruder_fuzz_ctx.current_payload_idx = 0;
  intruder_fuzz_ctx.timeout = plugin_radio_get_delay();

  memset(&intruder_fuzz_ctx.spp_context, 0, sizeof(spp_apid_context_t));
  intruder_fuzz_ctx.spp_context.apid = (uint16_t)plugin_spp_get_apid();
  intruder_fuzz_ctx.spp_context.tm = plugin_spp_get_seq_counter();
  intruder_fuzz_ctx.spp_context.tc = plugin_spp_get_seq_counter();
}

static gboolean fuzzer_gui_progress_worker(gpointer user_data) {
  GtkWidget *window = (GtkWidget*)user_data;

  if (!intruder_fuzz_ctx.running) { return FALSE; }

  gdouble progress = 0.0;
  gboolean attack_finished = FALSE;

  g_print("Running: %d of %d\n", intruder_fuzz_ctx.current, intruder_fuzz_ctx.to);

  // 1. DISCOVERY & EXHAUSTION LOGIC
  if (intruder_fuzz_ctx.attack_type == ATTACK_SERVICE_DISCOVERY ||
      intruder_fuzz_ctx.attack_type == ATTACK_SEQ_EXHAUSTION) {

    if (intruder_fuzz_ctx.current > intruder_fuzz_ctx.to) {
      attack_finished = TRUE;
    } else {
      if (intruder_fuzz_ctx.attack_type == ATTACK_SERVICE_DISCOVERY) {
        intruder_fuzz_ctx.spp_context.apid = (uint16_t)intruder_fuzz_ctx.current;
      } else {
        intruder_fuzz_ctx.spp_context.tm = intruder_fuzz_ctx.attack_type == ATTACK_SEQ_EXHAUSTION ? (uint16_t)rand(): (uint16_t)intruder_fuzz_ctx.current;
        intruder_fuzz_ctx.spp_context.tc = intruder_fuzz_ctx.attack_type == ATTACK_SEQ_EXHAUSTION ? (uint16_t)rand(): (uint16_t)intruder_fuzz_ctx.current;
      }

      fuzzer_gui_packet_viewer_add_numeric_payload((uint32_t)intruder_fuzz_ctx.current,
                                    intruder_fuzz_ctx.base_payload,
                                    intruder_fuzz_ctx.base_payload_len);
      intruder_build_packet_to_send(intruder_fuzz_ctx.base_payload,
                                    intruder_fuzz_ctx.base_payload_len,
                                    &intruder_fuzz_ctx.spp_context);

      // Calculate progress
      if (intruder_fuzz_ctx.to > intruder_fuzz_ctx.from) {
        progress = (gdouble)(intruder_fuzz_ctx.current - intruder_fuzz_ctx.from) /
                   (intruder_fuzz_ctx.to - intruder_fuzz_ctx.from);
      } else {
        progress = 1.0;
      }

      intruder_fuzz_ctx.current += intruder_fuzz_ctx.steps;
    }
  }
  // 2. PAYLOAD MUTATION LOGIC
  else if (intruder_fuzz_ctx.attack_type == ATTACK_MANUAL_INJECTION) {
    if (intruder_fuzz_ctx.current_node == NULL) {
      attack_finished = TRUE;
    } else {
      gchar *payload_text = (gchar *)intruder_fuzz_ctx.current_node->data;

      int new_payload_len = 0;
      uint8_t *new_buffer = ascii_to_uint8_buffer(payload_text, &new_payload_len);

      fuzzer_gui_packet_viewer_add_list_payload(payload_text, new_buffer, new_payload_len);
      intruder_build_packet_to_send(new_buffer, new_payload_len, &intruder_fuzz_ctx.spp_context);

      if (new_buffer) g_free(new_buffer);

      intruder_fuzz_ctx.current_payload_idx++;
      progress = (gdouble)intruder_fuzz_ctx.current_payload_idx / intruder_fuzz_ctx.total_payloads;

      intruder_fuzz_ctx.current_node = intruder_fuzz_ctx.current_node->next;
    }
  }

  // 3. UI UPDATES
  if (attack_finished) {
    intruder_fuzz_ctx.running = FALSE;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "100% - Complete");
    intruder_fuzzer_cleanup();
    alert_show_dialog(GTK_WINDOW(window), ALERT_SUCCESS, "Fuzzer", "Attack complete!");
    return FALSE;
  }

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), progress);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f%%", progress * 100.0);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), buf);

  return TRUE;
}

static void intruder_gui_loading_dialog_destroy(GtkWidget *widget) {
  // TODO: ADD CONFIRMATION
  if (intruder_fuzz_ctx.running) {
    intruder_fuzz_ctx.running = FALSE;
  }
  intruder_fuzzer_cleanup();
  gtk_widget_destroy(widget);
  packet_viewer_window = NULL;
}

static void intruder_gui_loading_dialog(void) {
  packet_viewer_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(packet_viewer_window),
    g_strdup_printf("Intruder - Fuzzer Attack"));
  gtk_window_set_default_size(GTK_WINDOW(packet_viewer_window), (int)(APPLICATION_MIN_WIDTH * 0.8), (int)(APPLICATION_MIN_HEIGHT * 0.8));
  gtk_window_set_position(GTK_WINDOW(packet_viewer_window), GTK_WIN_POS_CENTER);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(packet_viewer_window), vbox);

  GtkWidget *packet_viewer = fuzzer_gui_packet_viewer();
  progress_bar = gtk_progress_bar_new();
  gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
  gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), packet_viewer, TRUE, TRUE, 0);

  g_signal_connect(packet_viewer_window, "destroy", G_CALLBACK(intruder_gui_loading_dialog_destroy), NULL);

  g_timeout_add((intruder_fuzz_ctx.timeout * 1000), fuzzer_gui_progress_worker, packet_viewer_window);

  gtk_widget_show_all(packet_viewer_window);
}

void intruder_send_attack(const gint attack) {
  switch (attack) {
    case ATTACK_SERVICE_DISCOVERY:
      intruder_setup_discovery_attack();
      break;
    case ATTACK_SEQ_EXHAUSTION:
      intruder_setup_exhaustion_attack();
      break;
    case ATTACK_MANUAL_INJECTION:
      intruder_setup_payload_attack();
      break;
    default: break;
  }
  intruder_fuzz_ctx.running = TRUE;
  intruder_gui_loading_dialog();
}

proto_packet_t *intruder_get_packet_data(void) {
  return context_packet;
}

void intruder_inspect_packet(proto_packet_t *packet) {
  if (intruder_gui_get_instance() == NULL) {
    intruder_gui_create();
  }

  context_packet = g_new0(proto_packet_t, 1);
  memcpy(context_packet->buffer, packet->buffer, packet->length);
  context_packet->length = packet->length;

  plugin_spp_parse_packet(context_packet->buffer, context_packet->length);
  intruder_gui_hexeditor_update(context_packet->buffer, context_packet->length);

  gtk_window_present(GTK_WINDOW(intruder_gui_get_instance()));
}
