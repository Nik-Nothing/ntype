#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

enum control{write,backspace};

WINDOW *create_newwin(int h,int w,int sy ,int sx,short pressed) {
	WINDOW *local_win;
	init_pair(1,COLOR_RED,COLOR_BLACK);

	local_win = newwin(h,w,sy,sx);
	box(local_win,0,0);
	if(pressed == 1)
		wbkgd(local_win,COLOR_PAIR(1));
	wrefresh(local_win);
	return local_win;
}

void create_keyboard(int len, int hei,char in) {
	int x=0,y=0;
	char c1[10] = {'q','w','e','r','t','z','u','i','o','p'};
	char c2[9] = {'a','s','d','f','g','h','j','k','l'};
	char c3[8] = {'y','x','c','v','b','n','m',','};
	while(x != 10) {
		if(c1[x] == in)
			create_newwin(hei/10,len/10,hei-(hei / 10)*4,(len / 10) * x,1);
		else
			create_newwin(hei/10,len/10,hei-(hei / 10)*4,(len / 10) * x,0);
		++x;
	}
	x = 0;
	while(x != 9) {
		if(c2[x] == in)
			create_newwin(hei / 10,len / 10 ,hei-(hei / 10)*3,((len / 10) * x) + (len / 25),1 ) ;
		else
			create_newwin(hei / 10,len / 10 ,hei-(hei / 10)*3,((len / 10) * x) + (len / 25),0 ) ;
		++x;
	}
	x = 0;
	while(x != 8) {
		if(c3[x] == in)
			create_newwin(hei / 10,len / 10 ,hei-(hei/10)*2, ((len / 10) * x) +((len / 25)*2),1);
		else
			create_newwin(hei / 10,len / 10 ,hei-(hei/10)*2, ((len / 10) * x) +((len / 25)*2),0);
		++x;
	}
	if(in == ' ')
		create_newwin(hei / 10, len / 2, hei-(hei/10), (len / 4),1);
	else
		create_newwin(hei / 10, len / 2, hei-(hei/10), (len / 4),0);
}

int main() {
	WINDOW *my_win;
	initscr();
	start_color();
	cbreak();
	noecho();
	int x = 0,y = 0;
	
	enum control curcon;

	char c;
	
	
	while(1) {
		refresh();
		create_keyboard(COLS,LINES,c);
		c = getchar();
		
		switch(c) {
			case 10: /* Linebreak */
				x = 0;
				++y;
				break;
			case 11: /* Tabulator */
				x += 8;
				c = ' ';
				break;
			case 27: /* Escape */
				endwin();
				return 0;
			case 127: /* Backspace */
				if(x > 0) {
					c = ' ';
					--x;
				}else if(y > 0) {
					--y;
				}
				c = ' ';
				curcon = backspace;
				break;
			default:
		}
		if(curcon == backspace) {
			mvaddch(y, x,c);
			move(y, x);
		} else {
			mvaddch(y, x,c);
			++x;
		}
		refresh();
		create_keyboard(COLS,LINES,c);
		curcon = write;
	}
}
