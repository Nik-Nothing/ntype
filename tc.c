#include<stdio.h>
#include<stdlib.h>
#include<stdnoreturn.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<ctype.h>

#include<curses.h>

#include"dict.h"


enum gamemode{gm_undefined,words_count,time_count,endless};

enum control{insert,backspace,eow};

typedef struct settings{
	bool stall;
	bool stay_in_one_spot;
} set;

static noreturn void quit(unsigned long start_time,unsigned int mistakes,unsigned int wc,bool print,char* custom_log) {
	unsigned long time_elps; 
	endwin();
	if(print) {
		if(custom_log == NULL) {
			time_elps = (unsigned long)time(NULL) - start_time;
			printf("%d Words in %lu Seconds! that is %.2f WPM, you made %d Mistakes\n",wc,time_elps,(double)60*((double)wc / (double)time_elps), mistakes) ;
		} else
			printf("%s",custom_log);
	}
	exit(0);
}

static WINDOW *create_newwin(int h,int w,int sy ,int sx,bool pressed,bool hi) {
	WINDOW *local_win;

	local_win = newwin(h,w,sy,sx);
	box(local_win,0,0);
	if(pressed)
		wbkgd(local_win,COLOR_PAIR(hi ? 2 : 1));
	wrefresh(local_win);
	return local_win;
}

static void create_keyboard(int len, int hei,char in,bool hi) {
	int x=-1;
	char c1[27] = {'q','w','e','r','t','z','u','i','o','p',
					'a','s','d','f','g','h','j','k','l',
					  'y','x','c','v','b','n','m',','};
	int hr = 4,ofs=0,idp=0;
	while(++x != 27) {
		create_newwin(hei / 14, hei / 8,hei-(hei / 14)*hr,((hei / 8) * idp) + ((hei / 20) * ofs) + ((len / 2) - (hei / 8)*5),(c1[x] == in),hi);
		++idp;
		if(x == 9 || x == 18){
			--hr;
			++ofs;
			idp = 0;
		}
	}
	create_newwin(hei / 14, (hei / 8) * 5, hei-(hei/14), (len / 2) - ((hei /14)*4),in == ' ' ? 1 : 0,hi);
}

static void generate_giberish(char** buf,unsigned int len,enum dict dic) {
	int ran;
	if(dic == en) {
		ran = (rand() % 1000);
		strcpy(buf[len],en_dict[ran]);
	} else if(dic == unix) {
		ran = (rand() % 107);
		strcpy(buf[len],unix_dict[ran]);
	}
}

static int check_word(char** buf_pre,char** buf_post,unsigned int wp) {
	unsigned int x = 0;
	do {
		if(buf_pre[wp][x] != buf_post[wp][x])
			return 1;
	}while(++x < strlen(buf_pre[wp]));
	return 0;
}

static void print_pre(char ** words_pre,WINDOW* tpwin,unsigned int words, unsigned int max,unsigned int wln){
	 unsigned int wcc=0,wcc_off=0,y=0,yw=0;
	wattron(tpwin,COLOR_PAIR(3));
	do {
			if((wcc_off + strlen(words_pre[wcc])) > max){
				++y;
				yw = y-wln;
				wcc_off = 0;
			}
			if(y >= wln)
				mvwaddstr(tpwin,yw,wcc_off,words_pre[wcc]);
			wcc_off += (strlen(words_pre[wcc])+1);
	}while(++wcc < words);
	wattroff(tpwin,COLOR_PAIR(3));
}
static char setch(char* c) {
	*c = (char)getch();
	return *c;
}

