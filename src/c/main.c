#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_steps_layer;
static Layer *s_battery_layer;

static GFont s_time_font, s_date_font, s_steps_font;
// static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static BitmapLayer *s_bt_icon_layer, *s_walking_man_layer;
// static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static GBitmap *s_bt_icon_bitmap, *s_walking_man_bitmap;

static int s_battery_level;

/*
  vibes pattern
*/
// Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
static uint32_t const segments[] = { 200, 100, 200, 100, 200, 400, 500 };
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};

void updateSteps(){
  HealthMetric metric = HealthMetricStepCount;

  time_t end = time(NULL);
  time_t start = time_start_of_today();
  time_t oneHour = end - SECONDS_PER_HOUR;

  static char date_buffer[26];

  // Check data is available
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, oneHour, end);
  if(result & HealthServiceAccessibilityMaskAvailable) {
    // Data is available! Read it
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps in the last hour: %d", (int)health_service_sum(metric, oneHour, end));
    // if the # of steps in the last hour is less then 500
    // change the color of the layer
    if( (int)health_service_sum(metric, oneHour, end) < 180 ){
      layer_set_hidden(bitmap_layer_get_layer(s_walking_man_layer), false);

//      text_layer_set_background_color(s_steps_layer, GColorRed);
//      text_layer_set_text_color(s_steps_layer, GColorBlack);
    }else{
      layer_set_hidden(bitmap_layer_get_layer(s_walking_man_layer), true);
//      text_layer_set_background_color(s_steps_layer, GColorClear);      
//      text_layer_set_text_color(s_steps_layer, GColorPastelYellow);
    }
    text_layer_set_background_color(s_steps_layer, GColorClear);      
    text_layer_set_text_color(s_steps_layer, GColorPastelYellow);

    snprintf( date_buffer, sizeof(date_buffer), "T %d H %d", (int)health_service_sum_today(metric), 
                                                             (int)health_service_sum(metric, oneHour, end) );
    text_layer_set_text(s_steps_layer, date_buffer);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No data available!");
  }
  
}
/*
  health system 20170212
  */
static void health_handler(HealthEventType event, void *context) {
  
  // Which type of event occurred?
  switch(event) {
    case HealthEventMetricAlert:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMetricAlert event");
      break;
    case HealthEventSignificantUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSignificantUpdate event");
      updateSteps();
    break;
    case HealthEventMovementUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMovementUpdate event");
      updateSteps();
      break;
    case HealthEventSleepUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSleepUpdate event");
      break;
    case HealthEventHeartRateUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO,
              "New HealthService HealthEventHeartRateUpdate event");
      break;
  }
}  
  

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
  
  // in case of state change vibes and sets the background color
  if(!connected) {
//    vibes_double_pulse();
    vibes_enqueue_custom_pattern(pat);
    window_set_background_color(s_main_window, GColorRed);
  }else{
    vibes_double_pulse();
    window_set_background_color(s_main_window, GColorDukeBlue);
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer, and show the time
  static char buffer[] = "00:00";
  if(clock_is_24h_style()) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, buffer);
  
  // Show the date
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);
  
  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, GCornerNone, 0);
  
  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), GCornerNone, 0);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  //Create GBitmap, then set to created BitmapLayer
  /*
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  */
  // Create GFonts
//  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ASO_48));
//  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  s_time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  
//  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ASO_20));
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_steps_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorPastelYellow);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 132, 144, 30));
  text_layer_set_text_color(s_date_layer, GColorPastelYellow);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "Sept 23");
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Create steps TextLayer
  s_steps_layer = text_layer_create(GRect(10, 104, 144-20, 30));
  text_layer_set_text_color(s_steps_layer, GColorPastelYellow);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentCenter);
  text_layer_set_text(s_steps_layer, "steps: --");
  text_layer_set_font(s_steps_layer, s_steps_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_steps_layer));

  
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(14, 54, 115, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
  s_walking_man_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WALKING_MAN);
  
  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(GRect(59, 12, 30, 30));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  
  s_walking_man_layer = bitmap_layer_create(GRect(10, 10, 30, 32));
  bitmap_layer_set_bitmap(s_walking_man_layer, s_walking_man_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_walking_man_layer));

  // Initialize the display
  update_time();
  battery_callback(battery_state_service_peek());

  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  // Subscrive the health service
  #if defined(PBL_HEALTH)
    // Attempt to subscribe 
    if(!health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health NOT available!");
    }
    #else
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health available!");
  #endif
      
}

static void main_window_unload(Window *window) {
//  fonts_unload_custom_font(s_time_font);
//  fonts_unload_custom_font(s_date_font);
  
  // gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_walking_man_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);

  // bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  bitmap_layer_destroy(s_walking_man_layer);
  
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  
  layer_destroy(s_battery_layer);
}
  
static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  window_set_background_color(s_main_window, GColorDukeBlue);
  
  // Register with Event Services
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
