#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include<curses.h>

#include"dict.h"

enum control{write,backspace,eow};

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

void generate_giberish(char** buf,int len) {
	int ran;
	ran = (rand() % 1000);
	strcpy(buf[len],en_dict[ran]);
	//strcat(buf[len],"\0");
}

int check_word(char** buf_pre,char** buf_post,int wp) {
	int x=-1;
	while(++x < strlen(buf_pre[wp]))
		if(buf_pre[wp][x] != buf_post[wp][x])
			return 1;
	return 0;
}

int main() {
	int started = 0,start_time,wc = 50, y = 0, z,w,g,f, length, mistakes = 0, cor = 0, *mistake_pos, time_elps, hide_lines;
	size_t pre_malloc = 0,post_malloc = 0,mis_malloc = 0,mistake_pos_malloc = 0;
	char c,*mis;
	enum control curcon = write;
	WINDOW *tpwin;
	
	initscr();
	curs_set(0);
	cbreak();
	noecho();
	start_color();
	srand(time(NULL));
	
	init_color(COLOR_CYAN,350,350,350);
	init_pair(1,COLOR_BLACK,COLOR_GREEN);
	init_pair(2,COLOR_BLACK,COLOR_RED);
	init_pair(3,COLOR_CYAN,COLOR_BLACK);
	
	/*pre = malloc(sizeof(char) * 1000);
	post = malloc(sizeof(char) * 1000);*/
	mis = malloc(sizeof(char) * 1000);
	mistake_pos = malloc(sizeof(int) * 1000);

		int wln=0;
	// arr of arr
	char** words_pre = malloc(sizeof(char*) * 100);
	char** words_post = malloc(sizeof(char*) * 100);
	
	int cts=-1;
	while(++cts < 100){
		words_pre[cts] = malloc(sizeof(char) * 100);
		words_post[cts] = malloc(sizeof(char) * 100);
	}

	int words=-1,current_word = 0,current_word_position = 0;
	while(++words < 100) {
		generate_giberish(words_pre,words);
	}
	
	while(1) {
		refresh();
		create_newwin(LINES-(LINES/14)*4,COLS,0,0,0,0);
		tpwin = newwin((LINES-(LINES/14)*4)-4,COLS-6,2,3);
		wattron(tpwin,COLOR_PAIR(3));
		//mvwaddstr(tpwin,y,0,pre);
		int yw=0;
		int wcc=-1,wcc_off=0;
		y=0;
		while(++wcc < words) {
				if((wcc_off + strlen(words_pre[wcc]))> COLS-6){
					++y;
					yw = y-wln;
					wcc_off = 0;
				}
				if(y >= wln)
					mvwaddstr(tpwin,yw,wcc_off,words_pre[wcc]);
				wcc_off += (strlen(words_pre[wcc])+1);
		}
		wattroff(tpwin,COLOR_PAIR(3));
		y=0,wcc=-1,wcc_off=0;
		while(++wcc < current_word+1) {
			if((wcc_off + strlen(words_pre[wcc]))> COLS-6){
				++y;
				if(y>wln)
					++wln;
				wcc_off = 0;
			}
			if(y >= wln){
				if(wcc < current_word)
					wattron(tpwin,COLOR_PAIR(1));
				mvwaddstr(tpwin,0,wcc_off,words_post[wcc]);
				if(wcc < current_word)
					wattroff(tpwin,COLOR_PAIR(1));
				
				if(wcc == current_word) {
					z = -1;
					while(++z < mistakes) {
						wattron(tpwin,COLOR_PAIR(2));
						mvwaddch(tpwin,0,wcc_off + mistake_pos[z],mis[z]);
						wattroff(tpwin,COLOR_PAIR(2));
					}
				}
			}
				wcc_off += (strlen(words_pre[wcc]));
			if(y >= wln)
				mvwaddch(tpwin,y,wcc_off,' ');
				wcc_off += 1;
		}
		wrefresh(tpwin);
		refresh();
		create_keyboard(COLS,LINES,c,cor);
		cor = 0;
		if(current_word == words)
				break;
		c = getchar();	
		if(started == 0) {
			start_time = (unsigned long)time(NULL);
			started = 1;
		}
		if(curcon != eow)
			curcon = write;
		switch(c) {
			case 10: /* Linebreak */
				c = ' ';
				break;
			case 11: /* Tabulator */
				c = ' ';
				break;
			case 27: /* Escape */
				endwin();
				return 0;
			case 127: /* Backspace */
				if(curcon != eow) {
					if(current_word_position > 0)
						--current_word_position;
					words_post[current_word][current_word_position] = '\0';
					c = '\0';
					curcon = backspace;
					break;
				}
			default:
			if(curcon != eow)
				words_post[current_word][current_word_position] = c;
		}
		if(curcon == backspace){
			if(mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
					--mistakes;
		} else if(curcon == write){
			if(current_word_position == strlen(words_pre[current_word])-1 && check_word(words_pre,words_post,current_word) == 0) {
					current_word_position = 0;
					++current_word;
					curcon = eow;
			}
			if(curcon != eow){
				if(c != words_pre[current_word][current_word_position]) {
					cor = 1;
					mistake_pos[mistakes] = current_word_position;
					mis[mistakes] = c;
					++mistakes;
				} else {
					if(mistakes > 0 && mistake_pos[mistakes-1] == current_word_position)
						--mistakes;
					cor = 0;
				}
				if(current_word_position < strlen(words_pre[current_word]))
					++current_word_position;
			}
		} else if(curcon == eow && c == ' ')
			curcon = write;
	}
	endwin();
	time_elps = (unsigned long)time(NULL) - start_time;
	printf("%d Words in %d Seconds! that is %.2f WPM, you made %d Mistakes\n",wc,time_elps,(float)60*((float)wc / (float)time_elps), mistakes) ;
	return 0;
}
