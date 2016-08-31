#include <pebble.h>
#include "main.h"  
#include "kiezelpay.h"

static Window *window;
static TextLayer *textlayer;
time_t next;

int buzz_intensity, buzz_interval, buzz_start;
int enable_quiet_time, quiet_time_start_hour, quiet_time_end_hour;

// Create and add app glance slices...
#ifndef PBL_PLATFORM_APLITE
static void prv_update_app_glance(AppGlanceReloadSession *session, size_t limit, void *context) {
  
  // This should never happen, but developers should always ensure they arenot adding more slices than are available
  if (limit < 1) return;
  
  //to store next wake up message
  char message[25];
  struct tm *next_tm = localtime(&next);
  bool status = (bool)context;
  
  if (status) {
    strftime(message, sizeof(message), "Next: %b-%d %H:%M", next_tm);
  } else {
    snprintf(message, sizeof(message), "Disabled");
  }
  
  // Create the AppGlanceSlice
  // NOTE: When .icon is not set, the app's default icon is used
  const AppGlanceSlice entry = (AppGlanceSlice) {
    .layout = {
      //.icon = ((PublishedId)RESOURCE_ID_MENU),
      .subtitle_template_string = message
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };

  // Add the slice, and check the result
  const AppGlanceResult result = app_glance_add_slice(session, entry);

  if (result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
  }
  
}
#endif



// schedules next wake up and does current buzz
static void schedule_and_buzz() {
  
  //canceling and resubscribing to wakeup
  vibes_cancel();
  wakeup_cancel_all();
  wakeup_service_subscribe(NULL);
    
  //getting current time
  if (persist_exists(KEY_NEXT_TIME)){ //if stored time exist - read it
     next = persist_read_int(KEY_NEXT_TIME);
  } else { // otherwise, for the first time - read system time
     next  = time(NULL);  
  }
  
  
  // if inital call is to start at specific hour time - calculate that time
  if (buzz_start != START_IMMEDIATLY) {
    
      int period;
      switch (buzz_start){
        case START_ON_15MIN: period = 15; break;
        case START_ON_HALFHOUR: period = 30; break;
        case START_ON_HOUR: period = 60; break;
        default: period = 60; break;
      }
    
      next = (next / 60) * 60; // rounding to a minute (removing seconds)
      next = next - (next % (period * 60)) + (period * 60); // calculating exact start timing    
    
      //and after that there will be regular wakeup call
      persist_write_int(KEY_BUZZ_START, START_IMMEDIATLY);
      buzz_start = START_IMMEDIATLY;
    
  } else { // otherwise scheduling next call according to inteval
      next = next + buzz_interval*60;  
  }
  
  
  time_t t = time(NULL);
  struct tm now = *(localtime(&t)); 
  
// APP_LOG(APP_LOG_LEVEL_INFO, "QT Enabled = %d, start = %d, end = %d", enable_quiet_time, quiet_time_start_hour, quiet_time_end_hour);
// APP_LOG(APP_LOG_LEVEL_INFO, "Now Hour = %d", now.tm_hour);
  
  //only buzzing if quiet time is disabled or if we're outside quiet time (this is checked only if we're licensed)
  if ((enable_quiet_time == NO_DISABLE) || (
    
    (quiet_time_end_hour > quiet_time_start_hour && (now.tm_hour < quiet_time_start_hour || now.tm_hour >= quiet_time_end_hour)) // within same day t:15; 16-17
    ||
    (now.tm_hour >= quiet_time_end_hour && now.tm_hour < quiet_time_start_hour) // crossing to the next day: t:22; 23-6
  
  )) 
  {
    
      //buzzing
      switch(buzz_intensity){
        case BUZZ_SHORT:
          vibes_short_pulse();
          break;
        case BUZZ_LONG:
          vibes_short_pulse();
          break;
        case BUZZ_DOUBLE:
          vibes_double_pulse();
          break;
        case BUZZ_SUPER_MARIO:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {125,75,125,275,200,275,125,75,125,275,200,600,200,600},
          	.num_segments = 14
          });          
         break;  
        case BUZZ_TMNT:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {75,75,75,75,75,75,75,75,150,150,150,450,75,75,75,75,75,525},
          	.num_segments = 18
          });          
         break;  
        case BUZZ_VOLTRON:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {250,200,150,150,100,50,450,450,150,150,100,50,900,2250},
          	.num_segments = 14
          });          
         break;  
        case BUZZ_FINAL_FANTASY:
           vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {50,100,50,100,50,100,400,100,300,100,350,50,200,100,100,50,600},
          	.num_segments = 17
          });          
         break;  
        case BUZZ_STAR_WARS:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {500,110,500,110,450,110,200,110,170,40,450,110,200,110,170,40,500},
          	.num_segments = 17
          });          
         break;  
        case BUZZ_POWER_RANGERS:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {50,150,150,150,75,75,150,150,150,150,450},
          	.num_segments = 11
          });          
         break;  
        case BUZZ_JAMES_BOND:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {200,100,200,275,425,100,200,100,200,275,425,100,75,25,75,125,75,25,75,125,100,100},
          	.num_segments = 22
          });          
         break;  
        case BUZZ_MORTAL_COMBAT:
          vibes_enqueue_custom_pattern( (VibePattern){
          	.durations = (uint32_t []) {100,200,100,200,100,200,100,200,100,100,100,100,100,200,100,200,100,200,100,200,100,100,100,100,100,200,100,200,100,200,100,200,100,100,100,100,100,100,100,100,100,100,50,50,100,800},
          	.num_segments = 46
          });          
         break;  
      }
    
  }  
  
  //updating appglance to reflect new wake up time
  #ifndef PBL_PLATFORM_APLITE
  app_glance_reload(prv_update_app_glance, (void *)true); //second param - schedule enabled
  #endif

  // scheduling next wakeup
  persist_write_int(KEY_NEXT_TIME, next);
  wakeup_schedule(next, 0, false);  
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_dropped_callback()");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "outbox_failed_callback()");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "outbox_sent_callback()");
}

