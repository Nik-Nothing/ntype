#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

#include <string.h>
#include <time.h>
#include <ctype.h>

#include <curses.h>

#include "dict.h"
#include "macros.h"

enum gamemode{gm_undefined, words_count, time_count, endless};

enum control{insert, backspace, eow};

typedef struct settings
{
	bool stall;
	bool stay;
}
set;

svoid curses_init()
{
	initscr();
	curs_set(0);
	cbreak();
	noecho();
	timeout(1);
	start_color();

	init_color(COLOR_CYAN, 350, 350, 350);
	init_pair(1, COLOR_BLACK, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_RED);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_BLACK);
}

svoid alloc_buffers(char ***words_pre, char **word_post, uint **mistake_pos)
{
	int cts = 0;

	*mistake_pos = malloc(sizeof(int) * 20);
	*words_pre = malloc(sizeof(char *) * 50);
	*word_post = malloc(sizeof(char) * 25);
	do
	{
		words_pre[0][cts] = malloc(sizeof(char) * 25);
	}
	while (++cts < 50);
}

svoid noreturn quit(ulong start_time, uint mistakes, uint wc, bool print, char *custom_log)
{
	ulong time_elps; 
	
	endwin();
	if (print)
	{
		if (custom_log == NULL)
		{
			time_elps = (unsigned long) time(NULL) - start_time;
			printf("%d Words in %lu Seconds! that is %.2f WPM, you made %d Mistakes\n", wc, time_elps, (double) 60 * ( (double) wc / (double) time_elps), mistakes);
		}
		else
			printf("%s", custom_log);
	}
	exit(0);
}

svoid free_bufs(char **pre, uint words, char *post, uint *mis)
{
	uint ctl = 0;
	
	do
	{
		free(pre[ctl]);
	}
	while (++ctl < words);
	free(pre);
	free(post);
	free(mis);
}

static WINDOW *create_newwin(int h, int w, int sy, int sx, bool pressed, bool hi)
{
	WINDOW *local_win;

	local_win = newwin(h, w, sy, sx);
	box(local_win, 0, 0);
	if (pressed)
		wbkgd(local_win,COLOR_PAIR(hi ? 2 : 1));
	wrefresh(local_win);
	return local_win;
}

