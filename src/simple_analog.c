#include "simple_analog.h"

#include "pebble.h"

static Window *window;
static Layer *s_hands_layer;

static GBitmap *s_hlt_background_bitmap;
static GBitmap *s_hlt_background_bitmap_nobt;
static BitmapLayer *s_hlt_background_layer;

static GPath *s_minute_arrow, *s_hour_arrow;

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // minute/hour hand
  #ifdef PBL_PLATFORM_APLITE
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #elif PBL_PLATFORM_BASALT
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
  #endif


  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);

  // dot in the middle
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void bt_handler(bool connected) {
  if (connected) {
    bitmap_layer_set_bitmap(s_hlt_background_layer, s_hlt_background_bitmap);
  } else {
    bitmap_layer_set_bitmap(s_hlt_background_layer, s_hlt_background_bitmap_nobt);
  }
  layer_mark_dirty(bitmap_layer_get_layer(s_hlt_background_layer));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);  

  s_hlt_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_HLTPBL);
  s_hlt_background_bitmap_nobt = gbitmap_create_with_resource(RESOURCE_ID_IMG_HLTPBL_NOBT);
  s_hlt_background_layer = bitmap_layer_create(bounds);
  if (bluetooth_connection_service_peek()) {
    bitmap_layer_set_bitmap(s_hlt_background_layer, s_hlt_background_bitmap);
  } else {
    bitmap_layer_set_bitmap(s_hlt_background_layer, s_hlt_background_bitmap_nobt);
  }
  #ifdef PBL_PLATFORM_APLITE
    bitmap_layer_set_compositing_mode(s_hlt_background_layer, GCompOpAssign);
  #elif PBL_PLATFORM_BASALT
    bitmap_layer_set_compositing_mode(s_hlt_background_layer, GCompOpSet);
  #endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_hlt_background_layer));
  layer_mark_dirty(bitmap_layer_get_layer(s_hlt_background_layer));

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_hlt_background_layer);
  layer_destroy(s_hands_layer);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
  bluetooth_connection_service_subscribe(bt_handler);
}

static void deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  gbitmap_destroy(s_hlt_background_bitmap);

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
