#include <pebble.h>

#define WORKER_TIMESTAMP 0
#define APP_NOTICE       1

static Window *s_main_window;
static TextLayer *s_title0_layer;
static TextLayer *s_data0_layer;
static TextLayer *s_title1_layer;
static TextLayer *s_data1_layer;
static TextLayer *s_title2_layer;
static TextLayer *s_data2_layer;

static void worker_message_handler(uint16_t type, AppWorkerMessage *data)
{
	unsigned int mins_tmp;
	if (type != WORKER_TIMESTAMP) return;

	// Read data from worker's packet
	unsigned int mins_since_plugged = data->data0;
	unsigned int mins_since_unplugged = data->data1;
	BatteryChargeState state = battery_state_service_peek();

	static char s_buffer0[32];
	snprintf(s_buffer0, sizeof(s_buffer0), "%u percent", state.charge_percent);
	text_layer_set_text(s_data0_layer, s_buffer0);

	static char s_buffer1[32];
	mins_tmp = (state.is_plugged) ? (mins_since_unplugged - mins_since_plugged) : mins_since_unplugged;
	snprintf(s_buffer1, sizeof(s_buffer1), "%uD%02uH%02uM", mins_tmp / (24 * 60),
			(mins_tmp % (24 * 60)) / 60, (mins_tmp % (24 * 60)) % 60);
	text_layer_set_text(s_data1_layer, s_buffer1);

	static char s_buffer2[32];
	mins_tmp = (state.is_plugged) ? mins_since_plugged : (mins_since_plugged - mins_since_unplugged);
	snprintf(s_buffer2, sizeof(s_buffer2), "%uD%02uH%02uM", mins_tmp / (24 * 60),
			(mins_tmp % (24 * 60)) / 60, (mins_tmp % (24 * 60)) % 60);
	text_layer_set_text(s_data2_layer, s_buffer2);
}

static void wakeup_worker(void)
{
	// Construct a data packet
	AppWorkerMessage msg_data = {
		.data0 = 0
	};

	// Send the data to the background worker
	app_worker_send_message(APP_NOTICE, &msg_data);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
	wakeup_worker();
}

static void click_config_provider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Create UI
	s_title0_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
    text_layer_set_text_color(s_title0_layer, GColorWhite);
    text_layer_set_background_color(s_title0_layer, GColorBlack);
	text_layer_set_text(s_title0_layer, "Battery Info");
	text_layer_set_text_alignment(s_title0_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_title0_layer));

	s_data0_layer = text_layer_create(GRect(0, 20, bounds.size.w, 35));
    text_layer_set_text_color(s_data0_layer, GColorBlack);
    text_layer_set_background_color(s_data0_layer, GColorWhite);
    text_layer_set_font(s_data0_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text(s_data0_layer, "No data yet.");
	text_layer_set_text_alignment(s_data0_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_data0_layer));

	s_title1_layer = text_layer_create(GRect(0, 55, bounds.size.w, 20));
    text_layer_set_text_color(s_title1_layer, GColorWhite);
    text_layer_set_background_color(s_title1_layer, GColorBlack);
	text_layer_set_text(s_title1_layer, "Discharging Time");
	text_layer_set_text_alignment(s_title1_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_title1_layer));

	s_data1_layer = text_layer_create(GRect(0, 75, bounds.size.w, 35));
    text_layer_set_text_color(s_data1_layer, GColorBlack);
    text_layer_set_background_color(s_data1_layer, GColorWhite);
    text_layer_set_font(s_data1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text(s_data1_layer, "No data yet.");
	text_layer_set_text_alignment(s_data1_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_data1_layer));

	s_title2_layer = text_layer_create(GRect(0, 110, bounds.size.w, 20));
    text_layer_set_text_color(s_title2_layer, GColorWhite);
    text_layer_set_background_color(s_title2_layer, GColorBlack);
	text_layer_set_text(s_title2_layer, "Charging Time");
	text_layer_set_text_alignment(s_title2_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_title2_layer));

	s_data2_layer = text_layer_create(GRect(0, 130, bounds.size.w, 35));
    text_layer_set_text_color(s_data2_layer, GColorBlack);
    text_layer_set_background_color(s_data2_layer, GColorWhite);
    text_layer_set_font(s_data2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text(s_data2_layer, "No data yet.");
	text_layer_set_text_alignment(s_data2_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_data2_layer));
}

static void main_window_unload(Window *window)
{
	// Destroy UI
	text_layer_destroy(s_title0_layer);
	text_layer_destroy(s_data0_layer);
	text_layer_destroy(s_title1_layer);
	text_layer_destroy(s_data1_layer);
	text_layer_destroy(s_title2_layer);
	text_layer_destroy(s_data2_layer);
}

static void init(void)
{
	// Setup main Window
	s_main_window = window_create();
	window_set_click_config_provider(s_main_window, click_config_provider);
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});
	window_stack_push(s_main_window, true);

	// Subscribe to Worker messages
	app_worker_message_subscribe(worker_message_handler);

	if (!app_worker_is_running()) {
		app_worker_launch();
	}
	else {
		wakeup_worker();
	}
}

static void deinit(void)
{
	// No more worker updates
	app_worker_message_unsubscribe();

	// Destroy main Window
	window_destroy(s_main_window);
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
