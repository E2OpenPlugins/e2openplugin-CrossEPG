static img_t red_full = NULL;
static img_t green_small = NULL;
static img_t yellow_small = NULL;
static img_t blue_small = NULL;
static img_t button_rec = NULL;
static img_t button_ok = NULL;
static img_t button_pg = NULL;
static img_t button_menu = NULL;
static img_t button_exit = NULL;
static img_t button_v = NULL;
static img_t button_play = NULL;
static img_t button_fwd = NULL;
static img_t button_rew = NULL;
static img_t button_vol = NULL;
static img_t button_i = NULL;
static img_t button_link = NULL;
static img_t button_stop = NULL;
static img_t bullet_blue = NULL;
static img_t bullet_red = NULL;
static img_t bullet_yellow = NULL;
static img_t bullet_green = NULL;

static void images_load ()
{
	char file[256];
	
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/red-full.bmp");
	red_full = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/green-small.bmp");
	green_small = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/yellow-small.bmp");
	yellow_small = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "tabs/blue-small.bmp");
	blue_small = img_open_absolute_path (file);

	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/rec.bmp");
	button_rec = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/ok.bmp");
	button_ok = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/page.bmp");
	button_pg = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/menu.bmp");
	button_menu = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/exit.bmp");
	button_exit = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/v.bmp");
	button_v = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/play.bmp");
	button_play = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/fwd.bmp");
	button_fwd = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/rew.bmp");
	button_rew = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/vol.bmp");
	button_vol = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/i.bmp");
	button_i = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/link.bmp");
	button_link = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "buttons/stop.bmp");
	button_stop = img_open_absolute_path (file);

	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/bullet-blue.bmp");
	bullet_blue = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/bullet-red.bmp");
	bullet_red = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/bullet-yellow.bmp");
	bullet_yellow = img_open_absolute_path (file);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "others/bullet-green.bmp");
	bullet_green = img_open_absolute_path (file);
}

static void images_clean ()
{
	if (red_full) img_close (red_full);
	if (green_small) img_close (green_small);
	if (yellow_small) img_close (yellow_small);
	if (blue_small) img_close (blue_small);

	if (button_rec) img_close (button_rec);
	if (button_ok) img_close (button_ok);
	if (button_pg) img_close (button_pg);
	if (button_menu) img_close (button_menu);
	if (button_exit) img_close (button_exit);
	if (button_v) img_close (button_v);
	if (button_play) img_close (button_play);
	if (button_fwd) img_close (button_fwd);
	if (button_rew) img_close (button_rew);
	if (button_vol) img_close (button_vol);
	if (button_i) img_close (button_i);
	if (button_link) img_close (button_link);
	if (button_stop) img_close (button_stop);

	if (bullet_blue) img_close (bullet_blue);
	if (bullet_red) img_close (bullet_red);
	if (bullet_yellow) img_close (bullet_yellow);
	if (bullet_green) img_close (bullet_green);
}
