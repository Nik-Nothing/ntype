#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "tc.h"

void alloc_buffers(buffs *bfs)
{
	uint ctl = 0;

	bfs->mistake_pos = malloc(sizeof(int) * 20);
	bfs->pre = malloc(sizeof(char *) * 50);
	bfs->post = malloc(sizeof(char) * 25);
	bfs->kbwin = malloc(sizeof(WINDOW *) * 30);
	do
		bfs->pre[ctl] = malloc(sizeof(char) * 25);
	while (++ctl < 50);
}

void free_bufs(buffs *bfs)
{
	uint ctl = 0;
	
	do
		free(bfs->pre[ctl]);
	while (++ctl < 50);
	free(bfs->pre);
	free(bfs->post);
	free(bfs->mistake_pos);
}

void shift_buffers(buffs *bfs)
{
	uint ctl = 0;
	
	free(bfs->pre[0]);
	memmove(bfs->pre, bfs->pre + 1, sizeof(bfs->pre[0]) * 49);
	bfs->pre[49] = malloc(sizeof(char *));
	
	do
		bfs->post[ctl] = '\0';
	while (++ctl < 25);
}
