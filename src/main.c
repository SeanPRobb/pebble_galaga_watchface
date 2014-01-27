#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID {0x59, 0x1C, 0x11, 0x75, 0xC4, 0xC8, 0x35, 0xF7, 0x9F, 0x4B, 0xEA, 0x13, 0x25, 0x9D, 0x0A, 0x10}
PBL_APP_INFO(MY_UUID, "Galaga Clock", "Work Hoodie", 0x1, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

	
Window window;

bool init;

RotBmpContainer rotHourContainer;
RotBmpContainer rotMinContainer;
RotBmpContainer rotSecondContainter;
BmpContainer backgroundContainer;

GPoint minOffset;
GPoint secOffset;

void set_init_coords(RotBmpContainer *image, GPoint initCoords) {
    GRect r = layer_get_frame(&image->layer.layer);
    r.origin.x = initCoords.x + 144 / 2;
    r.origin.y = -initCoords.y + 168 / 2;
    layer_set_frame(&image->layer.layer, r);
}

void set_angled_position_square(RotBmpContainer *image, double angle, GSize size, GPoint offset) {
	
    GPoint r;
	double xAngle = angle;
	double yAngle = angle;
	if (((angle/TRIG_MAX_ANGLE)*360) < 45) //0-15 sec
	{
		xAngle=angle;
		yAngle=TRIG_MAX_ANGLE * 0;
			r= GPoint(((size.w-26) * sin_lookup(xAngle) / TRIG_MAX_RATIO) - offset.x,(size.h * cos_lookup(yAngle) / TRIG_MAX_RATIO) + offset.y);	
	}
	else
	{
		if (((angle/TRIG_MAX_ANGLE)*360)< 135) // 15-23 sec
		{
			xAngle=TRIG_MAX_ANGLE * 90 / 360;
			yAngle=angle;
			r= GPoint(((size.w-26) * sin_lookup(xAngle) / TRIG_MAX_RATIO) - offset.x,(size.h * cos_lookup(yAngle) / TRIG_MAX_RATIO) + offset.y);	
		}
		else
		{
			if (((angle/TRIG_MAX_ANGLE)*360)<225) //23-37 sec
			{
				xAngle=angle;
				yAngle=TRIG_MAX_ANGLE * 180 / 360;
				r= GPoint(((size.w-26) * sin_lookup(xAngle) / TRIG_MAX_RATIO) - offset.x,(size.h * cos_lookup(yAngle) / TRIG_MAX_RATIO) + offset.y);	
			}	
			else
			{
				if (((angle/TRIG_MAX_ANGLE)*360)<315) // 38 - 52 sec
				{
					xAngle=TRIG_MAX_ANGLE * 270 / 360;
					yAngle=angle;
					r= GPoint(((size.w-26) * sin_lookup(xAngle) / TRIG_MAX_RATIO) - offset.x,(size.h * cos_lookup(yAngle) / TRIG_MAX_RATIO) + offset.y);	
				}
				else //52-60 sec
				{
					xAngle=angle;
					yAngle=TRIG_MAX_ANGLE * 0;
					r= GPoint(((size.w-26) * sin_lookup(xAngle) / TRIG_MAX_RATIO) - offset.x,(size.h * cos_lookup(yAngle) / TRIG_MAX_RATIO) + offset.y);	
				}
			}
		}
	}
				
	set_init_coords(image, r);
}

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
}

void update_hand_positions() {
    PblTm t;
    get_time(&t);
    
    int sec = t.tm_sec;
    int min = t.tm_min;
    int hour = t.tm_hour;
    
    int32_t aSec = TRIG_MAX_ANGLE * (sec * 6) /360;
	GSize secSquare=GSize(84,72);
	set_angled_position_square(&rotSecondContainter, aSec, secSquare, secOffset);
    layer_mark_dirty(&rotSecondContainter.layer.layer);
    
   if (!init || sec % 10 == 0) {
        int32_t aMin = TRIG_MAX_ANGLE * ((min * 6) + sec / 10) /360;
		GSize minSquare=GSize(55,45);
        set_angled_position_square(&rotMinContainer, aMin, minSquare, minOffset);
        layer_mark_dirty(&rotMinContainer.layer.layer);
    }
    
    if (!init || min % 5 == 0) {
	//For hour angle
		set_hand_angle(&rotHourContainer, ((hour % 12) * 30) + (min/2));
		layer_mark_dirty(&rotHourContainer.layer.layer);
    }
}

void handle_tick(AppContextRef params, PebbleTickEvent *t)
{
  	update_hand_positions();
}

void handle_init() {
	init = false;
	window_init(&window,"Galaga Clock");
	window_stack_push(&window, true /* Animated */);
  	//window_set_background_color(&window, GColorBlack);
    
    resource_init_current_app(&APP_RESOURCES);
    
	bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &backgroundContainer);
	layer_add_child(&window.layer, &backgroundContainer.layer.layer);
	
    // Set up a layer for the minute hand
    rotbmp_init_container(RESOURCE_ID_IMAGE_MIN_SPRITE, &rotMinContainer);
    GSize minSize = layer_get_frame(&rotMinContainer.layer.layer).size;
    minOffset = GPoint(minSize.w / 2, minSize.h / 2);
    layer_add_child(&window.layer, &rotMinContainer.layer.layer);
	
    // Set up a layer for the second hand
    rotbmp_init_container(RESOURCE_ID_IMAGE_SECOND_SPRITE, &rotSecondContainter);
    GSize secSize = layer_get_frame(&rotSecondContainter.layer.layer).size;
    secOffset = GPoint(secSize.w / 2, secSize.h / 2);
    layer_add_child(&window.layer, &rotSecondContainter.layer.layer);
	
	// Set up a layer for hour hand
  	rotbmp_init_container(RESOURCE_ID_IMAGE_SHIP_SPRITE, &rotHourContainer);
  	layer_add_child(&window.layer, &rotHourContainer.layer.layer);
    
	 // Avoids a blank screen on watch start. 	
  	update_hand_positions();
	
    init = true;    
}

void handle_deinit() {
	rotbmp_deinit_container(&rotHourContainer);
	rotbmp_deinit_container(&rotMinContainer);
	rotbmp_deinit_container(&rotSecondContainter);
	bmp_deinit_container(&backgroundContainer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };
	  
  app_event_loop(params, &handlers);
}