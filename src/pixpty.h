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

// From terminal.c
#define TERMINAL_BUFFER_DEFAULT_SIZE 4096
extern unsigned int terminal_visible, terminal_blink;
extern char *terminal_buffer;
extern unsigned int terminal_buffer_length, terminal_buffer_size;
extern unsigned int terminal_cursor_pos, terminal_cursor_blink;
void TerminalInit(void);
void TerminalParse(void);
void TerminalRender(void);

#endif /* PIXPTY_H */
