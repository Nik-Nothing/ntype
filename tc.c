#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<curses.h>

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
	int x=0,y=0;
	char c1[10] = {'q','w','e','r','t','z','u','i','o','p'};
	char c2[9] = {'a','s','d','f','g','h','j','k','l'};
	char c3[8] = {'y','x','c','v','b','n','m',','};
	while(x != 10) {
		if(c1[x] == in)
			create_newwin(hei/10,len/10,hei-(hei / 10)*4,(len / 10) * x,1,cor);
		else
			create_newwin(hei/10,len/10,hei-(hei / 10)*4,(len / 10) * x,0,cor);
		++x;
	}
	x = 0;
	while(x != 9) {
		if(c2[x] == in)
			create_newwin(hei / 10,len / 10 ,hei-(hei / 10)*3,((len / 10) * x) + (len / 25),1,cor) ;
		else
			create_newwin(hei / 10,len / 10 ,hei-(hei / 10)*3,((len / 10) * x) + (len / 25),0,cor) ;
		++x;
	}
	x = 0;
	while(x != 8) {
		if(c3[x] == in)
			create_newwin(hei / 10,len / 10 ,hei-(hei/10)*2, ((len / 10) * x) +((len / 25)*2),1,cor);
		else
			create_newwin(hei / 10,len / 10 ,hei-(hei/10)*2, ((len / 10) * x) +((len / 25)*2),0,cor);
		++x;
	}
	if(in == ' ')
		create_newwin(hei / 10, len / 2, hei-(hei/10), (len / 4),1,cor);
	else
		create_newwin(hei / 10, len / 2, hei-(hei/10), (len / 4),0,cor);
}

void generate_giberish(char* buf,int len) {
	char dict[101][101] = {"the","of","and","a","to","in","is","you","that","it","he","was","for","on","are","as","with","his","they","I","at","be","this","have","from","or","one","had","by","word","but","not","what","all","were","we","when","your","can","said","there","use","an","each","which","she","do","how","their","if","will","up","other","about","out","many","then","them","these","so","some","her","would","make","like","him","into","time","has","look","two","more","write","go","see","number","no","way","could","people","my","than","first","water","been","call","who","oil","its","now","find","long","down","day","did","get","come","made","may","part"};
	int ran;
	while(--len > 0) {
		ran = rand() % 100;
		strcat(buf,dict[ran]);
		strcat(buf," ");
	}
}

int main() {
	int start_time=(unsigned long)time(NULL);
	srand(time(NULL));
	WINDOW *my_win;
	initscr();
	curs_set(0);
	start_color();
	init_color(COLOR_CYAN,350,350,350);
	init_pair(1,COLOR_BLACK,COLOR_GREEN);
	init_pair(2,COLOR_BLACK,COLOR_RED);
	init_pair(3,COLOR_CYAN,COLOR_BLACK);
	init_pair(4,COLOR_RED,COLOR_BLACK);
	cbreak();
	noecho();
	int x = 0,y = 0;
	char* pre = malloc(sizeof(char) * 1000);
	int wc = 20;
	generate_giberish(pre,wc);
	int length = strlen(pre);

	char* post = malloc(sizeof(char) * 1000);
	enum control curcon = write;
	char c;
	int mistakes = 0;
	WINDOW *tpwin;
	while(1) {
		refresh();
		tpwin = create_newwin(LINES-(LINES/10)*4,COLS,0,0,0,0);
		wattron(tpwin,COLOR_PAIR(3));
		mvwaddstr(tpwin,y+1,1,pre);
		wrefresh(tpwin);
		wattroff(tpwin,COLOR_PAIR(3));
		mvwaddstr(tpwin,y+1,1,post);
		wrefresh(tpwin);
		int cor=0;
		if(pre[x-1] != post[x-1] && curcon != backspace){
			wattron(tpwin,COLOR_PAIR(2));
			mvwaddch(tpwin,y+1,x,c);
			wattroff(tpwin,COLOR_PAIR(2));
		wrefresh(tpwin);
			cor=1;
			++mistakes;
		}
		refresh();
		create_keyboard(COLS,LINES,c,cor);
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
				if(x > 0) {
					--x;
				}else if(y > 0) {
					--y;
				}
				post[x] = '\0';
				c = '\0';
				curcon = backspace;
				break;
			default:
			post[x] = c;
		}
		if(curcon == backspace) {
			move(y, x);
		} else {
			++x;
		}
	}
	endwin();
	int time_elps = (unsigned long)time(NULL) - start_time;
	printf("%d Words in %d Seconds! that is %.2f WPM, you made %d Mistakes\n",wc,time_elps,(float)60*((float)wc / (float)time_elps), mistakes) ;
	return 0;
}
