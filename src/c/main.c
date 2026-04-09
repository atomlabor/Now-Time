#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_header_layer, *s_time_layer, *s_date_layer, *s_music_layer, *s_health_layer;
static Layer *s_canvas_layer;
static char s_time_buf[32], s_date_buf[32], s_music_buf[256], s_health_buf[64], s_fav_song[64];
static AppTimer *s_music_timer = NULL, *s_scroll_timer = NULL;
static GColor s_accent_color;
static int s_battery_level, s_scroll_pos = 0;
static bool s_vibrate_enabled = true;

static void hide_music_info(void *data);

static uint32_t get_tuple_int(Tuple *t) {
  if (!t) return 0;
  if (t->type == TUPLE_INT) {
    if (t->length == 1) return (uint32_t)t->value->int8;
    else if (t->length == 2) return (uint32_t)t->value->int16;
    else if (t->length == 4) return (uint32_t)t->value->int32;
  } else if (t->type == TUPLE_UINT) {
    if (t->length == 1) return (uint32_t)t->value->uint8;
    else if (t->length == 2) return (uint32_t)t->value->uint16;
    else if (t->length == 4) return t->value->uint32;
  }
  return 0;
}

static void update_health() {
#if defined(PBL_HEALTH)
  int steps = (int)health_service_sum_today(HealthMetricStepCount);
  int hr = (int)health_service_peek_current_value(HealthMetricHeartRateRawBPM);
  snprintf(s_health_buf, sizeof(s_health_buf), "> Stats: %d | %d", steps, hr);
#else
  snprintf(s_health_buf, sizeof(s_health_buf), " ");
#endif
  if (s_health_layer) text_layer_set_text(s_health_layer, s_health_buf);
}

static void scroll_handler(void *data) {
  if (layer_get_hidden(text_layer_get_layer(s_music_layer))) {
    s_scroll_timer = NULL;
    return;
  }
  s_scroll_pos++;
  if (s_scroll_pos > (int)strlen(s_music_buf) - 5) s_scroll_pos = 0;
  text_layer_set_text(s_music_layer, &s_music_buf[s_scroll_pos]);
  s_scroll_timer = app_timer_register(350, scroll_handler, NULL);
}

static void hide_music_info(void *data) {
  s_music_timer = NULL;
  if (s_music_layer && s_date_layer) {
    layer_set_hidden(text_layer_get_layer(s_music_layer), true);
    layer_set_hidden(text_layer_get_layer(s_date_layer), false);
  }
}

static void show_music_info() {
  if (!s_music_layer || !s_date_layer) return;
  layer_set_hidden(text_layer_get_layer(s_date_layer), true);
  layer_set_hidden(text_layer_get_layer(s_music_layer), false);
  s_scroll_pos = 0;
  text_layer_set_text(s_music_layer, s_music_buf);
  
  if (s_music_timer) app_timer_cancel(s_music_timer);
  s_music_timer = app_timer_register(12000, hide_music_info, NULL);
  
  if (s_scroll_timer) app_timer_cancel(s_scroll_timer);
  s_scroll_timer = app_timer_register(350, scroll_handler, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  GRect u = layer_get_unobstructed_bounds(layer);
  int w = b.size.w; int h = b.size.h;

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorLightGray));
  graphics_fill_rect(ctx, b, 15, GCornersAll);
  
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_draw_line(ctx, GPoint(5, 3), GPoint(w - 6, 3));
  graphics_draw_line(ctx, GPoint(3, 5), GPoint(3, h - 6));
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(w - 3, 10), GPoint(w - 3, h - 3));

  int lcd_h = (h * 52) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack); 
  graphics_fill_rect(ctx, GRect(8, 8, w - 16, lcd_h + 4), 10, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorWhite); 
  graphics_fill_rect(ctx, GRect(10, 10, w - 20, lcd_h), 6, GCornersAll);

  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_draw_rect(ctx, GRect(25, lcd_h + 1, w - 50, 6)); 
  graphics_context_set_fill_color(ctx, s_accent_color);
  graphics_fill_rect(ctx, GRect(26, lcd_h + 2, ((w - 52) * s_battery_level) / 100, 4), 0, GCornerNone);

  if (u.size.h >= h) {
    GPoint center = GPoint(w / 2, (lcd_h + 13) + ((h - (lcd_h + 15)) / 2));
    int r = ((h - (lcd_h + 15)) / 2) - 4; if (r > 42) r = 42;
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, r + 1); 
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
    graphics_fill_circle(ctx, center, r);
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorWhite, GColorLightGray));
    graphics_fill_circle(ctx, center, r / 3);
    
    graphics_context_set_text_color(ctx, GColorDarkGray);
    GFont label_f = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    graphics_draw_text(ctx, "MENU", label_f, GRect(center.x - 25, center.y - r + 1, 50, 16), 0, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "> ||", label_f, GRect(center.x - 20, center.y + r - 18, 40, 16), 0, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "<<", label_f, GRect(center.x - r + 2, center.y - 11, 30, 16), 0, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, ">>", label_f, GRect(center.x + r - 32, center.y - 11, 30, 16), 0, GTextAlignmentRight, NULL);
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *track_t = dict_find(iterator, MESSAGE_KEY_ID_TRACK_NAME);
  Tuple *fav_t = dict_find(iterator, MESSAGE_KEY_ID_FAV_SONG);
  Tuple *color_t = dict_find(iterator, MESSAGE_KEY_ID_HEADER_COLOR);
  Tuple *vibrate_t = dict_find(iterator, MESSAGE_KEY_ID_VIBRATE);

  if (vibrate_t) {
    s_vibrate_enabled = (get_tuple_int(vibrate_t) == 1);
    persist_write_bool(MESSAGE_KEY_ID_VIBRATE, s_vibrate_enabled);
  }

  if (color_t) {
    uint32_t raw_color = get_tuple_int(color_t);
    s_accent_color = GColorFromHEX(raw_color);
    if (s_header_layer) text_layer_set_background_color(s_header_layer, s_accent_color);
    persist_write_int(MESSAGE_KEY_ID_HEADER_COLOR, (int)raw_color);
    layer_mark_dirty(s_canvas_layer);
  }
  
  if (fav_t) {
    snprintf(s_fav_song, sizeof(s_fav_song), "%s", fav_t->value->cstring);
    persist_write_string(MESSAGE_KEY_ID_FAV_SONG, s_fav_song);
    
    if (!track_t) {
      snprintf(s_music_buf, sizeof(s_music_buf), "> Fav: %s        ", s_fav_song);
      text_layer_set_text(s_music_layer, s_music_buf);
    }
  }
  
  if (track_t) {
    if (strlen(track_t->value->cstring) > 0) {
      snprintf(s_music_buf, sizeof(s_music_buf), "> Now: %s        ", track_t->value->cstring);
      text_layer_set_text(s_music_layer, s_music_buf);
      show_music_info();
      
      if (s_vibrate_enabled) {
        vibes_short_pulse();
      }
    } else {
      snprintf(s_music_buf, sizeof(s_music_buf), "> Fav: %s        ", s_fav_song);
      text_layer_set_text(s_music_layer, s_music_buf);
    }
  }
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  strftime(s_time_buf, sizeof(s_time_buf), "> Time: %H:%M", t);
  int w = layer_get_bounds(window_get_root_layer(s_main_window)).size.w;
  strftime(s_date_buf, sizeof(s_date_buf), (w > 160) ? "> Date: %d.%m.%Y" : "> Date: %d.%m.%y", t);
  text_layer_set_text(s_time_layer, s_time_buf);
  text_layer_set_text(s_date_layer, s_date_buf);
  update_health();
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}

