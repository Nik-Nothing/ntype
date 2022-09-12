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

typedef enum gamemd
{
	gm_undefined,
	words_count,
	time_count,
	endless
}
gamemode;

typedef enum contl
{
	unstarted,
	insert,
	backspace,
	eow
}
control;

uint	ctl;

typedef struct settings
{
	gamemode	gm;
	enum		dict dic;
	bool		stall;
	bool		stay;
}
set;

typedef struct buffers
{
	char	**pre;
	char	*post;
	uint	*mistake_pos;
}
buffs;

typedef struct runtime_info
{
	uint	words;
	uint	current_word;
	uint	mistakes_total;
	ulong	start_time;
	ulong	timer;
	WINDOW	*tpwin;
	control	curcon;
}
rt_info;

static void curses_init()
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

static void alloc_buffers(buffs *bfs)
{
	ctl = 0;

	bfs->mistake_pos = malloc(sizeof(int) * 20);
	bfs->pre = malloc(sizeof(char *) * 50);
	bfs->post = malloc(sizeof(char) * 25);
	do
		bfs->pre[ctl] = malloc(sizeof(char) * 25);
	while (++ctl < 50);
}

static void noreturn quit(rt_info *rti, bool print, char *custom_log)
{
	ulong time_elps; 
	
	endwin();
	if (print)
	{
		if (custom_log == NULL)
		{
			time_elps = (unsigned long) time(NULL) - rti->start_time;
			printf("%d Words in %lu Seconds! that is %.2f WPM, you made %d Mistakes\n", rti->current_word, time_elps, (double) 60 * ( (double) rti->current_word / (double) time_elps), rti->mistakes_total);
		}
		else
			printf("%s", custom_log);
	}
	exit(0);
}