svoid create_keyboard(bool init, int len, int hei, char in, bool hi, char last)
{
	int		x = -1;
	char	c1[27] = {'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ','};
	int		hr = 4, ofs = 0, idp = 0;
	
	while (++x != 27)
	{
		if (init || c1[x] == in || c1[x] == last)
			create_newwin(hei / 14, hei / 8, hei - (hei / 14) * hr,((hei / 8) * idp) + ((hei / 20) * ofs) + ((len / 2) - (hei / 8) * 5), (c1[x] == in), hi);
		++idp;
		if (x == 9 || x == 18)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	if (init || ' ' == in || last == ' ')
		create_newwin(hei / 14, (hei / 8) * 5, hei - (hei / 14), (len / 2) - ((hei / 14) * 4),in == ' ' ? 1 : 0, hi);
}

svoid generate_giberish(char **buf, uint len, enum dict dic)
{
	int ran;
	
	if (dic == en)
	{
		ran = (rand() % 1000);
		strcpy(buf[len], en_dict[ran]);
	} 
	elif (dic == unix)
	{
		ran = (rand() % 107);
		strcpy(buf[len], unix_dict[ran]);
	}
}

static int check_word(char **buf_pre, char *buf_post, uint wp)
{
	uint x = 0;
	
	do
	{
		if (buf_pre[wp][x] != buf_post[x])
			return 1;
	}
	while (++x < strlen(buf_pre[wp]));
	return 0;
}

svoid print_pre(char **words_pre, WINDOW *tpwin, uint words, uint max, uint wln, uint wff_off)
{
	uint wcc = 0, wcc_off = wff_off, y = 0, yw = 0;
	
	wattron(tpwin, COLOR_PAIR(3));
	do 
	{
		if ((wcc_off + strlen(words_pre[wcc])) > max)
		{
			++y;
			yw = y-wln;
			wcc_off = 0;
		}
		if (y >= wln)
			mvwaddstr(tpwin,yw + 2, wcc_off + 3,words_pre[wcc]);
		wcc_off += (strlen(words_pre[wcc]) + 1);
	}
	while (++wcc < words);
	wattroff(tpwin, COLOR_PAIR(3));
}

static char setch(char *c)
{
	*c = (char) getch();
	return *c;
}

svoid shift_buffers(char **words_pre, char *word_post)
{
	int ctl = -1;
	
	memmove(words_pre, words_pre + 1, sizeof(words_pre) * 50);
	words_pre[49] = malloc(sizeof(char *));
	while (++ctl < 25)
		*(word_post++) = '\0';
}

int main(int argc, char **argv)
{
	set *s,s_o =
	{ 
		.stall = false,
		.stay = false
	};
	
	bool	hi=false, tt = false, ww = false, dd = false;
	ulong	start_time = 0, timer = 0;
	uint	mistakes_total = 0, z, mistakes = 0, cts = 0, current_word_position = 0, *mistake_pos, wcc_off, words = 0;
	uint	current_word_indp = 0;
	uint	wff_off = 0;
	char 	c = '\0', **words_pre,*word_post, last = '\0';
	
	enum	gamemode gm = gm_undefined;
	enum	dict dic = d_undefined;
	enum	control curcon = insert;
	
	WINDOW	*tpwin;
	
	s = &s_o;
	while (0 < --argc)
	{
		++argv;
		if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0)
		{
			quit(0, 0, 0, true, "help\n");
		}
		elif (strcmp(*argv, "--version") == 0 || strcmp(*argv, "-v") == 0)
		{
			quit(0, 0, 0, true, "Typing Curses V0\n");
		}
		elif (strcmp(*argv, "--words") == 0 || strcmp(*argv, "-w") == 0)
		{
			gm = words_count;
			ww = true;
		}
		elif (strcmp(*argv, "--time")==0 || strcmp(*argv, "-t") == 0)
		{
			gm = time_count;
			tt = true;
		}
		elif (strcmp(*argv, "--endless") == 0 || strcmp(*argv, "-e") == 0)
		{
			gm = endless;
		}
		elif (strcmp(*argv, "--dictionary") == 0 || strcmp(*argv, "-d") == 0)
		{
			dd = true;
		}
		elif (strcmp(*argv, "--stay-inplace") == 0 || strcmp(*argv, "-S") == 0)
		{
			s->stay = true;
		}
		elif (strcmp(*argv, "--stall") == 0 || strcmp(*argv, "-s") == 0)
		{
			s->stall = true;
		}
		elif (tt)
		{
			int t = atoi(*argv);
			if (t < 0)
				quit(0, 0, 0, true, "invalid time\n");
			else
				timer = (unsigned int) t;
			tt = false;
		}
		elif (ww)
		{
			int t = atoi(*argv);
			if (t < 0)
				quit(0, 0, 0, true, "invalid time\n");
			else
				words = (uint) t;
			ww = false;
		}
		elif (dd)
		{
			if (strcmp(*argv, "en") == 0)
				dic = en;
			elif (strcmp(*argv, "unix") == 0)
				dic = unix;
			else
				quit(0, 0, 0, true, "dictionary not found\n");
		}
		else
			quit(0, 0, 0, true, "invalid argument\n");
	}
	if (tt || timer == 0)
		timer = 15;
	if (ww || words == 0)
		words = 40;
	if (gm == gm_undefined)
		gm = words_count;
	if (dic == d_undefined)
		dic = en;

	srand((uint) time(NULL));
	curses_init();
	
	alloc_buffers(&words_pre, &word_post, &mistake_pos);
	
	cts = 0;
	do
	{
		generate_giberish(words_pre, cts, dic);
	}
	while (++cts < words);
	
	while (1)
	{
		refresh();
		tpwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0, 0, 0);
		wcc_off = (wff_off - (((wff_off + (uint) strlen(words_pre[0])) / (uint) (COLS - 6)) * (uint) (COLS - 6)));
		print_pre(words_pre, tpwin, words, (uint) COLS - 6, 0, wcc_off);
		mvwaddstr(tpwin, 2, wcc_off + 3, word_post);
		if (mistakes > 0)
		{
			z = 0;
			do
			{
				wattron(tpwin, COLOR_PAIR(2));
				mvwaddch(tpwin, 2, 3 + wcc_off + mistake_pos[z], words_pre[0][mistake_pos[z]]);
				wattroff(tpwin, COLOR_PAIR(2));
			}
			while (++z < mistakes);
		}
		wcc_off += (strlen(words_pre[0]) + 1);
		wrefresh(tpwin);
		refresh();
		create_keyboard((start_time == 0), COLS, LINES, c, hi, last);
		last = c;
		hi = false;
		if (gm == words_count && current_word_indp == words)
		{
			free_bufs(words_pre,words,word_post,mistake_pos);
			quit(start_time, mistakes_total, current_word_indp, true, NULL);
		}
		while (setch(&c) < 1)
			if (gm == time_count && start_time != 0 && (ulong) time(NULL) > (start_time + timer))
			{
				free_bufs(words_pre, words, word_post, mistake_pos);
				quit(start_time, mistakes_total, current_word_indp, true, NULL);
			}
		if (start_time == 0)
			start_time = (ulong) time(NULL);
		if (curcon != eow)
			curcon = insert;
		switch (c)
		{
			case 10:	/* Linebreak */
				c = ' ';
				break;
			case 11:	/* Tabulator */
				c = ' ';
				break;
			case 27:	/* Escape */
				free_bufs(words_pre, words, word_post, mistake_pos);
				quit(start_time, mistakes_total, current_word_indp, true, NULL);
			case 127: /* Backspace */
				if (curcon != eow)
				{
					if (current_word_position > 0)
						--current_word_position;
					word_post[current_word_position] = '\0';
					c = '\0';
					curcon = backspace;
				}
				break;
			default:
			if (curcon != eow && current_word_position < strlen(words_pre[0]))
				word_post[current_word_position] = c;
			else
				hi = true;
		}
		if (curcon == backspace)
		{
			if (mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
				--mistakes;
		}
		elif (curcon == insert)
		{
			if (current_word_position == strlen(words_pre[0]) - 1)
			{
					if (check_word(words_pre, word_post, 0) == 0 || !s->stall)
					{
						current_word_position = 0;
						++current_word_indp;
						curcon = eow;
						mistakes = 0;
						if (!s->stay)
							wff_off += strlen(word_post)+1;
						shift_buffers(words_pre,word_post);
						if (gm == time_count || gm == endless)
							generate_giberish(words_pre, words-1, en);
					}
					if (check_word(words_pre, word_post, 0) == 1)
					{
						hi = true;
					}
			}
			if (curcon != eow && current_word_position < strlen(words_pre[0]))
			{
				if (c != words_pre[0][current_word_position])
				{
					hi = true;
					mistake_pos[mistakes] = current_word_position;
					++mistakes;
					++mistakes_total;
				}
				else
				{
					if (mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
						--mistakes;
					hi = false;
				}
				++current_word_position;
			}
		}
		elif (curcon == eow && c == ' ')
		{
			hi = false;
			curcon = insert;
		}
	}
}
