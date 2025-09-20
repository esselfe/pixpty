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

static void *TermptyReader(void *arg) {
	(void)arg;
	char buf[8192];
	for (;;) {
		ssize_t n = read(pty_fd, buf, sizeof(buf));
		if (n > 0) {
			buf[n] = '\0';
			printf("TermptyReader(): '%s'\n", (char *)buf);
			TermbufWrite(&terminal_buffer, buf, (size_t)n);
			ScrollbackAddLine(buf, n, 0);

//			terminal_cursor_pos += n;
//			terminal_buffer_length += n;

			// Nudge SDL thread to redraw
			SDL_Event ev;
			memset(&ev, 0, sizeof(ev));
			ev.type = EV_PTY_DATA;
			ev.user.code = (int)n;
			SDL_PushEvent(&ev);
			continue;
		}
		if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			// Sleep a tick to yield CPU
			struct timespec ts = {.tv_sec=0, .tv_nsec=2*1000*1000}; // 2ms
			nanosleep(&ts, NULL);
			continue;
		}
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
		perror("forkpty");
		return -1;
	}
	if (child_pid == 0) {
		// Child: new session on slave PTY
		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);
		setenv("PS1", "\\t\\u@\\h:\\l:\\w\\$ ", 1);
		const char *sh = (shell_path && *shell_path) ? shell_path : "/bin/bash";
		execlp(sh, sh, "-i", NULL);
		exit(127);
	}

	// Parent: non-blocking
	int flags = fcntl(pty_fd, F_GETFL, 0);
	fcntl(pty_fd, F_SETFL, flags | O_NONBLOCK);

	// Start reader thread
	if (pthread_create(&reader_thr, NULL, TermptyReader, NULL) != 0) {
		perror("pthread_create");
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
			ScrollbackAddLine(terminal_buffer.buf, terminal_buffer_length, 0);
//			(void)write(pty_fd, data, nbytes);
//			memset(terminal_buffer.buf, 0, terminal_buffer.cap);
//			terminal_cursor_pos = 0;
//			terminal_buffer_length = 0;
//			return;
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