static void free_bufs(buffs *bfs)
{
	ctl = 0;
	
	do
		free(bfs->pre[ctl]);
	while (++ctl < 50);
	free(bfs->pre);
	free(bfs->post);
	free(bfs->mistake_pos);
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

static void create_keyboard(bool init, int len, int hei, char in, bool hi, char last)
{
	char	c1[27] = {'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ','};
	int		hr = 4, ofs = 0, idp = 0;
	
	ctl = 0;
	do
	{
		if (init || c1[ctl] == in || c1[ctl] == last)
			create_newwin(hei / 14, hei / 8, hei - (hei / 14) * hr,((hei / 8) * idp) + ((hei / 20) * ofs) + ((len / 2) - (hei / 8) * 5), (c1[ctl] == in), hi);
		++idp;
		if (ctl == 9 || ctl == 18)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	while (++ctl != 27);
	
	if (init || ' ' == in || last == ' ')
		create_newwin(hei / 14, (hei / 8) * 5, hei - (hei / 14), (len / 2) - ((hei / 14) * 4),in == ' ' ? 1 : 0, hi);
}

static void generate_giberish(rt_info *rti, buffs *bfs, set *s, uint len)
{
	int ran;
	
	if (s->dic == en)
	{
		ran = (rand() % 1000);
		strcpy(bfs->pre[len], en_dict[ran]);
	} 
	elif (s->dic == unix)
	{
		ran = (rand() % 107);
		strcpy(bfs->pre[len], unix_dict[ran]);
	}
}

static int check_word(buffs *bfs, uint wp)
{
	ctl = 0;
	
	do
		if (bfs->pre[wp][ctl] != bfs->post[ctl])
			return 1;
	while (++ctl < strlen(bfs->pre[wp]));
	return 0;
}

static void print_pre(rt_info *rti, buffs *bfs, uint max, uint wln, uint wff_off)
{
	uint wcc_off = wff_off, y = 0, yw = 0;

	ctl = 0;
	wattron(rti->tpwin, COLOR_PAIR(3));
	do 
	{
		if ((wcc_off + strlen(bfs->pre[ctl])) > max)
		{
			++y;
			yw = y-wln;
			wcc_off = 0;
		}
		if (y >= wln)
			mvwaddstr(rti->tpwin,yw + 2, wcc_off + 3,bfs->pre[ctl]);
		wcc_off += (strlen(bfs->pre[ctl]) + 1);
	}
	while (++ctl < rti->words);
	wattroff(rti->tpwin, COLOR_PAIR(3));
}

static void print_post(rt_info *rti, buffs *bfs, uint wff_off, uint mistakes)
{
	uint wcc_off = wff_off;
	mvwaddstr(rti->tpwin, 2, wcc_off + 3, bfs->post);
	if (mistakes > 0)
	{
		ctl = 0;
		do
		{
			wattron(rti->tpwin, COLOR_PAIR(2));
			mvwaddch(rti->tpwin, 2, 3 + wcc_off + bfs->mistake_pos[ctl], bfs->pre[0][bfs->mistake_pos[ctl]]);
			wattroff(rti->tpwin, COLOR_PAIR(2));
		}
		while (++ctl < mistakes);
	}
}

static char setch(char *c)
{
	*c = (char) getch();
	return *c;
}

static void shift_buffers(buffs *bfs)
{
	ctl = 0;
	
	free(bfs->pre[0]);
	memmove(bfs->pre, bfs->pre + 1, sizeof(bfs->pre[0]) * 49);
	bfs->pre[49] = malloc(sizeof(char *));
	
	do
		bfs->post[ctl] = '\0';
	while (++ctl < 25);
}

static void input_loop(rt_info *rti, buffs *bfs, set *s, char *c)
{
	while (setch(c) < 1)
		if (s->gm == time_count && rti->start_time != 0 && (ulong) time(NULL) > (rti->start_time + rti->timer))
		{
			free_bufs(bfs);
			quit(rti, true, NULL);
		}
}

int main(int argc, char **argv)
{
	set *s,s_o =
	{
		.gm = gm_undefined,
		.dic = d_undefined,
		.stall = true,
		.stay = false
	};

	buffs *bfs,bfs_o;
	
	bool	hi=false, tt = false, ww = false, dd = false;
	uint	mistakes = 0, current_word_position = 0, wcc_off;
	uint	wff_off = 0;
	char 	c = '\0', last = '\0';
	
	rt_info *rti, rti_o =
	{
		.words = 0,
		.current_word = 0,
		.mistakes_total = 0,
		.start_time = 0,
		.timer = 0,
		.curcon = unstarted
	};
	
	bfs = &bfs_o;
	s = &s_o;
	rti = &rti_o;

	while (0 < --argc)
	{
		++argv;
		if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0)
		{
			quit(NULL, true,
			"Usage: tc [OPTION]\n"
			"   -h, --help          prints this\n"
			"   -v, --version       prints version\n"
			"   -w, --words         set gamemode to word\n"
			"   -t, --time          set gamemode to time\n"
			"   -e, --endless       set gamemode to endless\n"
			"   -d, --dictionary    set dictionary\n"
			"   -D, --dont-stall    don't stall on wrong words\n"
			"   -s, --stay-inplace  shift words forward\n"
			);
		}
		elif (strcmp(*argv, "--version") == 0 || strcmp(*argv, "-v") == 0)
		{
			quit(NULL, true, "Typing Curses V0\n");
		}
		elif (strcmp(*argv, "--words") == 0 || strcmp(*argv, "-w") == 0)
		{
			s->gm = words_count;
			ww = true;
		}
		elif (strcmp(*argv, "--time")==0 || strcmp(*argv, "-t") == 0)
		{
			s->gm = time_count;
			tt = true;
		}
		elif (strcmp(*argv, "--endless") == 0 || strcmp(*argv, "-e") == 0)
		{
			s->gm = endless;
		}
		elif (strcmp(*argv, "--dictionary") == 0 || strcmp(*argv, "-d") == 0)
		{
			dd = true;
		}
		elif (strcmp(*argv, "--stay-inplace") == 0 || strcmp(*argv, "-s") == 0)
		{
			s->stay = true;
		}
		elif (strcmp(*argv, "--dont-stall") == 0 || strcmp(*argv, "-D") == 0)
		{
			s->stall = false;
		}
		elif (tt)
		{
			int t = atoi(*argv);
			if (t < 0)
				quit(NULL, true, "invalid time\n");
			else
				rti->timer = (unsigned int) t;
			tt = false;
		}
		elif (ww)
		{
			int t = atoi(*argv);
			if (t < 0)
				quit(NULL, true, "invalid time\n");
			else
				rti->words = (uint) t;
			ww = false;
		}
		elif (dd)
		{
			if (strcmp(*argv, "en") == 0)
				s->dic = en;
			elif (strcmp(*argv, "unix") == 0)
				s->dic = unix;
			else
				quit(NULL, true, "dictionary not found\n");
		}
		else
			quit(NULL, true, "invalid argument\n");
	}
	if (tt || rti->timer == 0)
		rti->timer = 15;
	if (ww || rti->words == 0)
		rti->words = 40;
	if (s->gm == gm_undefined)
		s->gm = words_count;
	if (s->dic == d_undefined)
		s->dic = en;

	srand((uint) time(NULL));
	curses_init();
	
	alloc_buffers(bfs);
	
	ctl = 0;
	do
		generate_giberish(rti, bfs, s, ctl);
	while (++ctl < rti->words);
	
	while (1)
	{
		refresh();
		rti->tpwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0, 0, 0);
		wcc_off = (wff_off - (((wff_off + (uint) strlen(bfs->pre[0])) / (uint) (COLS - 6)) * (uint) (COLS - 6)));
		print_pre(rti, bfs, (uint) COLS - 6, 0, wcc_off);
		print_post(rti, bfs, wcc_off, mistakes);
		
		wcc_off += (strlen(bfs->pre[0]) + 1);
		wrefresh(rti->tpwin);
		refresh();
		create_keyboard((rti->start_time == 0), COLS, LINES, c, hi, last);
		last = c;
		hi = false;
		if (s->gm == words_count && rti->current_word == rti->words)
		{
			free_bufs(bfs);
			quit(rti, true, NULL);
		}
		input_loop(rti, bfs, s, &c);
		if (rti->curcon == unstarted)
			rti->start_time = (ulong) time(NULL);
		if (rti->curcon != eow)
			rti->curcon = insert;
		switch (c)
		{
			case 10:	/* Linebreak */
				c = ' ';
				break;
			case 11:	/* Tabulator */
				c = ' ';
				break;
			case 27:	/* Escape */
				free_bufs(bfs);
				quit(rti, true, NULL);
			case 127: /* Backspace */
				if (rti->curcon != eow)
				{
					if (current_word_position > 0)
						--current_word_position;
					bfs->post[current_word_position] = '\0';
					c = '\0';
					rti->curcon = backspace;
				}
				break;
			default:
			if (rti->curcon != eow && current_word_position < strlen(bfs->pre[0]))
				bfs->post[current_word_position] = c;
			else
				hi = true;
		}
		if (rti->curcon == backspace)
		{
			if (mistakes > 0 && bfs->mistake_pos[mistakes-1] == current_word_position)
				--mistakes;
		}
		elif (rti->curcon == insert)
		{
			if (current_word_position == strlen(bfs->pre[0]) - 1)
			{
					if (check_word(bfs, 0) == 0 || !s->stall)
					{
						current_word_position = 0;
						++rti->current_word;
						rti->curcon = eow;
						mistakes = 0;
						if (!s->stay)
							wff_off += strlen(bfs->post)+1;
						shift_buffers(bfs);
						if (s->gm == time_count || s->gm == endless)
							generate_giberish(rti, bfs, s, rti->words-1);
					}
					if (check_word(bfs, 0) == 1)
					{
						hi = true;
					}
			}
			if (rti->curcon != eow && current_word_position < strlen(bfs->pre[0]))
			{
				if (c != bfs->pre[0][current_word_position])
				{
					hi = true;
					bfs->mistake_pos[mistakes] = current_word_position;
					++mistakes;
					++rti->mistakes_total;
				}
				else
				{
					if (mistakes > 0 && bfs->mistake_pos[mistakes-1] == current_word_position)
						--mistakes;
					hi = false;
				}
				++current_word_position;
			}
		}
		elif (rti->curcon == eow && c == ' ')
		{
			hi = false;
			rti->curcon = insert;
		}
	}
}
