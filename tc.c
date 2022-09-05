#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include<curses.h>

#include"dict.h"

enum control{write,backspace};

WINDOW *create_newwin(int h,int w,int sy ,int sx,short pressed,short cor) {
	WINDOW *local_win;

	local_win = newwin(h,w,sy,sx);
	box(local_win,0,0);
	if(pressed == 1)
		wbkgd(local_win,COLOR_PAIR(1+cor));
	wrefresh(local_win);
	return local_win;
}

void create_keyboard(int len, int hei,char in,short cor) {
	int x=-1,y=0;
	char c1[27] = {'q','w','e','r','t','z','u','i','o','p',
					'a','s','d','f','g','h','j','k','l',
					  'y','x','c','v','b','n','m',','};
	int hr = 4,ofs=0,idp=0;
	while(++x != 27) {
		create_newwin(hei / 14, hei / 8,hei-(hei / 14)*hr,((hei / 8) * idp) + ((hei / 20) * ofs) + ((len / 2) - (hei / 8)*5),c1[x] == in ? 1 : 0,cor);
		++idp;
		if(x == 9 || x == 18){
			--hr;
			++ofs;
			idp = 0;
		}
	}
	create_newwin(hei / 14, (hei / 8) * 5, hei-(hei/14), (len / 2) - ((hei /14)*4),in == ' ' ? 1 : 0,cor);
}

void generate_giberish(char* buf,int len) {
	int ran;
	srand(time(NULL));
	while(--len > 0) {
		ran = (rand() % 1000);
		strcat(buf,en_dict[ran]);
		strcat(buf," ");
	}
}

int main() {
	int start_time=(unsigned long)time(NULL), wc = 50, x = 0, y = 0, z,w,g,f, length, mistakes = 0, cor = 0, *mistake_pos, time_elps;
	char c,*pre,*post,*mis;
	enum control curcon = write;
	WINDOW *tpwin;
	
	initscr();
	curs_set(0);
	cbreak();
	noecho();
	start_color();
	
	init_color(COLOR_CYAN,350,350,350);
	init_pair(1,COLOR_BLACK,COLOR_GREEN);
	init_pair(2,COLOR_BLACK,COLOR_RED);
	init_pair(3,COLOR_CYAN,COLOR_BLACK);
	
	pre = malloc(sizeof(char) * 10000);
	post = malloc(sizeof(char) * 10000);
	mis = malloc(sizeof(char) * 1000);
	mistake_pos = malloc(sizeof(int) * 100);

	generate_giberish(pre,wc);
	length = strlen(pre);

	
	while(1) {
		refresh();
		create_newwin(LINES-(LINES/14)*4,COLS,0,0,0,0);
		tpwin = newwin((LINES-(LINES/14)*4)-4,COLS-6,2,3);
		wattron(tpwin,COLOR_PAIR(3));
		mvwaddstr(tpwin,y,0,pre);
		wattroff(tpwin,COLOR_PAIR(3));
		mvwaddstr(tpwin,y,0,post);
		z = -1;
		while(++z != mistakes) {
			w = (mistake_pos[z] / (COLS - 6));
			g = mistake_pos[z] - ( (mistake_pos[z] / (COLS - 6)) * (COLS-6));;
			wattron(tpwin,COLOR_PAIR(2));
			mvwaddch(tpwin,w,g,mis[z]);
			wattroff(tpwin,COLOR_PAIR(2));
		}
		wrefresh(tpwin);
		refresh();
		create_keyboard(COLS,LINES,c,cor);
		cor = 0;
		if(x == length-1)
				break;
		c = getchar();	
		curcon = write;
		switch(c) {
			case 10: /* Linebreak */
				c = ' ';
				break;
			case 11: /* Tabulator */
				x += 8;
				c = ' ';
				break;
			case 27: /* Escape */
				endwin();
				return 0;
			case 127: /* Backspace */
				if(x > 0)
					--x;
				else if(y > 0)
					--y;
				post[x] = '\0';
				c = '\0';
				curcon = backspace;
				break;
			default:
			post[x] = c;
		}
		if(curcon == backspace){
			if(mistakes != 0 && mistake_pos[mistakes-1] == x)
					--mistakes;
			move(y, x);
		} else {
			if(c != pre[x]) {
				cor = 1;
				mistake_pos[mistakes] = x;
				mis[mistakes] = c;
				++mistakes;
			} else {
				f = -1;
				while(++f != mistakes)
					if(mistake_pos[f] == x)
						--mistakes;
			}
			++x;
		}
	}
	endwin();
	time_elps = (unsigned long)time(NULL) - start_time;
	printf("%d Words in %d Seconds! that is %.2f WPM, you made %d Mistakes\n",wc,time_elps,(float)60*((float)wc / (float)time_elps), mistakes) ;
	return 0;
}
