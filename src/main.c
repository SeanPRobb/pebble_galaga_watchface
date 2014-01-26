#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID {0x59, 0x1C, 0x11, 0x75, 0xC4, 0xC8, 0x35, 0xF7, 0x9F, 0x4B, 0xEA, 0x13, 0x25, 0x9D, 0x0A, 0x10}
PBL_APP_INFO(MY_UUID, "Space Invader", "Work Hoodie", 0x1, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

	
Window window;

bool init;

RotBmpContainer rotHourContainer;

//BmpContainer hourContainer;
//BmpContainer secContainer;

//BmpContainer image_containers
#define NUMBER_OF_IMAGES 12

// These images are 59 x 79 pixels,
// black and white with the digit character centered in the image.
const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  	RESOURCE_ID_IMAGE_SHIP_1, RESOURCE_ID_IMAGE_SHIP_2,
  	RESOURCE_ID_IMAGE_SHIP_3, RESOURCE_ID_IMAGE_SHIP_4,
	RESOURCE_ID_IMAGE_SHIP_5, RESOURCE_ID_IMAGE_SHIP_6,
	RESOURCE_ID_IMAGE_SHIP_7, RESOURCE_ID_IMAGE_SHIP_8,
  	RESOURCE_ID_IMAGE_SHIP_9,RESOURCE_ID_IMAGE_SHIP_10,
	RESOURCE_ID_IMAGE_SHIP_11,RESOURCE_ID_IMAGE_SHIP_12
};

void set_hand_angle(RotBmpContainer *hand_image_container, unsigned int hand_angle)
{
	
  signed short x_fudge = 0;
  signed short y_fudge = 0;


  hand_image_container->layer.rotation =  TRIG_MAX_ANGLE * hand_angle / 360;

  //
  // Due to rounding/centre of rotation point/other issues of fitting
  // square pixels into round holes by the time hands get to 6 and 9
  // o'clock there's off-by-one pixel errors.
  //
  // The `x_fudge` and `y_fudge` values enable us to ensure the hands
  // look centred on the minute marks at those points. (This could
  // probably be improved for intermediate marks also but they're not
  // as noticable.)
  //
  // I think ideally we'd only ever calculate the rotation between
  // 0-90 degrees and then rotate again by 90 or 180 degrees to
  // eliminate the error.
  //
  if (hand_angle == 180) {
    x_fudge = -1;
  } else if (hand_angle == 270) {
    y_fudge = -1;
  }

  // (144 = screen width, 168 = screen height)
  hand_image_container->layer.layer.frame.origin.x = (144/2) - (hand_image_container->layer.layer.frame.size.w/2) + x_fudge;
  hand_image_container->layer.layer.frame.origin.y = (168/2) - (hand_image_container->layer.layer.frame.size.h/2) + y_fudge;

  layer_mark_dirty(&hand_image_container->layer.layer);
}

unsigned short get_display_hour(unsigned short hour) {

//  if (clock_is_24h_style()) {
//    return hour;
//  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}

void load_ship_image(int digit_value) {
  /*

     Loads the digit image from the application's resources and
     displays it on-screen in the correct location.

     Each slot is a quarter of the screen.

   */

  // TODO: Signal these error(s)?

  GRect bounds = layer_get_bounds(window_get_root_layer(&window));
  const GPoint center = grect_center_point(&bounds);
	
//  bmp_init_container(IMAGE_RESOURCE_IDS[digit_value - 1], &hourContainer);
//  hourContainer.layer.layer.frame.origin.x =  center.x-30;
//  hourContainer.layer.layer.frame.origin.y = center.y-36;
//  layer_add_child(&window.layer, &hourContainer.layer.layer);
	
  rotbmp_init_container(IMAGE_RESOURCE_IDS[digit_value - 1],&rotHourContainer);
//  rotHourContainer.layer.layer.frame.origin.x =  center.x - 30;
//  rotHourContainer.layer.layer.frame.origin.y = center.y - 36;
  layer_add_child(&window.layer, &rotHourContainer.layer.layer);	
}


void unload_ship_image() {
  /*

     Removes the digit from the display and unloads the image resource
     to free up RAM.

     Can handle being called on an already empty slot.

   */
	
//    layer_remove_from_parent(&hourContainer.layer.layer);
//    bmp_deinit_container(&hourContainer);
	
    layer_remove_from_parent(&rotHourContainer.layer.layer);
    rotbmp_deinit_container(&rotHourContainer);

}

void update_ship_hour_hand(unsigned short value)
{
	if (init)
	{
    unload_ship_image();
	}
	  
    load_ship_image(value);
}

void update_angle_min_hand(PblTm *tick_time)
{
	set_hand_angle(&rotHourContainer, ((tick_time->tm_hour % 12) * 30) + (tick_time->tm_min/2));
}

void display_time(PblTm *tick_time) {

  // TODO: Use `units_changed` and more intelligence to reduce
  //       redundant digit unload/load?

  update_ship_hour_hand(get_display_hour(tick_time->tm_hour));
  update_angle_min_hand(tick_time);
  //display_value(tick_time->tm_min, 1, true);
}

void handle_tick(AppContextRef params, PebbleTickEvent *t)
{
	 // Avoids a blank screen on watch start.
 	PblTm tick_time;

  	get_time(&tick_time);
  	display_time(&tick_time);
}

void handle_init() {
	init = false;
	window_init(&window,"Space Invader");
	window_stack_push(&window, true /* Animated */);
	
  	window_set_background_color(&window, GColorBlack);
    
    resource_init_current_app(&APP_RESOURCES);
	
	
	
	 // Avoids a blank screen on watch start.
 	PblTm tick_time;

  	get_time(&tick_time);
 	display_time(&tick_time);
	
    init = true;    
}

void handle_deinit() {
	unload_ship_image();
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    }
  };
	  
  app_event_loop(params, &handlers);
}