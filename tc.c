#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>


#define uint unsigned int
#define ulong unsigned long

#define elif else if


typedef enum contl
{
	unstarted,
	insert,
	backspace,
	eow
}
control;

typedef struct runtime_info
{
	uint    cp;
	uint    current_word;
	uint    mistakes_total;
	ulong   start_time;
	WINDOW  *tpwin;
	WINDOW  *rtwin;
	control curcon;
}
rt_info;

typedef struct buffers
{
	char    pre[100][100];
	char    post[100];
	uint    mistake_pos[100];
}
buffs;

typedef enum gamemd
{
	gm_undefined,
	words_count,
	time_count,
	endless
}
gamemode;


typedef struct settings
{
	gamemode  gm;
	ulong   timer;
	uint    words;
	bool      stall;
	bool      stay;
}
set;

char         dic   [1000][100];
size_t       dic_s = 0;
char         kmp   [31];
unsigned int thm   [6]   [3];


static void strip_newline(char *str)
{
	if (str[strlen(str) - 1] == '\n')
	{
		str[strlen(str) - 1] = '\0';
	}
}

static void rectangle(int y1, int x1, int y2, int x2)
{
	mvhline(y1, x1, 0, x2-x1);
	mvhline(y2, x1, 0, x2-x1);
	mvvline(y1, x1, 0, y2-y1);
	mvvline(y1, x2, 0, y2-y1);
	mvaddch(y1, x1, ACS_ULCORNER);
	mvaddch(y2, x1, ACS_LLCORNER);
	mvaddch(y1, x2, ACS_URCORNER);
	mvaddch(y2, x2, ACS_LRCORNER);
}

void load_dict(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) return;
	while(fgets(dic[dic_s], 100, fp)) strip_newline(dic[dic_s++]);
	fclose(fp);
}

void load_kmp(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) return;
	fgets(kmp, 30, fp);
	fclose(fp);
}

void load_thm(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) return;
	size_t x = 0;
	while (x < 6)
	{
		char col[10];
		if (!fgets(col, 10, fp)) break;
		sscanf(col, "#%2x%2x%2x", &thm[x][0], &thm[x][1], &thm[x][2] );
		thm[x][0] *= ( 1000 / 256 );
		thm[x][1] *= ( 1000 / 256 );
		thm[x][2] *= ( 1000 / 256 );
		++x;
	}
	fclose(fp);
}

void shift_buffers(buffs *bfs)
{
	uint ctl = 0;
	
	memmove(bfs->pre, bfs->pre + 1, sizeof(bfs->pre[0]) * 49);
	bfs->pre[49][0] =  '\0';
	
	bfs->post[ctl] = '\0';
}

void generate_giberish(rt_info *rti, buffs *bfs, set *s, uint len)
{
	int ran;
	
	ran = (rand() % dic_s);
	strcpy(bfs->pre[len], dic[ran]);
}

WINDOW *create_newwin(int h, int w, int sy, int sx)
{
	WINDOW *local_win;
	
	local_win = newwin(h, w, sy, sx);
	box(local_win, 0, 0);
	wbkgd(local_win, COLOR_PAIR(5));
	return local_win;
}

void srect(int h, int w, int sy, int sx)
{
	rectangle(sy, sx, sy + h, sx + w);
}

void update_win(int h, int w, int sy, int sx, bool pressed, bool hi)
{
	attron(COLOR_PAIR(!pressed ? 5 : hi ? 2 : 1));
	srect(h, w, sy, sx);
	attroff(COLOR_PAIR(!pressed ? 5 : hi ? 2 : 1));
}

