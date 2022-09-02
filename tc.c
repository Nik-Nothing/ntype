#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<curses.h>

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

void generate_giberish(char* buf,int len) {
	char dict[100][100] = {"the","of","and","a","to","in","is","you","that","it","he","was","for","on","are","as","with","his","they","I","at","be","this","have","from","or","one","had","by","word"};
	int ran;
	while(--len > 0) {
		ran = rand() % 29;
		strcat(buf,dict[ran]);
		strcat(buf," ");
	}
}

int main() {
	WINDOW *my_win;
	initscr();
	start_color();
	init_color(COLOR_CYAN,250,250,250);
	init_pair(2,COLOR_CYAN,COLOR_BLACK);
	cbreak();
	noecho();
	int x = 0,y = 0;
	char* pre = malloc(sizeof(char) * 100);
	generate_giberish(pre,25);

	//strcpy(pre,"this is what you have to type");
	
	char* post = malloc(sizeof(char) * 100);
	enum control curcon;

	char c;	
	
	while(1) {
		attron(COLOR_PAIR(2));
		mvaddstr(y,0,pre);
		attroff(COLOR_PAIR(2));
		mvaddstr(y,0,post);
		refresh();
		create_keyboard(COLS,LINES,c);
		c = getchar();	
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
				if(x > 0) {
					--x;
				}else if(y > 0) {
					--y;
				}
				post[x] = '\0';
				curcon = backspace;
				break;
			default:
			post[x] = c;
		}
		if(curcon == backspace) {
			//mvaddch(y, x,c);
			move(y, x);
		} else {
			//mvaddch(y,x,c);
			++x;
		}
		refresh();
		create_keyboard(COLS,LINES,c);
		curcon = write;
	}
}