int main(int argc,char** argv) {
	bool hi=false;
	unsigned long start_time = 0,timer = 0;
	unsigned int mistakes_total = 0,z,mistakes = 0,y,wln = 0,cts = 0,current_word_position = 0,*mistake_pos,wcc_off,current_word = 0,words = 0,gn_off = 20,wcc;
	char c = '\0',**words_pre,**words_post;
	enum gamemode gm = gm_undefined;
	enum dict dic = d_undefined;
	enum control curcon = insert;
	WINDOW *tpwin;

	int arg=0;
	bool tt = false,ww = false,dd = false;
	while(++arg < argc) {
		if(strcmp(argv[arg],"--help")==0 || strcmp(argv[arg],"-h")==0) {
			quit(0,0,0,true,"help\n");
		} else if(strcmp(argv[arg],"--version")==0 || strcmp(argv[arg],"-v")==0) {
			quit(0,0,0,true,"Typing Curses V0\n");
		} else if(strcmp(argv[arg],"--words")==0 || strcmp(argv[arg],"-w")==0) {
			gm = words_count;
			ww = true;
		} else if(strcmp(argv[arg],"--time")==0 || strcmp(argv[arg],"-t")==0) {
			gm = time_count;
			tt = true;
		} else if(strcmp(argv[arg],"--endless")==0 || strcmp(argv[arg],"-e")==0) {
			gm = endless;
		} else if(strcmp(argv[arg],"--dictionary")==0 || strcmp(argv[arg],"-d")==0) {
			dd = true;
		} else if(tt) {
			int t = atoi(argv[arg]);
			if(t < 0)
				quit(0,0,0,true,"invalid time\n");
			else
				timer = (unsigned int)t;
			tt = false;
		} else if(ww) {
			int t = atoi(argv[arg]);
			if(t < 0)
				quit(0,0,0,true,"invalid time\n");
			else
				words = (unsigned int)t;
			ww = false;
		} else if(dd) {
			if(strcmp(argv[arg],"en") == 0)
				dic = en;
			else if(strcmp(argv[arg],"unix") == 0)
				dic = unix;
		} else
			quit(0,0,0,true,"invalid argument\n");
	}
	if(tt || timer == 0)
		timer = 15;
	if(ww || words == 0)
		words = 15;
	if(gm == gm_undefined)
		gm = words_count;
	if(dic == d_undefined)
		dic = en;

	initscr();
	curs_set(0);
	cbreak();
	noecho();
	timeout(1);
	start_color();
	srand((unsigned int)time(NULL));
	
	init_color(COLOR_CYAN,350,350,350);
	init_pair(1,COLOR_BLACK,COLOR_GREEN);
	init_pair(2,COLOR_BLACK,COLOR_RED);
	init_pair(3,COLOR_CYAN,COLOR_BLACK);
	init_pair(4,COLOR_BLACK,COLOR_BLACK);
	
	mistake_pos = malloc(sizeof(int) * 20);

	words_pre = malloc(sizeof(char*) * 100);
	words_post = malloc(sizeof(char*) * 100);
	
	cts=0;
	do{
		words_pre[cts] = malloc(sizeof(char) * 25);
		words_post[cts] = malloc(sizeof(char) * 25);
	}while(++cts < 100);
	cts = 0;
	do {
		generate_giberish(words_pre,cts,dic);
	}while(++cts < words);
	
	while(1) {
		refresh();
		create_newwin(LINES-(LINES/14)*4,COLS,0,0,0,0);
		tpwin = newwin((LINES-(LINES/14)*4)-4,COLS-6,2,3);
		cts=0;
		do {
			werase(tpwin);
			print_pre(words_pre,tpwin,words,(unsigned int)COLS-6,wln);
			wcc=0;
			wcc_off=0;
			y=0;
			cts = 0;
			do {
				if((wcc_off + strlen(words_pre[wcc]))> (unsigned int)COLS-6){
					++y;
					if(y>wln){
						++wln;
						cts = 1;
					}
					wcc_off = 0;
				}
				if(y >= wln){
					if(wcc < current_word)
						wattron(tpwin,COLOR_PAIR(4));
					mvwaddstr(tpwin,0,wcc_off,words_post[wcc]);
					if(wcc < current_word)
						wattroff(tpwin,COLOR_PAIR(4));
					
					if(wcc == current_word && mistakes > 0) {
						z = 0;
						do {
							wattron(tpwin,COLOR_PAIR(2));
							mvwaddch(tpwin,0,wcc_off + mistake_pos[z],words_pre[current_word][mistake_pos[z]]);
							wattroff(tpwin,COLOR_PAIR(2));
						}while(++z < mistakes);
					}
				}
					wcc_off += (strlen(words_pre[wcc])+1);
			}while(++wcc < current_word+1);
		}while (cts == 1);
		wrefresh(tpwin);
		refresh();
		create_keyboard(COLS,LINES,c,hi);
		hi = false;
		if(gm == words_count && current_word == words)
				quit(start_time,mistakes_total,words,true,NULL);
		if((gm == endless || gm == time_count) && current_word == words - gn_off) {
			unsigned int before = words;
			do {
				generate_giberish(words_pre,words,dic);
			}while(++words < before + gn_off);
		}
		while(setch(&c) < 1 ) {
			if(gm == time_count && start_time != 0 && (unsigned long)time(NULL) > ( start_time + timer)){
				quit(start_time,mistakes_total,current_word,true,NULL);
			}
		}
		if(start_time == 0)
			start_time = (unsigned long)time(NULL);
		if(curcon != eow)
			curcon = insert;
		switch(c) {
			case 10: /* Linebreak */
				c = ' ';
				break;
			case 11: /* Tabulator */
				c = ' ';
				break;
			case 27: /* Escape */
				quit(start_time,mistakes_total,current_word,true,NULL);
			case 127: /* Backspace */
				if(curcon != eow) {
					if(current_word_position > 0)
						--current_word_position;
					words_post[current_word][current_word_position] = '\0';
					c = '\0';
					curcon = backspace;
				}
				break;
			default:
			if(curcon != eow && current_word_position < strlen(words_pre[current_word]))
				words_post[current_word][current_word_position] = c;
			else
				hi = true;
		}
		if(curcon == backspace){
			if(mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
					--mistakes;
		} else if(curcon == insert){
			if(current_word_position == strlen(words_pre[current_word])-1 && check_word(words_pre,words_post,current_word) == 0) {
					current_word_position = 0;
					++current_word;
					curcon = eow;
			}
			if(curcon != eow && current_word_position < strlen(words_pre[current_word])){
				if(c != words_pre[current_word][current_word_position]) {
					hi = true;
					mistake_pos[mistakes] = current_word_position;
					++mistakes;
					++mistakes_total;
				} else {
					if(mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
						--mistakes;
					hi = false;
				}
				++current_word_position;
			}
		} else if(curcon == eow && c == ' '){
			hi = false;
			curcon = insert;
		}
	}
}