static bool kiezelpay_event_callback(kiezelpay_event e, void* extra_data) {
  switch (e) {
    case KIEZELPAY_ERROR:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_ERROR");
      break;
    case KIEZELPAY_BLUETOOTH_UNAVAILABLE:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_BLUETOOTH_UNAVAILABLE");
      break;
    case KIEZELPAY_INTERNET_UNAVAILABLE:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_INTERNET_UNAVAILABLE");
      break;
#if KIEZELPAY_DISABLE_TIME_TRIAL == 0
    case KIEZELPAY_TRIAL_STARTED:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_TRIAL_STARTED");
      break;
#endif
    case KIEZELPAY_TRIAL_ENDED:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_TRIAL_ENDED");
      break;
    case KIEZELPAY_CODE_AVAILABLE:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_CODE_AVAILABLE");
      break;
    case KIEZELPAY_PURCHASE_STARTED:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_PURCHASE_STARTED");
      break;
    case KIEZELPAY_LICENSED:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(): KIEZELPAY_LICENSED");
      break;
    default:
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "kiezelpay_event_callback(); unknown event");
      break;
  };
  
  //return true;   //prevent the kiezelpay lib from showing messages by signaling it that we handled the event ourselves
  return false;    //let the kiezelpay lib handle the event
}


// handle configuration change
static void in_recv_handler(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  
  bool keep_window = 0;

  while (t)  {

    switch(t->key)    {

      // when user is unlocking premium feature - initiate payment
      case KEY_UNLOCK:
           keep_window = 1;
           kiezelpay_end_trial();
           break;
      
    
      case KEY_BUZZ_INTENSITY:
           persist_write_int(KEY_BUZZ_INTENSITY, t->value->uint8);
           buzz_intensity = t->value->uint8;
           break;
      case KEY_BUZZ_INTERVAL:
           persist_write_int(KEY_BUZZ_INTERVAL, t->value->uint8);
           buzz_interval = t->value->uint8;
           break;
      case KEY_BUZZ_START:
           persist_write_int(KEY_BUZZ_START, t->value->uint8);
           buzz_start = t->value->uint8;
           break;      
      case KEY_ENABLE_QUIET_TIME:
           persist_write_int(KEY_ENABLE_QUIET_TIME, t->value->uint8);
           enable_quiet_time = t->value->uint8;
           break; 
      case KEY_QUIET_TIME_START_HOUR:
           persist_write_int(KEY_QUIET_TIME_START_HOUR, t->value->uint8);
           quiet_time_start_hour = t->value->uint8;
           break; 
      case KEY_QUIET_TIME_END_HOUR:
           persist_write_int(KEY_QUIET_TIME_END_HOUR, t->value->uint8);
           quiet_time_end_hour = t->value->uint8;
           break; 
    }    
    
    t = dict_read_next(iterator);
  }
  
  //removing window causing app exit;
  if (keep_window == 0) {
    window_stack_pop(false);
  }  
  
}  

