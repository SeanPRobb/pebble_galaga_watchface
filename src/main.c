#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID {0x59, 0x1C, 0x11, 0x75, 0xC4, 0xC8, 0x35, 0xF7, 0x9F, 0x4B, 0xEA, 0x13, 0x25, 0x9D, 0x0A, 0x10}
PBL_APP_INFO(MY_UUID, "Space Invader", "Work Hoodie", 0x1, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

	
Window window;

bool init;

BmpContainer hourContainer;
BmpContainer secContainer;

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

  bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &hourContainer);
  hourContainer.layer.layer.frame.origin.x =  1*72;
  hourContainer.layer.layer.frame.origin.y = 1*84;
  layer_add_child(&window.layer, &hourContainer.layer.layer);

}


void unload_ship_image() {
  /*

     Removes the digit from the display and unloads the image resource
     to free up RAM.

     Can handle being called on an already empty slot.

   */

    layer_remove_from_parent(&hourContainer.layer.layer);
    bmp_deinit_container(&hourContainer);

}

void update_ship_hour_hand(unsigned short value)
{
//	value = value % 100; // Maximum of two digits per row.
	if (init)
	{
    unload_ship_image();
	}
	  
    load_ship_image(value);
    
//    value = value / 10;
}

void display_time(PblTm *tick_time) {

  // TODO: Use `units_changed` and more intelligence to reduce
  //       redundant digit unload/load?

  update_ship_hour_hand(get_display_hour(tick_time->tm_hour));
  //display_value(tick_time->tm_min, 1, true);
}

void handle_tick(AppContextRef params, PebbleTickEvent *t)
{
	 // Avoids a blank screen on watch start.
 	PblTm tick_time;

  	get_time(&tick_time);
//  	display_time(&tick_time);
}

void handle_init() {
	init = false;
	window_init(&window,"Space Invader");
	window_stack_push(&window, true /* Animated */);
    
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