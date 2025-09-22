#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <pty.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#include "pixpty.h"

static int pty_fd = -1;
static pid_t child_pid = -1;
static pthread_t reader_thr;
Uint32 EV_PTY_DATA; // custom SDL event to wake render

// Compute terminal cell size from glyphs (8x16 here)
static inline void TermptyCalcRowsCols(int px_w, int px_h, int *cols, int *rows) {
	int cw = 8, ch = 16;
	terminal_cols = *cols = (px_w  > 0) ? (px_w / cw)  : 80;
	terminal_rows = *rows = (px_h  > 0) ? (px_h / ch)  : 25;
}

static void TermptySetWindowSizeFromPixels(int px_w, int px_h) {
	if (pty_fd < 0) return;
	int cols, rows;
	TermptyCalcRowsCols(px_w, px_h, &cols, &rows);
	struct winsize ws = {
		.ws_row = rows, .ws_col = cols,
		.ws_xpixel = px_w, .ws_ypixel = px_h
	};
	ioctl(pty_fd, TIOCSWINSZ, &ws);
}

static void TermptyStripEscapes(char *in, char *out, unsigned long max_length, unsigned long *out_length) {
	char *c = in;
	unsigned long cnt_in = 0, cnt_out = 0;
	while (1) {
		if (*c == '\0' || cnt_in >= max_length) {
			out[cnt_out] = '\0';
			break;
		}
		else if (*c == '\033' && *(c+1) == '[') {
			cnt_in += 2;
			c += 2;
			char *c2 = c;
			char *c3 = c;
			// ignore \033[?2004l\r\n
			if (*c2 == '?' && *(++c2) == '2' &&
			  *(++c2) == '0' && *(++c2) == '0' && *(++c2) == '4' &&
			  *(++c2) == 'l' && *(++c2) == '\r' && *(++c2) == '\n') {
		  		cnt_in += 8;
		  		c = c2 + 1;
				continue;
			}
			// ignore \033[?2004h
			else if (*c3 == '?' && *(++c3) == '2' &&
			  *(++c3) == '0' && *(++c3) == '0' && *(++c3) == '4' && *(++c3) == 'h') {
				cnt_in += 6;
				c = c3 + 1;
				continue;
			}
			else {
				while (1) {
					if (isdigit(*c) || *c == ';') {
						++cnt_in;
						++c;
						continue;
					}
					else if (*c == 'm') {
						++cnt_in;
						++c;
						break;
					}
					else if (*c == '\0')
						break;
					else {
						++cnt_in;
						++c;
					}
				}
				
				if (*c == '\0') {
					out[cnt_out] = '\0';
					break;
				}
				else
					continue;
			}
		}
		else {
			out[cnt_out] = *c;
			++c;
			++cnt_in;
			++cnt_out;
		}
	}
	
	if (out_length != NULL)
		*out_length = cnt_out;
}

static void *TermptyReader(void *arg) {
	char buf[8192];
	char buf_no_escapes[8192];
	unsigned long buf_no_escapes_length;
	for (;;) {
		memset(buf, 0, 8192);
		memset(buf_no_escapes, 0, 8192);
		buf_no_escapes_length = 0;
		long n = read(pty_fd, buf, 8191);
		if (n > 0) {
			TermptyStripEscapes(buf, buf_no_escapes, (unsigned long)n, &buf_no_escapes_length);
			printf("TermptyReader(): '%s'\n", buf_no_escapes);
			
			/* printf("###>");
			char *c = buf;
			while (*c != '\0') {
				printf("#%d:%c:", (int)*c, (char)*c);
				++c;
			}
			printf("<###\n"); */
			
			ScrollbackAddLine(buf_no_escapes, buf_no_escapes_length, 0);
			
			char *buf_last_line = TermbufOnlyKeepLastLine(buf_no_escapes);
			TermbufWrite(&terminal_buffer, buf_last_line, strlen(buf_last_line));
			
			terminal_cursor_pos = buf_no_escapes_length;
			terminal_buffer_length = buf_no_escapes_length;

			// Nudge SDL thread to redraw
			SDL_Event ev;
			memset(&ev, 0, sizeof(ev));
			ev.type = EV_PTY_DATA;
			ev.user.code = (int)buf_no_escapes_length;
			SDL_PushEvent(&ev);
			
			continue;
		}
		else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			// Sleep a tick to yield CPU
			struct timespec ts = {.tv_sec=0, .tv_nsec=2*1000*1000}; // 2ms
			nanosleep(&ts, NULL);
			continue;
		}
		else if (n <= 0)
			continue;

		// n == 0 (EOF) or fatal error => child likely exited
		break;
	}
	return NULL;
}

