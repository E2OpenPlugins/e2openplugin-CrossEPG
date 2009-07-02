static window_t *window_bottom = NULL;

static void window_bottom_update (int type)
{
	struct _font font;
	int offset;
	
	gc_set_fc (window_bottom->gc, COLOR_TRANSPARENT);
	gt_fillrect (&window_bottom->fb, window_bottom->gc, 0, 0, 720, 66);
	
	gc_set_bc (window_bottom->gc, COLOR_TRANSPARENT);
	gc_set_fc (window_bottom->gc, COLOR_WINDOW_BOTTOM_TEXT);

	font.size = 12;
	
	if (type == 0)
	{
		offset = 20;
		if (button_i)
		{
			char msg[256];
			sprintf (msg, intl (MORE_INFO));
			img_draw (&window_bottom->fb, button_i, window_bottom->gc, offset, 5, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 4);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_link)
		{
			char msg[256];
			// TODO intl ()
			sprintf (msg, "Link TV Series");
			img_draw (&window_bottom->fb, button_link, window_bottom->gc, offset, 5, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 4);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}

		offset = 20;
		if (button_menu)
		{
			char msg[256];
			sprintf (msg, intl (MENU));
			img_draw (&window_bottom->fb, button_menu, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_ok)
		{
			char msg[256];
			sprintf (msg, intl (ZAP));
			img_draw (&window_bottom->fb, button_ok, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_rec)
		{
			char msg[256];
			sprintf (msg, intl (RECORD));
			img_draw (&window_bottom->fb, button_rec, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_pg)
		{
			char msg[256];
			sprintf (msg, intl (CHANGE_GROUP));
			img_draw (&window_bottom->fb, button_pg, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_play)
		{
			char msg[256];
			sprintf (msg, intl (GO_TO_NOW));
			img_draw (&window_bottom->fb, button_play, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_rew && button_fwd)
		{
			char msg[256];
			sprintf (msg, intl (LG_24));
			img_draw (&window_bottom->fb, button_rew, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 32;
			img_draw (&window_bottom->fb, button_fwd, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_vol)
		{
			char msg[256];
			sprintf (msg, intl (PAGE_UD));
			img_draw (&window_bottom->fb, button_vol, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 5;
		}
	}
	else
	{
		offset = 20;
		if (button_stop)
		{
			char msg[256];
			// TODO: intl()
			sprintf (msg, "Delete event");
			img_draw (&window_bottom->fb, button_stop, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
		if (button_link)
		{
			char msg[256];
			// TODO intl ()
			sprintf (msg, "Relink TV Series");
			img_draw (&window_bottom->fb, button_link, window_bottom->gc, offset, 25, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			offset += 35;
			font_draw_str (&font, msg, strlen (msg), &window_bottom->fb, window_bottom->gc, offset, 24);
			offset += font_width_str (&font, msg, strlen (msg));
			offset += 10;
		}
	}
	
	wm_redraw (window_bottom);
}

static void window_bottom_show (int type)
{
	if (window_bottom) window_bottom_update (type);
	else
	{
		rect_t rect = { 0, 510, 720, 66 };
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