void init_keyboard(buffs *bfs, int len, int hei)
{
	int  hr = 4, ofs = 0, idp = 0;
	
	uint ctl = 0;
	do
	{
		srect(hei / 17, hei / 9, hei - (hei / 15) * hr,((hei / 8) * idp) + ((hei / 15) * ofs) + ((len / 2) - (hei / 15) * 10));
		++idp;
		if (ctl == 9 || ctl == 19)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	while (++ctl != 29);
	
	srect(hei / 17, (hei / 9) * 5, hei - (hei / 15), (len / 2) - ((hei / 15) * 4));
}


void update_keyboard(buffs *bfs, int len, int hei, bool hi, char in, char last)
{
	int		hr = 4, ofs = 0, idp = 0;
	
	uint ctl = 0;
	do
	{
		if (kmp[ctl] == in || kmp[ctl] == last)
		{
			update_win(hei / 17, hei / 9, hei - (hei / 15) * hr,((hei / 8) * idp) + ((hei / 15) * ofs) + ((len / 2) - (hei / 15) * 10),(kmp[ctl] == in), hi);
		}
		++idp;
		if (ctl == 9 || ctl == 19)
		{
			--hr;
			++ofs;
			idp = 0;
		}
	}
	while (++ctl != 29);
	
	if (' ' == in || last == ' ')
	{
		update_win(hei / 17, (hei / 9) * 5, hei - (hei / 15), (len / 2) - ((hei / 15) * 4),(' ' == in), hi);
	}
}

static void curses_init()
{
	initscr();
	printf("\033[6 q");
	cbreak();
	noecho();
	timeout(1);
	start_color();
	
	init_color(1, thm[0][0], thm[0][1], thm[0][2]);
	init_color(2, thm[1][0], thm[1][1], thm[1][2]);
	init_color(3, thm[2][0], thm[2][1], thm[2][2]);
	init_color(4, thm[3][0], thm[3][1], thm[3][2]);
	init_color(5, thm[4][0], thm[4][1], thm[4][2]);
	init_color(6, thm[5][0], thm[5][1], thm[5][2]);
	init_pair(1, 6, 3);
	init_pair(2, 6, 4);
	init_pair(3, 1, 6);
	init_pair(4, 6, 6);
	init_pair(5, 5, 6);
}

static void noreturn quit(rt_info *rti, bool print, char *custom_log)
{
	ulong time_elps;
	
	printf("\033[1 q");
	endwin();
	if (print)
	{
		if (!custom_log)
		{
			time_elps = (unsigned long) time(NULL) - rti->start_time;
			printf("%d Words in %lu Seconds! that is %.2f WPM, you made %d Mistakes\n", rti->current_word, time_elps, (double) 60 * ( (double) (rti->cp / 5) / (double) time_elps), rti->mistakes_total);
		}
		else
		{
			printf("%s", custom_log);
		}
	}
	exit(0);
}

static void print_pre(set *s, rt_info *rti, buffs *bfs, uint max, uint wln, uint wff_off)
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
		{
			mvwaddstr(rti->tpwin,yw + 2, wcc_off + 3,bfs->pre[ctl]);
		}
		wcc_off += (strlen(bfs->pre[ctl]) + 1);
	}
	while (++ctl < s->words);
	wattroff(rti->tpwin, COLOR_PAIR(3));
}

static void print_post(rt_info *rti, buffs *bfs, uint wff_off, uint mistakes)
{
	uint ctl = 0, wcc_off = wff_off;
	mvwaddstr(rti->tpwin, 2, wcc_off + 3, bfs->post);
	while (ctl < mistakes)
	{
		wattron(rti->tpwin, COLOR_PAIR(2));
		mvwaddch(rti->tpwin, 2, 3 + wcc_off + bfs->mistake_pos[ctl], bfs->pre[0][bfs->mistake_pos[ctl]]);
		wattroff(rti->tpwin, COLOR_PAIR(2));
		ctl++;
	}
	wmove(rti->tpwin, 2, wcc_off + 3 + strlen(bfs->post));
}

static char setch(char *c)
{
	*c = (char) getch();
	return *c;
}

static void input_loop(rt_info *rti, buffs *bfs, set *s, char *c, char l, size_t wcc_off)
{
	int ti = 0;
	while (setch(c) < 1)
	{
		if (ti++ > 100)
		{
			update_keyboard(bfs, COLS, LINES, 0, 0, l);
			if (rti->curcon == eow)
			{
				move(2, wcc_off + 2 + strlen(bfs->post));
			}
			else
			{
				move(2, wcc_off + 3 + strlen(bfs->post));
			}
			wrefresh(rti->tpwin);
		}
		if (s->gm == time_count && rti->start_time != 0 && (ulong) time(NULL) > (rti->start_time + s->timer))
		{
			quit(rti, true, NULL);
		}
	}
}

