#include <curses.h>

#include "macros.h"
#include "tc.h"

WINDOW *create_newwin(int h, int w, int sy, int sx)
{
	WINDOW *local_win;

	local_win = newwin(h, w, sy, sx);
	box(local_win, 0, 0);
	wrefresh(local_win);
	return local_win;
}

void update_win(WINDOW* local_win, bool pressed, bool hi)
{
	box(local_win, 0, 0);
	if (pressed)
		wbkgd(local_win, COLOR_PAIR(hi ? 2 : 1));
	else
		wbkgd(local_win, COLOR_PAIR(5));
	wrefresh(local_win);
}

void init_keyboard(buffs *bfs, int len, int hei)
{
	int		hr = 4, ofs = 0, idp = 0;

	uint ctl = 0;
	do
	{
		bfs->kbwin[ctl] = create_newwin(hei / 14, hei / 8, hei - (hei / 14) * hr,((hei / 8) * idp) + ((hei / 20) * ofs) + ((len / 2) - (hei / 8) * 5));
		++idp;
		if (ctl == 9 || ctl == 18)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	while (++ctl != 27);
	
	bfs->kbwin[ctl] = create_newwin(hei / 14, (hei / 8) * 5, hei - (hei / 14), (len / 2) - ((hei / 14) * 4));
}

void update_keyboard(buffs *bfs, int len, int hei, bool hi, char in, char last)
{
	const char	c1[27] = {'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ','};
	int		hr = 4, ofs = 0, idp = 0;

	uint ctl = 0;
	do
	{
		if (c1[ctl] == in || c1[ctl] == last)
				update_win(bfs->kbwin[ctl], (c1[ctl] == in), hi);
		++idp;
		if (ctl == 9 || ctl == 18)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	while (++ctl != 27);
	
	if (' ' == in || last == ' ')
		update_win(bfs->kbwin[ctl], in == ' ' ? 1 : 0, hi);
}
