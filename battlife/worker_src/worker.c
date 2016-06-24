#include <pebble_worker.h>

#define WORKER_TIMESTAMP 0
#define APP_NOTICE       1

static bool is_plugged_last;
static time_t time_plugged;
static time_t time_unplugged;

static void send_message(void)
{
	time_t time_now = time(NULL);
	unsigned int mins_since_plugged = (time_now - time_plugged) / 60;
	unsigned int mins_since_unplugged = (time_now - time_unplugged) / 60;

	// Construct a data packet
	AppWorkerMessage msg_data = {
		.data0 = mins_since_plugged,
		.data1 = mins_since_unplugged,
	};

	// Send the data to the foreground app
	app_worker_send_message(WORKER_TIMESTAMP, &msg_data);
}

static void app_message_handler(uint16_t type, AppWorkerMessage *data) 
{
	if (type == APP_NOTICE) { 
		send_message();
	}
}

static void battery_state_handler(BatteryChargeState charge) 
{
	// start charging
	if (!is_plugged_last && charge.is_plugged) {
		time_plugged = time(NULL);
	}
	// stop charging
	else if (is_plugged_last && !charge.is_plugged) {
		time_unplugged = time(NULL);
	}

	send_message();
	is_plugged_last = charge.is_plugged;
}

static void worker_init() 
{
	BatteryChargeState initial = battery_state_service_peek();
	is_plugged_last = initial.is_plugged;

	time_plugged = time(NULL);
	time_unplugged = time_plugged;

    battery_state_service_subscribe(&battery_state_handler);

	// Subscribe to App messages
	app_worker_message_subscribe(app_message_handler);
}

static void worker_deinit() 
{
    battery_state_service_unsubscribe();
	app_worker_message_unsubscribe();
}

int main(void) 
{
	worker_init();
	worker_event_loop();
	worker_deinit();
}