static void init_info(rt_info *rti, int len, int hei)
{
	rti->rtwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0);
}

static void update_info(rt_info *rti)
{
	return;
}

int loop(set *s)
{
	buffs *bfs,bfs_o =
	{
		.post = ""
	};
	
	bool hi=false;
	uint mistakes = 0, current_word_position = 0, wcc_off;
	uint wff_off = 0, ctl = 0;
	char c = '\0', last = '\0';
	
	rt_info *rti, rti_o =
	{
		.current_word = 0,
		.mistakes_total = 0,
		.start_time = 0,
		.curcon = unstarted,
		.cp = 0
	};
	
	bfs = &bfs_o;
	rti = &rti_o;
	
	
	srand((uint) time(NULL));
	curses_init();
	wbkgd(stdscr, COLOR_PAIR(5));
	ctl = 0;
	do
	{
		generate_giberish(rti, bfs, s, ctl);
	}
	while (++ctl < s->words);
	
	init_keyboard(bfs, COLS, LINES);
	
	rti->tpwin = create_newwin(LINES - (LINES / 14) * 4, COLS, 0, 0);
	wbkgd(rti->tpwin, COLOR_PAIR(5));
	
	while (1)
	{
		refresh();
		
		werase(rti->tpwin);
		box(rti->tpwin, 0, 0);
		if(!s->stay)	
		{
			wcc_off = wff_off;
		}
		else
		{
			wcc_off = 0;
		}
		if (rti->curcon == unstarted)
		{
			init_keyboard(bfs, COLS, LINES);
		}
		update_keyboard(bfs, COLS, LINES, hi, c, last);
		print_pre(s, rti, bfs, (uint) COLS - 6, 0, wcc_off);
		print_post(rti, bfs, wcc_off, mistakes);
		if (rti->curcon == eow)
		{
			move(2, wcc_off + 2 + strlen(bfs->post));
		}
		else
		{
			move(2, wcc_off + 3 + strlen(bfs->post));
		}
		wrefresh(rti->tpwin);
		refresh();
		
		last = c;
		hi = false;
		if (s->gm == words_count && rti->current_word == s->words)
		{
			quit(rti, true, NULL);
		}
		input_loop(rti, bfs, s, &c, last, wcc_off);
		wcc_off += (strlen(bfs->pre[0]) + 1);
		if (rti->curcon == unstarted)
		{
			rti->start_time = (ulong) time(NULL);
		}
		if (rti->curcon != eow)
		{
			rti->curcon = insert;
		}
		switch (c)
		{
			case 10:	/* Linebreak */
			{
				c = ' ';
				break;
			};
			case 11:	/* Tabulator */
			{
				return;
			};
			case 27:	/* Escape */
			{
				quit(rti, true, NULL);
			};
			case 127: /* Backspace */
			{
				if (rti->curcon != eow)
				{
					if (current_word_position > 0)
					{
						--current_word_position;
					}
					bfs->post[current_word_position] = '\0';
					c = '\0';
					rti->curcon = backspace;
				}
				break;
			};
			default:
				if (rti->curcon != eow && current_word_position < strlen(bfs->pre[0]))
				{
					bfs->post[current_word_position] = c;
					bfs->post[current_word_position + 1] = '\0';
				}
				else
				{
					hi = true;
				}
		}
		if (rti->curcon == backspace)
		{
			if (mistakes > 0 && bfs->mistake_pos[mistakes-1] == current_word_position)
			{
				--mistakes;
			}
		}
		elif (rti->curcon == insert)
		{
			if (current_word_position == strlen(bfs->pre[0]) - 1)
			{
				{
					if (!strcmp(bfs->pre[0], bfs->post) || !s->stall)
					{
						current_word_position = 0;
						++rti->current_word;
						rti->curcon = eow;
						mistakes = 0;
						if (!s->stay)
						{
							wff_off += strlen(bfs->pre[0])+1;
							rti->cp += strlen(bfs->pre[0])+1;
							if(1 <= ((wff_off + strlen(bfs->pre[1]))) / ((uint) COLS - 6))
							{
								wff_off = 0;
							}
						}
						shift_buffers(bfs);
						if (s->gm == time_count || s->gm == endless)
						{
							generate_giberish(rti, bfs, s, s->words-1);
						}
					}
					else
					{
						hi = true;
					}
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
					{
						--mistakes;
					}
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

int main(int argc, char **argv)
{
	bool tt = false, ww = false, dd = false, th = false, ths = false;
	set s_o =
	{
		.gm = gm_undefined,
		.stall = true,
		.stay = false
	};
	
	while (0 < --argc)
	{
		++argv;
		if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0)
		{
			quit(NULL, true, \
			"Usage: tc [OPTION]\n" \
			"   -h, --help          prints this\n" \
			"   -v, --version       prints version\n" \
			"   -w, --words         set gamemode to word\n" \
			"   -t, --time          set gamemode to time\n" \
			"   -e, --endless       set gamemode to endless\n" \
			"   -d, --dictionary    set dictionary\n" \
			"   -D, --dont-stall    don't stall on wrong words\n" \
			"   -s, --stay-inplace  shift words forward\n" \
			);
		}
		elif (strcmp(*argv, "--version") == 0 || strcmp(*argv, "-v") == 0)
		{
			quit(NULL, true, "Typing Curses V0\n");
		}
		elif (strcmp(*argv, "--words") == 0 || strcmp(*argv, "-w") == 0)
		{
			s_o.gm = words_count;
			ww = true;
		}
		elif (strcmp(*argv, "--time")==0 || strcmp(*argv, "-t") == 0)
		{
			s_o.gm = time_count;
			tt = true;
		}
		elif (strcmp(*argv, "--endless") == 0 || strcmp(*argv, "-e") == 0)
		{
			s_o.gm = endless;
		}
		elif (strcmp(*argv, "--dictionary") == 0 || strcmp(*argv, "-d") == 0)
		{
			dd = true;
		}
		elif (strcmp(*argv, "--stay-inplace") == 0 || strcmp(*argv, "-s") == 0)
		{
			s_o.stay = true;
		}
		elif (strcmp(*argv, "--theme") == 0 || strcmp(*argv, "-H") == 0)
		{
			th = true;
		}
		elif (strcmp(*argv, "--dont-stall") == 0 || strcmp(*argv, "-D") == 0)
		{
			s_o.stall = false;
		}
		elif (tt)
		{
			int t = atoi(*argv);
			if (t < 0)
			{
				quit(NULL, true, "invalid time\n");
			}
			else
			{
				s_o.timer = (unsigned int) t;
			}
			tt = false;
		}
		elif (ww)
		{
			int t = atoi(*argv);
			if (t < 0)
			{
				quit(NULL, true, "invalid time\n");
			}
			else
			{
				s_o.words = (uint) t;
			}
			ww = false;
		}
		elif (dd)
		{
			if (strcmp(*argv, "en") == 0)
			{
				load_dict("en.dic");
			}
			elif (strcmp(*argv, "unix") == 0)
			{
				load_dict("unix.dic");
			}
			else
			{
				quit(NULL, true, "dictionary not found\n");
			}
		}
		elif (th)
		{
			load_thm(*argv);
			ths = true;
		}
		else
		{
			quit(NULL, true, "invalid argument\n");
		}
	}
	if (tt || s_o.timer == 0)
	{
		s_o.timer = 15;
	}
	if (ww || s_o.words == 0)
	{
		s_o.words = 40;
	}
	if (s_o.gm == gm_undefined)
	{
		s_o.gm = words_count;
	}
	load_dict("dicts/en.dic");
	load_kmp("keymaps/dvorak.kmp");
	if (!ths)
	{
		load_thm("themes/default.theme");
	}
	loop(&s_o);
}
