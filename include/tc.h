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
	uint	words;
	uint	current_word;
	uint	mistakes_total;
	ulong	start_time;
	ulong	timer;
	WINDOW	*tpwin;
	WINDOW	*rtwin;
	control	curcon;
}
rt_info;

typedef struct buffers
{
	char	**pre;
	char	*post;
	WINDOW	**kbwin;
	uint	*mistake_pos;
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

enum dict
{
	d_undefined,
	en
};

typedef struct settings
{
	gamemode	gm;
	enum		dict dic;
	bool		stall;
	bool		stay;
}
set;

extern void generate_giberish(rt_info *rti, buffs *bfs, set *s, uint len);

extern WINDOW *create_newwin(int h, int w, int sy, int sx);
extern void update_win(WINDOW* local_win, bool pressed, bool hi);
extern void init_keyboard(buffs *bfs, int len, int hei);
extern void update_keyboard(buffs *bfs, int len, int hei, bool hi, char in, char last);

extern void alloc_buffers(buffs *bfs);
extern void free_bufs(buffs *bfs);
extern void shift_buffers(buffs *bfs);