#if !defined(PBL_PLATFORM_APLITE)
static void unobstructed_change_handler(AnimationProgress progress, void *context) {
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}
#endif

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect b = layer_get_bounds(window_layer);
  
  if (persist_exists(MESSAGE_KEY_ID_VIBRATE)) {
    s_vibrate_enabled = persist_read_bool(MESSAGE_KEY_ID_VIBRATE);
  }
  s_accent_color = persist_exists(MESSAGE_KEY_ID_HEADER_COLOR) ? GColorFromHEX(persist_read_int(MESSAGE_KEY_ID_HEADER_COLOR)) : GColorDukeBlue;
  if (persist_exists(MESSAGE_KEY_ID_FAV_SONG)) persist_read_string(MESSAGE_KEY_ID_FAV_SONG, s_fav_song, sizeof(s_fav_song));
  else snprintf(s_fav_song, sizeof(s_fav_song), "Wonderwall");
  snprintf(s_music_buf, sizeof(s_music_buf), "> Fav: %s        ", s_fav_song);

  s_canvas_layer = layer_create(b);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  s_header_layer = text_layer_create(GRect(10, 10, b.size.w - 20, 24));
  text_layer_set_background_color(s_header_layer, s_accent_color);
  text_layer_set_text_color(s_header_layer, GColorWhite);
  text_layer_set_text(s_header_layer, "NOW: TIME");
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  GFont f = fonts_get_system_font(b.size.w > 160 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_14_BOLD);
  int row = (b.size.h > 170) ? 23 : 18; int sy = (b.size.h > 170) ? 36 : 34;

  s_time_layer = text_layer_create(GRect(20, sy, b.size.w - 40, row));
  s_health_layer = text_layer_create(GRect(20, sy + row, b.size.w - 40, row));
  s_date_layer = text_layer_create(GRect(20, sy + (row * 2), b.size.w - 40, row));
  s_music_layer = text_layer_create(GRect(20, sy + (row * 2), b.size.w - 40, row));

  TextLayer *layers[] = {s_time_layer, s_health_layer, s_date_layer, s_music_layer};
  for(int i=0; i<4; i++) {
    text_layer_set_background_color(layers[i], GColorWhite); 
    text_layer_set_text_color(layers[i], GColorBlack);
    text_layer_set_font(layers[i], f);
    layer_add_child(window_layer, text_layer_get_layer(layers[i]));
  }
  text_layer_set_text(s_music_layer, s_music_buf);
  layer_set_hidden(text_layer_get_layer(s_music_layer), true);
  update_time();
}

static void main_window_unload(Window *window) {
  if (s_music_timer) app_timer_cancel(s_music_timer);
  if (s_scroll_timer) app_timer_cancel(s_scroll_timer);

  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_music_layer);
  text_layer_destroy(s_health_layer);
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) { 
    .load = main_window_load, 
    .unload = main_window_unload 
  });
  window_stack_push(s_main_window, true);
  
  #if !defined(PBL_PLATFORM_APLITE)
  unobstructed_area_service_subscribe((UnobstructedAreaHandlers){.change=unobstructed_change_handler}, NULL);
  #endif
  
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)update_time);
  accel_tap_service_subscribe((AccelTapHandler)show_music_info);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(1024, 1024);
}

static void deinit() {
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  
  #if !defined(PBL_PLATFORM_APLITE)
  unobstructed_area_service_unsubscribe();
  #endif
  
  window_destroy(s_main_window);
}

int main(void) { 
  init(); 
  app_event_loop(); 
  deinit(); 
}