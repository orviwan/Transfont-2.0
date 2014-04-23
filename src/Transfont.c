#include "pebble.h"

static Window *window;
static Layer *window_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static GBitmap *colon_image;
static BitmapLayer *colon_layer;

static GBitmap *div_image;
static BitmapLayer *div_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_SUN,
  RESOURCE_ID_IMAGE_DAY_MON,
  RESOURCE_ID_IMAGE_DAY_TUE,
  RESOURCE_ID_IMAGE_DAY_WED,
  RESOURCE_ID_IMAGE_DAY_THU,
  RESOURCE_ID_IMAGE_DAY_FRI,
  RESOURCE_ID_IMAGE_DAY_SAT
};

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

const int MONTH_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MONTH_JAN,
  RESOURCE_ID_IMAGE_MONTH_FEB,
  RESOURCE_ID_IMAGE_MONTH_MAR,
  RESOURCE_ID_IMAGE_MONTH_APR,
  RESOURCE_ID_IMAGE_MONTH_MAY,
  RESOURCE_ID_IMAGE_MONTH_JUN,
  RESOURCE_ID_IMAGE_MONTH_JUL,
  RESOURCE_ID_IMAGE_MONTH_AUG,
  RESOURCE_ID_IMAGE_MONTH_SEP,
  RESOURCE_ID_IMAGE_MONTH_OCT,
  RESOURCE_ID_IMAGE_MONTH_NOV,
  RESOURCE_ID_IMAGE_MONTH_DEC
  
};

static GBitmap *month_name_image;
static BitmapLayer *month_name_layer;

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_SMALL_0,
  RESOURCE_ID_IMAGE_SMALL_1,
  RESOURCE_ID_IMAGE_SMALL_2,
  RESOURCE_ID_IMAGE_SMALL_3,
  RESOURCE_ID_IMAGE_SMALL_4,
  RESOURCE_ID_IMAGE_SMALL_5,
  RESOURCE_ID_IMAGE_SMALL_6,
  RESOURCE_ID_IMAGE_SMALL_7,
  RESOURCE_ID_IMAGE_SMALL_8,
  RESOURCE_ID_IMAGE_SMALL_9
};

#define TOTAL_DATE_DIGITS 2
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

#define TOTAL_YEAR_DIGITS 4
static GBitmap *year_digits_images[TOTAL_YEAR_DIGITS];
static BitmapLayer *year_digits_layers[TOTAL_YEAR_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_BIG_0,
  RESOURCE_ID_IMAGE_BIG_1,
  RESOURCE_ID_IMAGE_BIG_2,
  RESOURCE_ID_IMAGE_BIG_3,
  RESOURCE_ID_IMAGE_BIG_4,
  RESOURCE_ID_IMAGE_BIG_5,
  RESOURCE_ID_IMAGE_BIG_6,
  RESOURCE_ID_IMAGE_BIG_7,
  RESOURCE_ID_IMAGE_BIG_8,
  RESOURCE_ID_IMAGE_BIG_9
};

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
  if (old_image != NULL) {
	gbitmap_destroy(old_image);
	old_image = NULL;
  }
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

unsigned int get_display_year(unsigned int year) {
	return 1900+year;
}

static void update_days(struct tm *tick_time) {
  //DAY NAME
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(3, 7));

  //MONTH NAME
  set_container_image(&month_name_image, month_name_layer, MONTH_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(56, 7));

  //DAY NUMBER
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(116, 7));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(128, 7));

  //YEAR
  unsigned int display_year = get_display_year(tick_time->tm_year);

  int data, i;    
  int split[4];
  for(i=3 ; i>=0 ; i--)
  {
    data = display_year % 10;
    split[i] = data;
    display_year /= 10;
  }

  set_container_image(&year_digits_images[0], year_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[split[0]], GPoint(48, 147));
  set_container_image(&year_digits_images[1], year_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[split[1]], GPoint(62, 147));
  set_container_image(&year_digits_images[2], year_digits_layers[2], DATENUM_IMAGE_RESOURCE_IDS[split[2]], GPoint(76, 147));
  set_container_image(&year_digits_images[3], year_digits_layers[3], DATENUM_IMAGE_RESOURCE_IDS[split[3]], GPoint(90, 147));
}

static void update_hours(struct tm *tick_time) {

  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(6, 70));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(38, 70));
    
  if (!clock_is_24h_style()) {
    if (display_hour/10 == 0) {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
    }
    else {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }
  }
}
static void update_minutes(struct tm *tick_time) {
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(81, 68));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(112, 68));
}
static void update_seconds(struct tm *tick_time) {
	layer_set_hidden(bitmap_layer_get_layer(colon_layer), tick_time->tm_sec%2);
}
static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  }	
  if (units_changed & SECOND_UNIT) {
    update_seconds(tick_time);
  }	
}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  memset(&year_digits_layers, 0, sizeof(year_digits_layers));
  memset(&year_digits_images, 0, sizeof(year_digits_images));

  window = window_create();
  if (window == NULL) {
      return;
  }
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  
  GRect dummy_frame = { {0, 0}, {0, 0} };

  //BACKGROUND
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
 	
  //DIVIDER
  div_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DIV);
  GRect frame = (GRect) {
    .origin = { .x = 108, .y = 13 },
    .size = div_image->bounds.size
  };
  div_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(div_layer, div_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(div_layer));   
  
  //COLON
  colon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLON);
  frame = (GRect) {
    .origin = { .x = 70, .y = 81 },
    .size = colon_image->bounds.size
  };
  colon_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(colon_layer, colon_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(colon_layer)); 

    day_name_layer = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));	

    month_name_layer = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(month_name_layer));	
	
  //TIME
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  
  //DATE
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  
  //YEAR
  for (int i = 0; i < TOTAL_YEAR_DIGITS; ++i) {
    year_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(year_digits_layers[i]));
  }
  
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

}


static void deinit(void) {
  
  tick_timer_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(div_layer));
  bitmap_layer_destroy(div_layer);
  gbitmap_destroy(div_image);
  div_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(colon_layer));
  bitmap_layer_destroy(colon_layer);
  gbitmap_destroy(colon_image);
  colon_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  day_name_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(month_name_layer));
  bitmap_layer_destroy(month_name_layer);
  gbitmap_destroy(month_name_image);
  month_name_image = NULL;
	
	
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
	date_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
	time_digits_layers[i] = NULL;
  }
	
  for (int i = 0; i < TOTAL_YEAR_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(year_digits_layers[i]));
    gbitmap_destroy(year_digits_images[i]);
    bitmap_layer_destroy(year_digits_layers[i]);
	year_digits_layers[i] = NULL;
  }
	
  layer_destroy(window_layer);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}