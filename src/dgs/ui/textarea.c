bool textarea (window_t *wnd, char *message, int x, int y, int width, int height, int fontsize, int rowsize, int max_rows, int row_offset)
{
	struct _font font;
	char *words[4096];
	int words_sizes[4096];
	bool words_returns[4096];
	int words_count = 0;
	int words_last = 0;
	char *tmp = message;
	int i;
	int rows = 0;
	//bool ret = false;
	
	for (i=0; i<4096; i++)
		words[i] = NULL;
	
	font.size = fontsize;
	while (words_count < 4096)
	{
		int j;
		int len = 0;
		int max = strlen (tmp);
		bool word_return = false;

		for (j=0; j<max; j++)
		{
			if ((tmp[j] == '\n') || (tmp[j] == ' '))
			{
				if (tmp[j] == '\n')
					word_return = true;
				len = j;
				break;
			}
		}
		if (j == max) len = max;
		//if (len > 0)
		//{
			words_returns[words_count] = word_return;
			words[words_count] = _malloc (len + 1);
			memcpy (words[words_count], tmp, len);
			words[words_count][len] = '\0';
			words_sizes[words_count] = font_width_str (&font, words[words_count], len);
			words_count++;
		//}
		if (strlen(tmp) > (len+1)) tmp += len + 1;
		else break;
	}
	
	while (words_last < words_count)
	{
		for (i=words_last; i<words_count; i++)
		{
			int j;
			if ((i - words_last) > 0)
			{
				int len = 0;

				for (j=words_last; j<i; j++)
					len += words_sizes[j] + 5;

				if (len > width)
				{
					int adjust = 0;
					int wnum = i - words_last - 2;
					len -= words_sizes[i-1];
					len -= 5;
					if (wnum > 0)
						adjust = (width - len) / wnum;
					adjust++;
					
					int offset = 0;
					for (j=words_last; j<(i-1); j++)
					{
						if ((rows >= row_offset) && (rows < (max_rows + row_offset)))
							font_draw_str (&font, words[j], strlen (words[j]), &wnd->gui, wnd->gc, x + offset, y + ((rows-row_offset)*rowsize));
						offset += words_sizes[j] + 5 + adjust;
						if (j == (i-3)) offset += width - offset - words_sizes[i-2];
					}
					words_last = i-1;
					break;
				}
			}
			
			if ((words_count-1) == i)
			{
				int offset = 0;
				for (j=words_last; j<(i+1); j++)
				{
					if ((rows >= row_offset) && (rows < (max_rows + row_offset)))
						font_draw_str (&font, words[j], strlen (words[j]), &wnd->gui, wnd->gc, x + offset, y + ((rows-row_offset)*rowsize));
					offset += words_sizes[j] + 5;
				}
				words_last = i+1;
				//ret = true;
				break;
			}

			// UNTESTED !!!
			if (words_returns[i])
			{
				int offset = 0;
				for (j=words_last; j<(i+1); j++)
				{
					if ((rows >= row_offset) && (rows < (max_rows + row_offset)))
						font_draw_str (&font, words[j], strlen (words[j]), &wnd->gui, wnd->gc, x + offset, y + ((rows-row_offset)*rowsize));
					offset += words_sizes[j] + 5;
				}
				words_last = i+1;
				break;
			}
		}
		rows++;
	}
	
	for (i=0; i<4096; i++)
		if (words[i] != NULL)
			_free (words[i]);
	
	return (rows <= (max_rows + row_offset));
}