static void init() {
  
  // reading stored values
  buzz_intensity = persist_exists(KEY_BUZZ_INTENSITY)? persist_read_int(KEY_BUZZ_INTENSITY) : BUZZ_SHORT;
  buzz_interval = persist_exists(KEY_BUZZ_INTERVAL)? persist_read_int(KEY_BUZZ_INTERVAL) : 60;
  buzz_start = persist_exists(KEY_BUZZ_START)? persist_read_int(KEY_BUZZ_START) : START_ON_HOUR;
  
  enable_quiet_time = persist_exists(KEY_ENABLE_QUIET_TIME)? persist_read_int(KEY_ENABLE_QUIET_TIME) : NO_DISABLE;
  quiet_time_start_hour = persist_exists(KEY_QUIET_TIME_START_HOUR)? persist_read_int(KEY_QUIET_TIME_START_HOUR) : 23;
  quiet_time_end_hour = persist_exists(KEY_QUIET_TIME_END_HOUR)? persist_read_int(KEY_QUIET_TIME_END_HOUR) : 6;
  
  // if it's not a wake up call, meaning we're launched from Config - display current config
  if ( launch_reason() != APP_LAUNCH_WAKEUP) { 
    
      /* begin KiezelPay init */
      /*
      kiezelpay_settings.messaging_inbox_size = 1024;    //when receiving larger appmessages then the size kiezelpay uses
      kiezelpay_settings.messaging_outbox_size = 1024;   //when sending larger appmessages then the size kiezelpay uses
      */
      kiezelpay_settings.on_kiezelpay_event = kiezelpay_event_callback;
      
      //receive your own appmessages, will be forwarded by KiezelPay lib
      kiezelpay_settings.on_inbox_received = in_recv_handler;
      kiezelpay_settings.on_inbox_dropped = inbox_dropped_callback;
      kiezelpay_settings.on_outbox_failed = outbox_failed_callback;
      kiezelpay_settings.on_outbox_sent = outbox_sent_callback;
      
      kiezelpay_init();
      /* end KiezelPay init */
    
    
      window = window_create();
      window_set_background_color(window, GColorBlack);
      GRect bounds = layer_get_bounds(window_get_root_layer(window));
    
      #ifdef PBL_RECT
        textlayer = text_layer_create(GRect(bounds.origin.x + 10, bounds.origin.y + 20, bounds.size.w -20, bounds.size.h - 20));
      #else
        textlayer = text_layer_create(GRect(bounds.origin.x + 15, bounds.origin.y + 35, bounds.size.w - 30, bounds.size.h -30));
      #endif
      text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      text_layer_set_background_color(textlayer, GColorBlack);
      text_layer_set_text_color(textlayer, GColorWhite);
      text_layer_set_text(textlayer, "Please configure the Nag on your phone and tap 'Set' to start the app. This screen will automatically disppear.");
      text_layer_set_text_alignment(textlayer, GTextAlignmentCenter);
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(textlayer));
    
      window_stack_push(window, false);
        
  } else { // if it is a wake up call - buzz and reschedule
     schedule_and_buzz();
  }  
}

static void deinit() {
  
  if (window) { // if UI existed - destroy it and schedule first buzz
    kiezelpay_deinit();
    persist_delete(KEY_NEXT_TIME); // upon config close delete saved time so upon 1st start new system one can be read
    text_layer_destroy(textlayer);
    window_destroy(window);
    
    if (buzz_intensity != BUZZ_DISABLED) { //if we did not disable buzz - schedule wakeup
      schedule_and_buzz();  
    } else { // otherwise cancel all wake ups
        vibes_cancel();
        wakeup_cancel_all();
      
        //updating appglance to show disabled status
        #ifndef PBL_PLATFORM_APLITE
        app_glance_reload(prv_update_app_glance, (void *)false); //second param - schedule disabled
        #endif
    }
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}