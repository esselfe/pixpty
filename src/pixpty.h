#ifndef PIXPTY_H
#define PIXPTY_H 1

#include <GL/gl.h>
#include <SDL2/SDL.h>

extern char *pixpty_version_string;
extern int mainloopend;
extern int winX, winY, winW, winH;
extern SDL_Window *window;

// From delta.c
extern GLfloat delta;
void DeltaInit(void);
void DeltaUpdate(void);

// From events.c
#define MOD_NONE    0
#define MOD_CTRL    1
#define MOD_ALT     (1<<1)
#define MOD_SHIFT   (1<<2)
extern unsigned int mods;
extern int mouse_x, mouse_y, mouse_x_prev, mouse_y_prev;
extern unsigned int mouse_button, mouse_held;
void EventsInit(void);
void EventsCheck(void);

// From font.c
#define BG_NONE            0
#define BG_BLACK           1
#define BG_GRAY            2
#define BG_GREY            2
void FontInit(void);
void FontRender(int bgcolor, int x, int y, char *text);

// From image.c
GLubyte *ImageFromPNGFile(unsigned int width, unsigned int height, char *filename);

// From render.c
extern unsigned int fps;
extern char *fps_text;
void RenderInit(void);
void Render(void);
void RenderResize(int width, int height);

// From scrollback.c
struct Line {
	struct Line *prev, *next;
	unsigned long rank;
	unsigned int is_wrapped;
	unsigned long length;
	char *text;
};
struct Scrollback {
	struct Line *first_line, *last_line;
	unsigned long total_bytes;
	unsigned long total_lines;
};
extern struct Scrollback scrollback;
void ScrollbackInit(void);
void ScrollbackAddLine(char *line, unsigned int length, int is_wrapped);
void ScrollbackClear(void);

// From termbuf.c
#define TERMINAL_BUFFER_DEFAULT_SIZE 8192
typedef struct {
    char *buf;
    unsigned long cap;       // capacity
    unsigned long r, w;                // read, write indices (mod cap)
    pthread_mutex_t mu;
} termbuf_t;
extern termbuf_t terminal_buffer;
void TermbufInit(termbuf_t *tb, unsigned long cap);
void TermbufFree(termbuf_t *tb);
char *TermbufOnlyKeepLastLine(char *buffer);
void TermbufReset(termbuf_t *tb);
void TermbufWrite(termbuf_t *tb, const char *src, unsigned long n);

// From termpty.c
extern Uint32 EV_PTY_DATA;
int TerminalSpawnShell(const char *shell_path);
void TerminalSendInput(const void *data, size_t nbytes);
void TerminalOnResize(int px_w, int px_h);
int TerminalChildIsAlive(void);
void TerminalShutdown(void);

// From terminal.c
extern unsigned long terminal_buffer_length, terminal_buffer_size;
extern unsigned int terminal_cursor_pos, terminal_cursor_blink;
extern unsigned int terminal_rows, terminal_cols;
void TerminalInit(void);
void TerminalParse(void);
void TerminalRender(void);

#endif /* PIXPTY_H */
