#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <GL/gl.h>

#include "pixpty.h"

struct Scrollback scrollback;

void ScrollbackInit(void) {

}

void ScrollbackAddLine(char *text, unsigned int length, int is_wrapped) {
	struct Line *line = malloc(sizeof(struct Line));
	if (line == NULL) {
		printf("pixpty::ScrollbackAddLine() error: malloc() returned NULL, exiting.\n");
		exit(ENOMEM);
	}
	
	if (scrollback.first_line == NULL && scrollback.last_line == NULL) {
		line->prev = NULL;
		line->rank = 1;
		scrollback.first_line = line;
	}
	else {
		line->prev = scrollback.last_line;
		scrollback.last_line->next = line;
		line->rank = scrollback.last_line->rank + 1;
	}
	line->next = NULL;
	unsigned int line_size = (length <= terminal_cols) ? length : terminal_cols;
	line->text = malloc(line_size + 1);
	memset(line->text, 0, line_size + 1);
	
	scrollback.last_line = line;
	++scrollback.total_lines;
		
	if (is_wrapped || length > terminal_cols) {
		line->is_wrapped = 1;
		int cnt;
		char *c;
		for (c = text, cnt = 0; cnt < length; c++, cnt++) {
			if (*c == '\0') break;
			
			if (*c == '\n') {
				++c;
				ScrollbackAddLine(c, strlen(c), 1);
				break;
			}
			else
				line->text[cnt] = *c;
		}
	}
	else {
		sprintf(line->text, "%s", text);
	}
}

void ScrollbackClear(void) {

}

