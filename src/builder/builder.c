/**
 * @file src/builder/builder.c
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
#include "main_gui.h"
#include "app_state.h"
#include "alerts.h"

typedef struct {
  GtkWidget *window;
  GtkWidget *template_view;
  GtkTextBuffer *template_buffer;
  GtkWidget *combo_strategy;
  GtkWidget *progress_bar;
  GtkWidget *btn_launch;
  GtkWidget *combo_tokens;
  GtkWidget *attack_stack;

  // General widgets
  // Numeric
  GtkWidget *numeric_from;
  GtkWidget *numeric_to;
  GtkWidget *numeric_step;
} advanced_gui_ctx_t;

typedef struct {
  double numeric_from;
  double numeric_to;
  double numeric_step;

  int bof_length;
  int bof_pattern_type;

  GList *payload_list;
} token_data_t;

typedef struct {
  int id;
  gint attack_type;
  GtkTextMark *start_mark;
  GtkTextMark *end_mark;
  GtkTextTag *tag;
  const char *color;
  token_data_t data;
} advanced_marked_ctx_t;

static const char *marker_colors[] = {
  "#ff5555", "#50fa7b", "#f1fa8c", "#bd93f9", "#ff79c6", "#8be9fd", "#ffb86c", "#6272a4"
};
#define MAX_COLORS (sizeof(marker_colors) / sizeof(marker_colors[0]))

static advanced_gui_ctx_t gui_ctx;
static GList *marked_list = NULL;
static GtkWidget *combo_gen;

static void on_combo_attack_change(GtkComboBox *combo, gpointer data);
static void on_token_selection_changed(GtkComboBox *combo, gpointer data);

// Cambia la firma de la función. El 'widget' es el SpinButton que se modificó.
static void on_numeric_change(GtkWidget *widget, gpointer data) {
  (void)widget;
  (void)data;

  const gint token_index = gtk_combo_box_get_active(GTK_COMBO_BOX(gui_ctx.combo_tokens));

  if (token_index < 0) {return;}

  advanced_marked_ctx_t *token = (advanced_marked_ctx_t *)g_list_nth_data(marked_list, token_index);

  if (!token) {return;}

  token->data.numeric_from = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gui_ctx.numeric_from));
  token->data.numeric_to = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gui_ctx.numeric_to));
  token->data.numeric_step = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gui_ctx.numeric_step));
}

static void on_update_numeric_values(advanced_marked_ctx_t *token) {
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui_ctx.numeric_from), token->data.numeric_from);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui_ctx.numeric_to), token->data.numeric_to);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui_ctx.numeric_step), token->data.numeric_step);
}

static GtkWidget *intruder_gui_number_rage_create(void) {
  GtkWidget *num_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(num_grid), 8);
  gtk_grid_set_column_spacing(GTK_GRID(num_grid), 15);
  gtk_container_set_border_width(GTK_CONTAINER(num_grid), 10);

  GtkAdjustment *adj_from = gtk_adjustment_new(0, 0, 1000000, 1, 10, 0);
  GtkAdjustment *adj_to = gtk_adjustment_new(0, 0, 1000000, 1, 10, 0);
  GtkAdjustment *adj_steps = gtk_adjustment_new(0, 1, 1000000, 1, 10, 0);

  gui_ctx.numeric_from = gtk_spin_button_new(adj_from, 1, 0);
  gui_ctx.numeric_to = gtk_spin_button_new(adj_to, 1, 0);
  gui_ctx.numeric_step = gtk_spin_button_new(adj_steps, 1, 0);

  int i = 0;
  gtk_grid_attach(GTK_GRID(num_grid), gtk_label_new("From"), 0, i, 1, 1);
  gtk_grid_attach(GTK_GRID(num_grid), gui_ctx.numeric_from, 1, i, 1, 1);
  i++;

  gtk_grid_attach(GTK_GRID(num_grid), gtk_label_new("To"), 0, i, 1, 1);
  gtk_grid_attach(GTK_GRID(num_grid), gui_ctx.numeric_to, 1, i, 1, 1);
  i++;

  gtk_grid_attach(GTK_GRID(num_grid), gtk_label_new("Steps"), 0, i, 1, 1);
  gtk_grid_attach(GTK_GRID(num_grid), gui_ctx.numeric_step, 1, i, 1, 1);

  g_signal_connect(gui_ctx.numeric_from, "value-changed", G_CALLBACK(on_numeric_change), NULL);
  g_signal_connect(gui_ctx.numeric_to, "value-changed", G_CALLBACK(on_numeric_change), NULL);
  g_signal_connect(gui_ctx.numeric_step, "value-changed", G_CALLBACK(on_numeric_change), NULL);

  return num_grid;
}

static void sync_token_widgets(advanced_marked_ctx_t *token) {
  if (!token) return;

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_gen), token->attack_type);

  const char *page_name[] = {"numeric_page", "list_page", "blocks_page"};
  gtk_stack_set_visible_child_name(GTK_STACK(gui_ctx.attack_stack), page_name[token->attack_type]);

  on_update_numeric_values(token);
}

static void on_token_selection_changed(GtkComboBox *combo, gpointer data) {
  (void)data;
  const gint index = gtk_combo_box_get_active(combo);
  if (index < 0) return;

  advanced_marked_ctx_t *token = g_list_nth_data(marked_list, index);
  sync_token_widgets(token);
}

static void on_combo_attack_change(GtkComboBox *combo, gpointer data) {
  (void)data;
  const gint attack_type = gtk_combo_box_get_active(combo);
  const gint token_index = gtk_combo_box_get_active(GTK_COMBO_BOX(gui_ctx.combo_tokens));

  if (token_index >= 0) {
    advanced_marked_ctx_t *token = g_list_nth_data(marked_list, token_index);
    token->attack_type = attack_type;
    sync_token_widgets(token);
  }
}

static void update_token_id(void) {
  gint counter = 0;
  for (GList *l = marked_list; l != NULL; l = l->next) {
    advanced_marked_ctx_t *marker = (advanced_marked_ctx_t *)l->data;
    marker->id = counter + 1;
  }
}

static void on_update_token_combo(void) {
  if (!gui_ctx.combo_tokens) { return; }

  gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(gui_ctx.combo_tokens));

  for (GList *l = marked_list; l != NULL; l = l->next) {
    const advanced_marked_ctx_t *marker = (advanced_marked_ctx_t *)l->data;
    char label[48];
    sprintf(label, "Token %d (§)", marker->id);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gui_ctx.combo_tokens), label);
  }
  if (g_list_length(marked_list) > 0) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(gui_ctx.combo_tokens), (gint)g_list_length(marked_list) - 1);
  }
}

static void on_clear_last_marker_clicked(void) {
  if (marked_list == NULL) return;

  GList *last_node = g_list_last(marked_list);
  advanced_marked_ctx_t *mark_ctx = (advanced_marked_ctx_t *)last_node->data;

  if (mark_ctx) {
    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_mark(gui_ctx.template_buffer, &start, mark_ctx->start_mark);
    gtk_text_buffer_get_iter_at_mark(gui_ctx.template_buffer, &end, mark_ctx->end_mark);

    gtk_text_buffer_remove_tag(gui_ctx.template_buffer, mark_ctx->tag, &start, &end);

    GtkTextTagTable *table = gtk_text_buffer_get_tag_table(gui_ctx.template_buffer);
    gtk_text_tag_table_remove(table, mark_ctx->tag);

    gtk_text_buffer_delete_mark(gui_ctx.template_buffer, mark_ctx->start_mark);
    gtk_text_buffer_delete_mark(gui_ctx.template_buffer, mark_ctx->end_mark);

    g_free(mark_ctx);
    marked_list = g_list_delete_link(marked_list, last_node);
  }
  update_token_id();
  on_update_token_combo();
}

static void on_clear_all_marker_clicked(void) {
  if (!gui_ctx.combo_tokens) { return; }

  GList *l = marked_list;
  int i = 0;
  while (l != NULL) {
    advanced_marked_ctx_t *mark_ctx = (advanced_marked_ctx_t *)l->data;

    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_mark(gui_ctx.template_buffer, &start, mark_ctx->start_mark);
    gtk_text_buffer_get_iter_at_mark(gui_ctx.template_buffer, &end, mark_ctx->end_mark);

    char tag_name[32];
    snprintf(tag_name, sizeof(tag_name), "token_tag_%d", i++);

    gtk_text_buffer_remove_tag_by_name(gui_ctx.template_buffer, tag_name, &start, &end);
    gtk_text_buffer_delete_mark(gui_ctx.template_buffer, mark_ctx->start_mark);
    gtk_text_buffer_delete_mark(gui_ctx.template_buffer, mark_ctx->end_mark);

    l = l->next;
  }

  g_list_free_full(marked_list, g_free);
  marked_list = NULL;

  on_update_token_combo();
}

static void on_add_marker_clicked(void) {
  GtkTextIter start, end;

  if (gtk_text_buffer_get_selection_bounds(gui_ctx.template_buffer, &start, &end)) {
    const guint current_id = g_list_length(marked_list);
    const char *color = marker_colors[current_id % MAX_COLORS];

    advanced_marked_ctx_t *mark_ctx = g_new0(advanced_marked_ctx_t, 1);
    mark_ctx->id = (gint)current_id + 1;
    mark_ctx->color = color;

    mark_ctx->tag = gtk_text_buffer_create_tag(gui_ctx.template_buffer, NULL,
                             "background", color,
                             "foreground", "#1e1e1e",
                             "weight", PANGO_WEIGHT_BOLD, NULL);

    gtk_text_buffer_apply_tag(gui_ctx.template_buffer, mark_ctx->tag, &start, &end);

    mark_ctx->start_mark = gtk_text_buffer_create_mark(gui_ctx.template_buffer, NULL, &start, FALSE);
    mark_ctx->end_mark = gtk_text_buffer_create_mark(gui_ctx.template_buffer, NULL, &end, FALSE);

    marked_list = g_list_append(marked_list, mark_ctx);

    on_update_token_combo();
  }
}

static GtkWidget *create_template_layout_left(void) {
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 15);

  GtkWidget *top_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

  gui_ctx.combo_strategy = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gui_ctx.combo_strategy), "sniper", "Sniper (One by one)");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gui_ctx.combo_strategy), "bomb", "Cluster Bomb (Cartesian Product)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(gui_ctx.combo_strategy), 0);
  gtk_box_pack_start(GTK_BOX(top_hbox), gui_ctx.combo_strategy, TRUE, FALSE, 0);

  GtkWidget *btn_start_attack = gtk_button_new_with_label("Start Attack");
  g_signal_connect(btn_start_attack, "clicked", G_CALLBACK(on_add_marker_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(top_hbox), btn_start_attack, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(left_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  GtkWidget *button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

  GtkWidget *btn_marker = gtk_button_new_with_label("Add Marker (§)");
  g_signal_connect(btn_marker, "clicked", G_CALLBACK(on_add_marker_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(button_hbox), btn_marker, FALSE, FALSE, 0);

  GtkWidget *btn_clear_last = gtk_button_new_with_label("Clear Last Marker (§)");
  g_signal_connect(btn_clear_last, "clicked", G_CALLBACK(on_clear_last_marker_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(button_hbox), btn_clear_last, FALSE, FALSE, 0);

  GtkWidget *btn_clear_all = gtk_button_new_with_label("Clear All Marker (§)");
  g_signal_connect(btn_clear_all, "clicked", G_CALLBACK(on_clear_all_marker_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(button_hbox), btn_clear_all, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(left_vbox), top_hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(left_vbox), button_hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(left_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

  GtkWidget *label_moded = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_moded), "<b><span size='large'>Packet (Hex)</span></b>");
  gtk_widget_set_halign(label_moded, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(left_vbox), label_moded, FALSE, FALSE, 0);

  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_widget_set_hexpand(scroll, TRUE);
  gui_ctx.template_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(gui_ctx.template_view), TRUE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(gui_ctx.template_view), TRUE);
  gui_ctx.template_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui_ctx.template_view));

  gtk_container_add(GTK_CONTAINER(scroll), gui_ctx.template_view);
  gtk_box_pack_start(GTK_BOX(left_vbox), scroll, TRUE, TRUE, 0);
  return left_vbox;
}

static GtkWidget *create_template_layout_right(void) {
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 15);

  GtkWidget *label_moded = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_moded), "<b><span size='large'>Payloads</span></b>");
  gtk_widget_set_halign(label_moded, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(right_vbox), label_moded, FALSE, FALSE, 0);

  gui_ctx.combo_tokens = gtk_combo_box_text_new();
  gtk_box_pack_start(GTK_BOX(right_vbox), gui_ctx.combo_tokens, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(right_vbox), gtk_label_new("Payload Generator:"), FALSE, FALSE, 0);

  combo_gen = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_gen), "Numeric");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_gen), "Simple List");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_gen), "Character Blocks");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_gen), 0);
  gtk_box_pack_start(GTK_BOX(right_vbox), combo_gen, FALSE, FALSE, 0);

  gui_ctx.attack_stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(gui_ctx.attack_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  gtk_stack_add_named(GTK_STACK(gui_ctx.attack_stack), intruder_gui_number_rage_create(), "numeric_page");
  gtk_stack_add_named(GTK_STACK(gui_ctx.attack_stack), gtk_label_new("List Editor..."), "list_page");
  gtk_stack_add_named(GTK_STACK(gui_ctx.attack_stack), gtk_label_new("BoF Config..."), "blocks_page");

  gtk_box_pack_start(GTK_BOX(right_vbox), gui_ctx.attack_stack, TRUE, TRUE, 0);

  g_signal_connect(gui_ctx.combo_tokens, "changed", G_CALLBACK(on_token_selection_changed), NULL);
  g_signal_connect(combo_gen, "changed", G_CALLBACK(on_combo_attack_change), NULL);

  return right_vbox;
}

static GtkWidget *create_template_page(void) {
  GtkWidget *split_layout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

  GtkWidget *left_vbox = create_template_layout_left();
  GtkWidget *right_vbox = create_template_layout_right();
  gtk_paned_pack1(GTK_PANED(split_layout), left_vbox, TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(split_layout), right_vbox, TRUE, FALSE);

  return split_layout;
}

static GtkWidget* create_status_area(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    gui_ctx.progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(gui_ctx.progress_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), gui_ctx.progress_bar, FALSE, FALSE, 0);

    return vbox;
}

void generator_gui_create(GtkWindow *parent) {
  gui_ctx.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(gui_ctx.window), "Advanced Campaign Builder");
  gtk_window_set_default_size(GTK_WINDOW(gui_ctx.window), (gint)(APPLICATION_MIN_WIDTH*0.8), (gint)(APPLICATION_MIN_HEIGHT*0.8));
  gtk_window_set_transient_for(GTK_WINDOW(gui_ctx.window), parent);
  gtk_window_set_position(GTK_WINDOW(gui_ctx.window), GTK_WIN_POS_CENTER);

  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(gui_ctx.window), main_vbox);

  GtkWidget *split_layout = create_template_page();

  gtk_box_pack_start(GTK_BOX(main_vbox), split_layout, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), create_status_area(), FALSE, FALSE, 0);

  gtk_widget_show_all(gui_ctx.window);
  while (gtk_events_pending()) {
    gtk_main_iteration();
  }
}

