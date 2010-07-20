static gui_image_t red_full = NULL;
static gui_image_t green_small = NULL;
static gui_image_t yellow_small = NULL;
static gui_image_t blue_small = NULL;
static gui_image_t button_rec = NULL;
static gui_image_t button_ok = NULL;
static gui_image_t button_pg = NULL;
static gui_image_t button_menu = NULL;
static gui_image_t button_exit = NULL;
static gui_image_t button_v = NULL;
static gui_image_t button_play = NULL;
static gui_image_t button_fwd = NULL;
static gui_image_t button_rew = NULL;
static gui_image_t button_vol = NULL;
static gui_image_t button_i = NULL;
static gui_image_t button_link = NULL;
static gui_image_t button_stop = NULL;
static gui_image_t button_timer = NULL;
static gui_image_t light_icon = NULL;
static gui_image_t grid_icon = NULL;
static gui_image_t scheduler_icon = NULL;
static gui_image_t list_icon = NULL;

static void images_load ()
{
	char file[256];
	
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/red-full.png");
	red_full = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/green-small.png");
	green_small = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/yellow-small.png");
	yellow_small = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/blue-small.png");
	blue_small = gui_open_image_by_path(file, gui_image_type_png);

	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/rec.png");
	button_rec = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/ok.png");
	button_ok = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/page.png");
	button_pg = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/menu.png");
	button_menu = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/exit.png");
	button_exit = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/v.png");
	button_v = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/play.png");
	button_play = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/fwd.png");
	button_fwd = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/rew.png");
	button_rew = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/vol.png");
	button_vol = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/i.png");
	button_i = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/link.png");
	button_link = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/stop.png");
	button_stop = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/timer.png");
	button_timer = gui_open_image_by_path(file, gui_image_type_png);

	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/light.png");
	light_icon = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/grid.png");
	grid_icon = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/scheduler.png");
	scheduler_icon = gui_open_image_by_path(file, gui_image_type_png);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/list.png");
	list_icon = gui_open_image_by_path(file, gui_image_type_png);
}

static void images_clean ()
{
	if (red_full) gui_close_image (red_full);
	if (green_small) gui_close_image (green_small);
	if (yellow_small) gui_close_image (yellow_small);
	if (blue_small) gui_close_image (blue_small);

	if (button_rec) gui_close_image (button_rec);
	if (button_ok) gui_close_image (button_ok);
	if (button_pg) gui_close_image (button_pg);
	if (button_menu) gui_close_image (button_menu);
	if (button_exit) gui_close_image (button_exit);
	if (button_v) gui_close_image (button_v);
	if (button_play) gui_close_image (button_play);
	if (button_fwd) gui_close_image (button_fwd);
	if (button_rew) gui_close_image (button_rew);
	if (button_vol) gui_close_image (button_vol);
	if (button_i) gui_close_image (button_i);
	if (button_link) gui_close_image (button_link);
	if (button_stop) gui_close_image (button_stop);
	if (button_timer) gui_close_image (button_timer);

	if (light_icon) gui_close_image (light_icon);
	if (grid_icon) gui_close_image (grid_icon);
	if (scheduler_icon) gui_close_image (scheduler_icon);
	if (list_icon) gui_close_image (list_icon);
}
