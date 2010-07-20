static window_t *window_bottom = NULL;

static void window_bottom_update (int type)
{
	struct _font font;
	int offset;
	
	gc_set_fc (window_bottom->gc, COLOR_TRANSPARENT);
	gt_fillrect (&window_bottom->gui, window_bottom->gc, 0, 0, 1280, 110);
	
	gc_set_bc (window_bottom->gc, COLOR_TRANSPARENT);
	gc_set_fc (window_bottom->gc, COLOR_WINDOW_BOTTOM_TEXT);

	font.size = 14;
	
	if (type == 0)
	{
		offset = 35;
		if (button_i)
		{
			char msg[256];
			sprintf (msg, intl (MORE_INFO));
			gui_draw_image (&window_bottom->gui, button_i, window_bottom->gc, offset, 5, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_i, window_bottom->gc, offset, 5, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 8);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_link)
		{
			char msg[256];
			// TODO intl ()
			sprintf (msg, "Link TV Series");
			gui_draw_image (&window_bottom->gui, button_link, window_bottom->gc, offset, 5, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_link, window_bottom->gc, offset, 5, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 8);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}

		
		offset = 35;
		if (button_menu)
		{
			char msg[256];
			sprintf (msg, intl (MENU));
			gui_draw_image (&window_bottom->gui, button_menu, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_menu, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_ok)
		{
			char msg[256];
			sprintf (msg, intl (ZAP));
			gui_draw_image (&window_bottom->gui, button_ok, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_ok, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_rec)
		{
			char msg[256];
			sprintf (msg, intl (RECORD));
			gui_draw_image (&window_bottom->gui, button_rec, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_rec, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_pg)
		{
			char msg[256];
			sprintf (msg, intl (CHANGE_GROUP));
			gui_draw_image (&window_bottom->gui, button_pg, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_pg, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_play)
		{
			char msg[256];
			sprintf (msg, intl (GO_TO_NOW));
			gui_draw_image (&window_bottom->gui, button_play, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_play, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_rew && button_fwd)
		{
			char msg[256];
			sprintf (msg, intl (LG_24));
			gui_draw_image (&window_bottom->gui, button_rew, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_rew, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 32;
			gui_draw_image (&window_bottom->gui, button_fwd, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_fwd, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_vol)
		{
			char msg[256];
			sprintf (msg, intl (PAGE_UD));
			gui_draw_image (&window_bottom->gui, button_vol, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_vol, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
	}
	else
	{
		offset = 35;
		if (button_stop)
		{
			char msg[256];
			// TODO: intl()
			sprintf (msg, "Delete event");
			gui_draw_image (&window_bottom->gui, button_stop, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_stop, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
		if (button_link)
		{
			char msg[256];
			// TODO intl ()
			sprintf (msg, "Relink TV Series");
			gui_draw_image (&window_bottom->gui, button_link, window_bottom->gc, offset, 45, 26, 23, 0);
			//img_draw (&window_bottom->gui, button_link, window_bottom->gc, offset, 25, 26, 23, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->gui, window_bottom->gc, offset, 48);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 30;
		}
	}
	
	wm_redraw (window_bottom);
}

static void window_bottom_show (int type)
{
	if (window_bottom) window_bottom_update (type);
	else
	{
		rect_t rect = { 0, 610, 1280, 110 };
		window_bottom = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
		window_bottom_update (type);
		wm_show (window_bottom);
	}
}

static void window_bottom_hide ()
{
	if (window_bottom) wm_destroy (window_bottom);
	window_bottom = NULL;
}
