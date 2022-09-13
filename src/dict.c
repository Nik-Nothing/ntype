#include <stdlib.h>
#include <curses.h>
#include <string.h>

#include "macros.h"
#include "tc.h"
#include "dict.h"

void generate_giberish(rt_info *rti, buffs *bfs, set *s, uint len)
{
	int ran;
	
	if (s->dic == en)
	{
		ran = (rand() % 1000);
		strcpy(bfs->pre[len], en_dict[ran]);
	} 
	elif (s->dic == unix)
	{
		ran = (rand() % 107);
		strcpy(bfs->pre[len], unix_dict[ran]);
	}
}
