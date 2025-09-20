#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>

#include "pixpty.h"

GLfloat delta;
static time_t delta_time_now, delta_time_previous;
static struct timeval delta_tv_blink_now, delta_tv_blink_previous;

void DeltaInit(void) {
	gettimeofday(&delta_tv_blink_previous, NULL);
	delta_time_previous = (time_t)delta_tv_blink_previous.tv_sec;
}

void DeltaUpdate(void) {
	gettimeofday(&delta_tv_blink_now, NULL);

	delta += 0.1;
	if (delta >= 360.0)
		delta -= 360.0;
	
	// Blink the terminal cursor once every half second
	struct timeval delta_tv_diff;
	timersub(&delta_tv_blink_now, &delta_tv_blink_previous, &delta_tv_diff);
	if (delta_tv_diff.tv_usec > 500000 || delta_tv_diff.tv_sec > 0) {
		delta_tv_blink_previous.tv_sec = delta_tv_blink_now.tv_sec;
		delta_tv_blink_previous.tv_usec = delta_tv_blink_now.tv_usec;
		
		terminal_cursor_blink = !terminal_cursor_blink;
	}

	// Once a second
	delta_time_now = (time_t)delta_tv_blink_now.tv_sec;
	if (delta_time_now > delta_time_previous) {
		delta_time_previous = delta_time_now;

		sprintf(fps_text, "%u fps", fps);
		fps = 0;
	}
}

