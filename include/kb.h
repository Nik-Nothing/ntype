extern WINDOW *create_newwin(int h, int w, int sy, int sx);
extern void update_win(WINDOW* local_win, bool pressed, bool hi);
extern void init_keyboard(buffs *bfs, int len, int hei);
extern void update_keyboard(buffs *bfs, int len, int hei, bool hi, char in, char last);