int TerminalSpawnShell(const char *shell_path) {
	// Register SDL user event once
	static int ev_regd = 0;
	if (!ev_regd) {
		EV_PTY_DATA = SDL_RegisterEvents(1);
		ev_regd = 1;
	}

	// Optional: tweak termios as needed
	struct termios tio;
	tcgetattr(STDIN_FILENO, &tio); // base off current
	cfmakeraw(&tio);               // raw mode for PTY; bash will set its own

	// Set initial size
	int px_w = winW, px_h = winH;
	int cols, rows;
	TermptyCalcRowsCols(px_w, px_h, &cols, &rows);
	struct winsize ws = {.ws_row=rows, .ws_col=cols, .ws_xpixel=px_w, .ws_ypixel=px_h};

	child_pid = forkpty(&pty_fd, NULL, &tio, &ws);
	if (child_pid < 0) {
		printf("pixpty::TerminalSpawnShell() error: forkpty() failed: %s\n",
			strerror(errno));
		return -1;
	}
	if (child_pid == 0) {
		// Child: new session on slave PTY
		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);
		const char *sh = (shell_path && *shell_path) ? shell_path : "/bin/bash";
		execlp(sh, sh, "-i", NULL);
		exit(127);
	}

	// Parent: non-blocking
	int flags = fcntl(pty_fd, F_GETFL, 0);
	fcntl(pty_fd, F_SETFL, flags | O_NONBLOCK);

	// Start reader thread
	int ret = pthread_create(&reader_thr, NULL, TermptyReader, NULL);
	if (ret != 0) {
		printf("pixpty::TerminalSpawnShell() error: pthread_create() failed: %s\n",
			strerror(ret));
		close(pty_fd);
		pty_fd = -1;
		return -1;
	}
	pthread_detach(reader_thr);

	return 0;
}

void TerminalSendInput(const void *data, size_t nbytes) {
	if (pty_fd >= 0 && nbytes) {
		//printf("TerminalSendInput(): '%s'\n", (char *)data);
		
		char *c = (char *)data;
		if (*c == '\n') {
			pthread_mutex_lock(&terminal_buffer.mu);
			ScrollbackAddLine(terminal_buffer.buf, terminal_buffer_length, 0);
			memset(terminal_buffer.buf, 0, terminal_buffer.cap);
			terminal_buffer.r = 0;
			terminal_buffer.w = 0;
			pthread_mutex_unlock(&terminal_buffer.mu);
			terminal_cursor_pos = 0;
			terminal_buffer_length = 0;
		}
		
		// Best effort; ignore EAGAIN (kernel ring full)
		(void)write(pty_fd, data, nbytes);
	}
}

void TerminalOnResize(int px_w, int px_h) {
	TermptySetWindowSizeFromPixels(px_w, px_h);
}

int TerminalChildIsAlive(void) {
	if (child_pid <= 0) return 0;
	int status = 0;
	pid_t r = waitpid(child_pid, &status, WNOHANG);
	if (r == 0) return 1; // still running
	else return 0;        // reaped or error
}

void TerminalShutdown(void) {
	if (child_pid > 0) {
		kill(child_pid, SIGHUP);
		kill(child_pid, SIGTERM);
		// Optionally wait/reap
		waitpid(child_pid, NULL, 0);
		child_pid = -1;
	}
	if (pty_fd >= 0) {
		close(pty_fd);
		pty_fd = -1;
	}
}

