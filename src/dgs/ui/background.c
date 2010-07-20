static window_t *window_background = NULL;
static gui_image_t background = NULL;

static void show_background (char *name, int view)
{
	char file[256];
	sprintf (file, "%s/skins/%s/backgrounds/%s", config_get_home_directory (), config_get_skin (), name);
	if (background) gui_close_image (background);
	background = gui_open_image_by_path(file, gui_image_type_png);
	bool toshow = false;
	
	if (!window_background)
	{
		toshow = true;
		rect_t rect = { 0, 0, 1280, 720 };
		window_background = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
	}
	
	if (background)
	{
		gc_set_fc (window_background->gc, COLOR_TRANSPARENT);
		gt_fillrect (&window_background->gui, window_background->gc, 0, 0, 1280, 720);
		gui_draw_image (&window_background->gui, background, window_background->gc, 0, 0, 1280, 720, 0);
		//img_draw (&window_background->fb, background, window_background->gc, 0, 0, 720, 576, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
	}
	//else
	//{
	//	gc_set_fc (window_background->gc, COLOR_WINDOW_BACKGROUND);
	//	gt_fillrect (&window_background->gui, window_background->gc, 0, 0, 720, 576);
	//	gc_set_fc (window_background->gc, COLOR_TRANSPARENT);
	//	if (view == 0) gt_fillrect (&window_background->gui, window_background->gc, 462, 52, 228, 206);
	//	else if (view == 1) gt_fillrect (&window_background->gui, window_background->gc, 0, 55, 720, 395);
	//}
	wm_redraw (window_background);
	if (toshow) wm_show (window_background);
}

static void hide_background ()
{
	if (background) gui_close_image (background);
	if (window_background) wm_destroy (window_background);
	window_background = NULL;
	background = NULL;
}
