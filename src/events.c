#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "pixpty.h"

unsigned int mods;
int mouse_x, mouse_y, mouse_x_prev, mouse_y_prev;
unsigned int mouse_button, mouse_held;
static SDL_Event event;

void EventsInit(void) {
	mouse_x_prev = mouse_x = (int)winW/2;
	mouse_y_prev = mouse_y = (int)winH/2;
	SDL_WarpMouseInWindow(window, mouse_x, mouse_y);
	SDL_ShowCursor(1);
}

static char ShiftKey(SDL_Keycode key) {
	switch (key) {
	case SDLK_BACKQUOTE:
		return '~';
	case SDLK_1:
		return '!';
	case SDLK_2:
		return '@';
	case SDLK_3:
		return '#';
	case SDLK_4:
		return '$';
	case SDLK_5:
		return '%';
	case SDLK_6:
		return '^';
	case SDLK_7:
		return '&';
	case SDLK_8:
		return '*';
	case SDLK_9:
		return '(';
	case SDLK_0:
		return ')';
	case SDLK_MINUS:
		return '_';
	case SDLK_EQUALS:
		return '+';
	case SDLK_LEFTBRACKET:
		return '{';
	case SDLK_RIGHTBRACKET:
		return '}';
	case SDLK_BACKSLASH:
		return '|';
	case SDLK_SEMICOLON:
		return ':';
	case SDLK_QUOTE:
		return '"';
	case SDLK_COMMA:
		return '<';
	case SDLK_PERIOD:
		return '>';
	case SDLK_SLASH:
		return '?';
	default:
		break;
	}
	
	if (key >= 'a' && key <= 'z')
		return (char)key-32;
	
	return 32;
}

void EventsCheck(void) {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			mainloopend = 1;
		else if (event.type == SDL_KEYDOWN && event.key.repeat == 1) {
			if (terminal_visible && event.key.keysym.sym == SDLK_BACKSPACE) {
				if (terminal_cursor_pos > 0) {
					terminal_buffer[--terminal_cursor_pos] = '\0';
					--terminal_buffer_length;
				}
				return;
			}
		}
		else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
			if (terminal_visible && (event.key.keysym.sym >= SDLK_SPACE && 
				event.key.keysym.sym <= SDLK_z)) {
				if (mods & MOD_CTRL && event.key.keysym.sym == SDLK_c) {
					terminal_cursor_pos = 0;
					memset(terminal_buffer, 0, terminal_buffer_size);
					terminal_buffer_length = 0;
				}
				else if (terminal_cursor_pos < terminal_buffer_size) {
					if (mods & MOD_SHIFT)
						terminal_buffer[terminal_cursor_pos++] = ShiftKey(event.key.keysym.sym);
					else
						terminal_buffer[terminal_cursor_pos++] = event.key.keysym.sym;

					++terminal_buffer_length;
				}	
				return;
			}
			else if (terminal_visible && event.key.keysym.sym == SDLK_BACKSPACE) {
				if (terminal_cursor_pos > 0) {
					terminal_buffer[--terminal_cursor_pos] = '\0';
					--terminal_buffer_length;
				}
				return;
			}
			else if (terminal_visible && event.key.keysym.sym == SDLK_RETURN) {
				TerminalParse();
				return;
			}
			
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				mainloopend = 1;
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				mods |= MOD_CTRL;
				break;
			case SDLK_LALT:
			case SDLK_RALT:
				mods |= MOD_ALT;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				mods |= MOD_SHIFT;
				break;
			case SDLK_TAB:
				terminal_visible = !terminal_visible;
				break;
			case SDLK_UP:
				break;
			case SDLK_DOWN:
				break;
			case SDLK_LEFT:
				break;
			case SDLK_RIGHT:
				break;
			case SDLK_PAGEUP:
				break;
			case SDLK_PAGEDOWN:
				break;
			case SDLK_KP_MINUS:
				break;
			case SDLK_KP_PLUS:
				break;
			case SDLK_F1:
				break;
			case SDLK_F2:
				break;
			default:
				break;
			}
		}
		else if (event.type == SDL_KEYUP) {
			switch(event.key.keysym.sym) {
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				mods ^= MOD_CTRL;
				break;
			case SDLK_LALT:
			case SDLK_RALT:
				mods ^= MOD_ALT;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				mods ^= MOD_SHIFT;
				break;
			default:
				break;
			}
		}
	}
}

