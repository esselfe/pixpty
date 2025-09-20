#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>

#include "pixpty.h"

unsigned int fps; // For measuring frames per second
char *fps_text; // Text rendered on the HUD

void RenderInit(void) {
	fps_text = malloc(120);
	sprintf(fps_text, "0 fps");
	
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glViewport((GLint)0, (GLint)0, (GLsizei)winW, (GLsizei)winH);
}
/*
static void RenderSet3DView(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(-1.0, 1.0, 1.0);
	gluPerspective(60.0, winW/winH, 0.01, 1500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 1.0, -10.0,
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, -1.0);
}
*/
static void RenderSet2DView(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0, winW, 0.0, winH, 0.0, 10.0);
	gluOrtho2D(0.0, winW, 0.0, winH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderSet2DView();

	FontRender(BG_NONE, winW - (8 * strlen(fps_text)) - 8, winH - 24, fps_text);

	TerminalRender();

	SDL_GL_SwapWindow(window);
	++fps;
}

void RenderResize(int width, int height) {
	winW = width;
	winH = height;
	glViewport((GLint)0, (GLint)0, (GLsizei)winW, (GLsizei)winH);
	RenderSet2DView();
}

