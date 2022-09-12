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

uint ctl;

typedef struct settings
{
	bool stall;
	bool stay;
}
set;

typedef struct buffers
{
	char **pre;
	char *post;
	uint *mistake_pos;
}
buffs;

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
	{
		bfs->pre[ctl] = malloc(sizeof(char) * 25);
	}
	while (++ctl < 50);
}

static void noreturn quit(ulong start_time, uint mistakes, uint wc, bool print, char *custom_log)
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

static void generate_giberish(buffs *bfs, uint len, enum dict dic)
{
	int ran;
	
	if (dic == en)
	{
		ran = (rand() % 1000);
		strcpy(bfs->pre[len], en_dict[ran]);
	} 
	elif (dic == unix)
	{
		ran = (rand() % 107);
		strcpy(bfs->pre[len], unix_dict[ran]);
	}
}

static int check_word(buffs *bfs, uint wp)
{
	ctl = 0;
	
	do
	{
		if (bfs->pre[wp][ctl] != bfs->post[ctl])
			return 1;
	}
	while (++ctl < strlen(bfs->pre[wp]));
	return 0;
}

static void print_pre(buffs *bfs, WINDOW *tpwin, uint words, uint max, uint wln, uint wff_off)
{
	uint wcc_off = wff_off, y = 0, yw = 0;

	ctl = 0;
	wattron(tpwin, COLOR_PAIR(3));
	do 
	{
		if ((wcc_off + strlen(bfs->pre[ctl])) > max)
		{
			++y;
			yw = y-wln;
			wcc_off = 0;
		}
		if (y >= wln)
			mvwaddstr(tpwin,yw + 2, wcc_off + 3,bfs->pre[ctl]);
		wcc_off += (strlen(bfs->pre[ctl]) + 1);
	}
	while (++ctl < words);
	wattroff(tpwin, COLOR_PAIR(3));
}

static void print_post(buffs *bfs, WINDOW *tpwin, uint wff_off, uint mistakes)
{
	uint wcc_off = wff_off;
	mvwaddstr(tpwin, 2, wcc_off + 3, bfs->post);
	if (mistakes > 0)
	{
		ctl = 0;
		do
		{
			wattron(tpwin, COLOR_PAIR(2));
			mvwaddch(tpwin, 2, 3 + wcc_off + bfs->mistake_pos[ctl], bfs->pre[0][bfs->mistake_pos[ctl]]);
			wattroff(tpwin, COLOR_PAIR(2));
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
	{
		bfs->post[ctl] = '\0';
	}
	while (++ctl < 25);
}

static void input_loop()
{
}

int main(int argc, char **argv)
{
	set *s,s_o =
	{ 
		.stall = false,
		.stay = false
	};

	buffs *bfs, bfs_decl;
	
	bool	hi=false, tt = false, ww = false, dd = false;
	ulong	start_time = 0, timer = 0;
	uint	mistakes_total = 0, mistakes = 0, current_word_position = 0, wcc_off, words = 0;
	uint	current_word_indp = 0;
	uint	wff_off = 0;
	char 	c = '\0', last = '\0';
	
	enum	gamemode gm = gm_undefined;
	enum	dict dic = d_undefined;
	enum	control curcon = insert;
	
	WINDOW	*tpwin;
	
	s = &s_o;
	bfs = &bfs_decl;

	while (0 < --argc)
	{
		++argv;
		if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0)
		{
			quit(0, 0, 0, true,
			"Usage: tc [OPTION]\n"
			"   -h, --help          prints this\n"
			"   -v, --version       prints version\n"
			"   -w, --words         set gamemode to word\n"
			"   -t, --time          set gamemode to time\n"
			"   -e, --endless       set gamemode to endless\n"
			"   -d, --dictionary    set dictionary\n"
			"   -s, --stall         stall on wrong words\n"
			"   -S, --stay-inplace  shift words forward\n"
			);
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
	
	alloc_buffers(bfs);
	
	ctl = 0;
	do
	{
		generate_giberish(bfs, ctl, dic);
	}
	while (++ctl < words);
	
	while (1)
	{
		refresh();
		tpwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0, 0, 0);
		wcc_off = (wff_off - (((wff_off + (uint) strlen(bfs->pre[0])) / (uint) (COLS - 6)) * (uint) (COLS - 6)));
		print_pre(bfs, tpwin, words, (uint) COLS - 6, 0, wcc_off);
		print_post(bfs, tpwin, wcc_off, mistakes);
		
		wcc_off += (strlen(bfs->pre[0]) + 1);
		wrefresh(tpwin);
		refresh();
		create_keyboard((start_time == 0), COLS, LINES, c, hi, last);
		last = c;
		hi = false;
		if (gm == words_count && current_word_indp == words)
		{
			free_bufs(bfs);
			quit(start_time, mistakes_total, current_word_indp, true, NULL);
		}
		while (setch(&c) < 1)
			if (gm == time_count && start_time != 0 && (ulong) time(NULL) > (start_time + timer))
			{
				free_bufs(bfs);
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
				free_bufs(bfs);
				quit(start_time, mistakes_total, current_word_indp, true, NULL);
			case 127: /* Backspace */
				if (curcon != eow)
				{
					if (current_word_position > 0)
						--current_word_position;
					bfs->post[current_word_position] = '\0';
					c = '\0';
					curcon = backspace;
				}
				break;
			default:
			if (curcon != eow && current_word_position < strlen(bfs->pre[0]))
				bfs->post[current_word_position] = c;
			else
				hi = true;
		}
		if (curcon == backspace)
		{
			if (mistakes > 0 && bfs->mistake_pos[mistakes-1] == current_word_position)
				--mistakes;
		}
		elif (curcon == insert)
		{
			if (current_word_position == strlen(bfs->pre[0]) - 1)
			{
					if (check_word(bfs, 0) == 0 || !s->stall)
					{
						current_word_position = 0;
						++current_word_indp;
						curcon = eow;
						mistakes = 0;
						if (!s->stay)
							wff_off += strlen(bfs->post)+1;
						shift_buffers(bfs);
						if (gm == time_count || gm == endless)
							generate_giberish(bfs, words-1, en);
					}
					if (check_word(bfs, 0) == 1)
					{
						hi = true;
					}
			}
			if (curcon != eow && current_word_position < strlen(bfs->pre[0]))
			{
				if (c != bfs->pre[0][current_word_position])
				{
					hi = true;
					bfs->mistake_pos[mistakes] = current_word_position;
					++mistakes;
					++mistakes_total;
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
		elif (curcon == eow && c == ' ')
		{
			hi = false;
			curcon = insert;
		}
	}
}
