#include <pebble.h>
#include <inttypes.h>


static Window *window;
static TextLayer *text_time_layer;
static TextLayer *text_date_layer;
static TextLayer *battery_layer;
static TextLayer *connection_layer;
static GBitmap *image1;
static GBitmap *image2;
static GBitmap *image0;
static BitmapLayer *image_layer;
static int seconds_counter = 0;

//static int image_id = 0;

enum {
   CONF_THINGBROKERURL = 0
};

/************ Custom functions ************/

void vibes_custom_pulse() {
  static const uint32_t const segments[] = { 50, 200, 50, 200, 50 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

void handle_image (int id) {
  if (id == 1){
    bitmap_layer_set_bitmap(image_layer, image1);
  }
  if (id == 2){
    bitmap_layer_set_bitmap(image_layer, image2);
  }
  if (id == 0) {
    bitmap_layer_set_bitmap(image_layer, image0);
  }
}

/************ Time, Accel, Battery and Bluetooth handlers ************/

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  
  char* thingbrokerurl_text =  "http://kimberly.magic.ubc.ca:8080/thingbroker";
  if ( axis == ACCEL_AXIS_Z || axis == ACCEL_AXIS_X ) {
    if ( persist_exists(CONF_THINGBROKERURL) ){
      //persist_read_string(CONF_THINGBROKERURL, thingbrokerurl_text, sizeof(thingbrokerurl_text));
      persist_read_string(CONF_THINGBROKERURL, thingbrokerurl_text, 64); //sizeof returns 4, will dig in later to find out why
      
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Size of buffer: %d", sizeof(thingbrokerurl_text) );
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "ThingBroker URL stored size: %d", persist_get_size(CONF_THINGBROKERURL) );
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "ThingBroker URL stored: %s", thingbrokerurl_text );

      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      Tuplet value = TupletCString(CONF_THINGBROKERURL, thingbrokerurl_text);
      //Tuplet value = TupletInteger(1, 42);      
      dict_write_tuplet(iter, &value);
      app_message_outbox_send();

      handle_image(1);
      seconds_counter = 0;
    }
  }

}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "OK" : "  ");
  vibes_custom_pulse();
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100";
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "CH");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

  seconds_counter++;

  if ( seconds_counter > 5 ) {
    handle_image(0);
  }

  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxx 00/00/00";


  char *time_format;

  //update date
  strftime(date_text, sizeof(date_text), "%a %y/%m/%d", tick_time);
  text_layer_set_text(text_date_layer, date_text);

  //update time
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(time_text, sizeof(time_text), time_format, tick_time);
  // Kludge to handle lack of non-padded hour format string (12 hour clock)
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  text_layer_set_text(text_time_layer, time_text);

  //handle battery state  
  handle_battery(battery_state_service_peek()); 


}

void update_face () {
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
}

/************ Communication Handlers ************/

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
}

void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CONF_thingbrokerurl EXISTS %d", persist_exists(CONF_THINGBROKERURL) );

  Tuple *thingbrokerurl_tuple = dict_find(received, CONF_THINGBROKERURL);
  if (thingbrokerurl_tuple) {
    char* value = thingbrokerurl_tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ThingBroker URL received from phone: %s", value);
    persist_write_string(CONF_THINGBROKERURL, value);
  }
  update_face();
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
}


/************ App initialization and finalization ************/

static void window_load(Window *window) {

  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer); //get boundaries of window

  //image
  image1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_1);
  image2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_2);
  image0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_0);
  image_layer = bitmap_layer_create(GRect(10, 23, 124, 124));
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  handle_image(0);

  //time
  text_time_layer = text_layer_create((GRect){ .origin={0,-7}, .size={bounds.size.w,24 } });
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  //bluetooth connection
  connection_layer = text_layer_create(GRect(0, 150, /* width */ bounds.size.w/5, 15 /* height */));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentLeft);
  handle_bluetooth(bluetooth_connection_service_peek());
  layer_add_child(window_layer, text_layer_get_layer(connection_layer));

  //date
  text_date_layer = text_layer_create((GRect){ .origin={bounds.size.w/5,150}, .size={(bounds.size.w/5)*3,15 } });
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  //battery status
  battery_layer = text_layer_create(GRect((bounds.size.w/4)*3, 150, /* width */ bounds.size.w/5, 15 /* height */));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);  
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  accel_tap_service_subscribe(&accel_tap_handler);

  //register incoming message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  update_face();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_layer);
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
  gbitmap_destroy(image1);
  gbitmap_destroy(image2);
  bitmap_layer_destroy(image_layer);
}

static void init(void) {
  window = window_create();
  //window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true /*animated*/); 
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
