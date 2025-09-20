#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "pixpty.h"

char *pixpty_version_string = "0.0.2";
char *window_title;
unsigned int window_title_len = 1024;
int winX = 100, winY = 50, winW = 800, winH = 600;
SDL_Window *window;
SDL_GLContext context;
int mainloopend;

int main(int argc, char **argv) {
	printf("pixpty started.\n");

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("pixpty error: SDL_Init() failed.\n");
		exit(1);
	}

	SDL_StartTextInput();

	window_title = malloc(window_title_len + 1);
	if (window_title == NULL) {
		printf("pixpty error: malloc() returned NULL, exiting.\n");
		exit(1);
	}
	memset(window_title, 0, window_title_len + 1);

	// Set OpenGL API version to 3.2 and show what we got
	int gl_major, gl_minor;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor);
	printf("Using OpenGL %d.%d.\n", gl_major, gl_minor);

	sprintf(window_title, "pixpty %s", pixpty_version_string);
	window = SDL_CreateWindow(window_title, winX, winY, winW, winH,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		printf("pixpty error: Cannot create SDL2 GL context. SDL error: %s.\n",
			SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	// Needs an OpenGL context initialized
	printf("OpenGL %s available.\n", glGetString(GL_VERSION));

	FontInit();
	EventsInit();
	DeltaInit();
	RenderInit();
	TermbufInit(&terminal_buffer, TERMINAL_BUFFER_DEFAULT_SIZE);
	TerminalInit();

	while (!mainloopend) {
		EventsCheck();
		DeltaUpdate();
		Render();
	}
	
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}

