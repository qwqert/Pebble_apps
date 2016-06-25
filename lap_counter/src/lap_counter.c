
#include <pebble.h>

#define LIST_HEADER_HEIGHT         18
#define LIST_MAX_ROWS              30
#define LIST_CELL_TITLE_HEIGHT     28
#define LIST_CELL_SUBTITLE_HEIGHT  16

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_list_message_layer;

static unsigned int s_num_rows = 0;

typedef struct {
    time_t t_s;
    uint16_t t_ms;
} lap_time_t;

static lap_time_t times[LIST_MAX_ROWS];

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) 
{
    return s_num_rows;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) 
{
    static char s_title[] = "[00]+00:00:00.000";
    static char s_subtitle[] = "+00:00:00.000/00:00:00";
    struct tm *tm_time = localtime(&times[cell_index->row].t_s);

    if (cell_index->row != 0) {
        unsigned int sublast_ms = times[cell_index->row].t_ms + 1000 - times[cell_index->row - 1].t_ms;
        unsigned int sublast_s = (unsigned int)times[cell_index->row].t_s -
                                 (unsigned int)times[cell_index->row - 1].t_s - 
                                 !(sublast_ms / 1000);
        unsigned int subhead_ms = times[cell_index->row].t_ms + 1000 - times[0].t_ms;
        unsigned int subhead_s = (unsigned int)times[cell_index->row].t_s - 
                                 (unsigned int)times[0].t_s - !(subhead_ms / 1000);

        snprintf(s_title, sizeof(s_title), "%02d +%02d:%02d:%02d.%03d",
                cell_index->row, sublast_s / 3600, sublast_s % 3600 / 60,
                sublast_s % 3600 % 60, sublast_ms % 1000);

        snprintf(s_subtitle, sizeof(s_subtitle), "+%02d:%02d:%02d.%03d/%02d:%02d:%02d",
                subhead_s / 3600, subhead_s % 3600 / 60, subhead_s % 3600 % 60, subhead_ms % 1000,
                tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

        menu_cell_basic_draw(ctx, cell_layer, s_title, s_subtitle, NULL);
    }
    else {
        snprintf(s_title, sizeof(s_title), "[START] %02d:%02d:%02d",
                tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

        menu_cell_basic_draw(ctx, cell_layer, s_title, NULL, NULL);
    }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
    if (cell_index->row == 0)
        return LIST_CELL_TITLE_HEIGHT;
    else
        return LIST_CELL_TITLE_HEIGHT + LIST_CELL_SUBTITLE_HEIGHT;
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) 
{
    static char time_text[] = "00:00";

    strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
    text_layer_set_text(s_list_message_layer, time_text);
}

void select_long_click_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
    s_num_rows = 0;
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

void select_click_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
    time_ms(&times[s_num_rows].t_s, &times[s_num_rows].t_ms);

    MenuIndex last_item = {
        .section = cell_index->section,
        .row = (s_num_rows >= LIST_MAX_ROWS)? LIST_MAX_ROWS : ++s_num_rows,
    };
    menu_layer_reload_data(menu_layer);
    menu_layer_set_selected_index(menu_layer, last_item, MenuRowAlignBottom, true);
}

static void main_window_load(Window *window) 
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_menu_layer = menu_layer_create(GRect(bounds.origin.x, bounds.origin.y + LIST_HEADER_HEIGHT,
                bounds.size.w, bounds.size.h - LIST_HEADER_HEIGHT));
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_pad_bottom_enable(s_menu_layer, false);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows      = get_num_rows_callback,
        .draw_row          = draw_row_callback,
        .get_cell_height   = get_cell_height_callback,
        .select_click      = select_click_callback,
        .select_long_click = select_long_click_callback,
    });
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

    s_list_message_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y,
                bounds.size.w, LIST_HEADER_HEIGHT));
    text_layer_set_text_alignment(s_list_message_layer, GTextAlignmentCenter);
    text_layer_set_text_color(s_list_message_layer, GColorWhite);
    text_layer_set_background_color(s_list_message_layer, GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_list_message_layer));

    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now);
    tick_handler(tick_time, 0); 
}

static void main_window_unload(Window *window) 
{
    menu_layer_destroy(s_menu_layer);
    text_layer_destroy(s_list_message_layer);
}

static void init(void)
{
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load   = main_window_load,
		.unload = main_window_unload,
	});
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	window_stack_push(s_main_window, true);
}

static void deinit(void)
{
    tick_timer_service_unsubscribe();
	window_destroy(s_main_window);
    s_main_window = NULL;
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
