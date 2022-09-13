#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

#include <string.h>
#include <time.h>
#include <ctype.h>

#include <curses.h>

#include "macros.h"
#include "tc.h"

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
	init_pair(5, COLOR_WHITE, COLOR_BLACK);
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

static int check_word(buffs *bfs)
{
	uint ctl = 0;
	
	do
		if (bfs->pre[0][ctl] != bfs->post[ctl])
			return 1;
	while (++ctl < strlen(bfs->pre[0]));
	return 0;
}

static void print_pre(rt_info *rti, buffs *bfs, uint max, uint wln, uint wff_off)
{
	uint wcc_off = wff_off, y = 0, yw = 0;

	uint ctl = 0;
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
	uint ctl, wcc_off = wff_off;
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


static void input_loop(rt_info *rti, buffs *bfs, set *s, char *c)
{
	while (setch(c) < 1)
		if (s->gm == time_count && rti->start_time != 0 && (ulong) time(NULL) > (rti->start_time + rti->timer))
		{
			free_bufs(bfs);
			quit(rti, true, NULL);
		}
}

static void init_info(rt_info *rti, int len, int hei)
{
	rti->rtwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0);
}

static void update_info(rt_info *rti)
{
	
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
	uint	wff_off = 0, ctl = 0;
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
	
	init_keyboard(bfs, COLS, LINES);
	
	rti->tpwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0);
	
	while (1)
	{
		refresh();
		
		werase(rti->tpwin);
		box(rti->tpwin, 0, 0);
		if(!s->stay)	
			wcc_off = wff_off;
		else
			wcc_off = 0;
		print_pre(rti, bfs, (uint) COLS - 6, 0, wcc_off);
		print_post(rti, bfs, wcc_off, mistakes);
		
		wcc_off += (strlen(bfs->pre[0]) + 1);
		wrefresh(rti->tpwin);
		refresh();
		if (rti->curcon == unstarted)
			init_keyboard(bfs, COLS, LINES);
		update_keyboard(bfs, COLS, LINES, hi, c, last);
		last = c;
		hi = false;
		if (s->gm == words_count && rti->current_word == rti->words)
		{
			free_bufs(bfs);
			quit(rti, true, NULL);
		}
		input_loop(rti, bfs, s, &c);
		if (rti->curcon == unstarted){
			rti->start_time = (ulong) time(NULL);
		}
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
					if (check_word(bfs) == 0 || !s->stall)
					{
						current_word_position = 0;
						++rti->current_word;
						rti->curcon = eow;
						mistakes = 0;
						if (!s->stay)
						{
							uint wff_off_b = wff_off;
							wff_off += strlen(bfs->post)+1;
							if(wff_off_b > wff_off - (wff_off / (((uint) COLS - 6)) * (uint) COLS - 6))
								wff_off = 0;
						}
						shift_buffers(bfs);
						if (s->gm == time_count || s->gm == endless)
							generate_giberish(rti, bfs, s, rti->words-1);
					}
					elif (check_word(bfs) == 1)
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
