#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <GL/gl.h>

#include "pixpty.h"

unsigned long terminal_buffer_length, terminal_buffer_size;
unsigned int terminal_cursor_pos, terminal_cursor_blink;
unsigned int terminal_rows, terminal_cols;

void TerminalInit(void) {
	if (terminal_buffer_size == 0)
		terminal_buffer_size = TERMINAL_BUFFER_DEFAULT_SIZE;
	
	terminal_buffer_length = 0;
	terminal_cursor_blink = 1;

	TerminalSpawnShell("/bin/bash");
}

// TODO: The terminal buffer contains both prompt and input command, which needs to be
// isolated for this function
// Meant for builtin-like parsing of commands
void TerminalParse(void) {
	return;
/*
	char w1[128], w2[128], w3[128], w4[128];
	memset(w1, 0, 128);
	memset(w2, 0, 128);
	memset(w3, 0, 128);
	memset(w4, 0, 128);
	unsigned int current_word = 0, wcnt = 0, bcnt = 0;
	char *c = terminal_buffer;
	while (1) {
		if (current_word == 0) {
			if (*c != ' ')
				w1[wcnt++] = terminal_buffer[bcnt++];
			else {
				++current_word;
				wcnt = 0;
				++bcnt;
			}
			if (bcnt >= terminal_buffer_size)
				break;
			else if (wcnt >= 128)
				break;
		}
		else if (current_word == 1) {
			if (*c != ' ')
				w2[wcnt++] = terminal_buffer[bcnt++];
			else {
				++current_word;
				wcnt = 0;
				++bcnt;
			}
			if (bcnt >= terminal_buffer_size)
				break;
			else if (wcnt >= 128)
				break;
		}
		else if (current_word == 2) {
			if (*c != ' ')
				w3[wcnt++] = terminal_buffer[bcnt++];
			else {
				++current_word;
				wcnt = 0;
				++bcnt;
			}
			if (bcnt >= terminal_buffer_size)
				break;
			else if (wcnt >= 128)
				break;
		}
		else if (current_word == 3) {
			if (*c != ' ')
				w4[wcnt++] = terminal_buffer[bcnt++];
			else
				break;
			
			
			if (bcnt >= terminal_buffer_size)
				break;
			else if (wcnt >= 128)
				break;
		}

		++c;
		if (*c == '\0')
			break;
	}

	if (strlen(w1)) {
		if (strcmp(w1, "exit") == 0 || strcmp(w1, "quit") == 0 ||
			strcmp(w1, "qw") == 0)
			mainloopend = 1;
	}
*/
}

void TerminalRender(void) {
	// Prompt background
	glColor4f(0.05, 0.1, 0.15, 1.0);
	glBegin(GL_QUADS);
	glVertex3i(0, 0, 0);
	glVertex3i(0, 20, 0);
	glVertex3i((int)winW, 20, 0);
	glVertex3i((int)winW, 0, 0);
	glEnd();

	if (terminal_cursor_blink) {
		glColor4f(0.3, 0.4, 0.5, 1.0);
		glBegin(GL_LINES);
		glVertex3i(terminal_cursor_pos * 8 + 1, 2, 1);
		glVertex3i(terminal_cursor_pos * 8 + 1, 18, 1);
		glEnd();
	}

	if (strlen(terminal_buffer.buf))
		FontRender(BG_GREY, 2, 2, terminal_buffer.buf);
	
	struct Line *line = scrollback.last_line;
	while (line != NULL) {
		glPushMatrix();
		glTranslatef(0.0, 20.0 * (scrollback.total_lines - line->rank), 0.0);
		FontRender(BG_BLACK, 2, 2, line->text);	
		glPopMatrix();
		
		if ((scrollback.total_lines - line->rank) * 20.0 >= winH)
			break;
		else
			line = line->prev;
	}
}

