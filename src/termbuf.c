#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "pixpty.h"

termbuf_t terminal_buffer = {0};

void TermbufInit(termbuf_t *tb, unsigned long cap) {
	tb->buf = malloc(cap);
	if (tb->buf == NULL) {
		printf("pixpty::TermbufInit() error: malloc() returned NULL, exiting.\n");
		exit(ENOMEM);
	}
	tb->cap = cap;
	memset(tb->buf, 0, tb->cap);
	tb->r = tb->w = 0;
	pthread_mutex_init(&tb->mu, NULL);
}

void TermbufFree(termbuf_t *tb) {
	if (!tb) return;
	
	free(tb->buf);
	tb->buf = NULL;
	tb->cap = tb->r = tb->w = 0;
	pthread_mutex_destroy(&tb->mu);
}

void TermbufReset(termbuf_t *tb) {
	pthread_mutex_lock(&tb->mu);
	memset(tb->buf, 0, tb->cap);
	tb->r = tb->w = 0;
	pthread_mutex_unlock(&tb->mu);
}

static inline int TermbufRingFree(termbuf_t *t) {
	return (t->r + t->cap - t->w - 1) % t->cap;
}

void TermbufWrite(termbuf_t *t, const char *src, unsigned long n) {
	pthread_mutex_lock(&t->mu);
	if (n >= t->cap) { // Keep the most recent (cap-1)
		src += (n - (t->cap - 1));
		n = t->cap - 1;
		t->r = 0;
		t->w = 0;
	}
	
	// Drop oldest if necessary
	while (TermbufRingFree(t) < n) {
		t->r = (t->r + 1) % t->cap;
	}
	
	// Write in up to two chunks
	unsigned long first = (t->cap - t->w);
	if (first > n) first = n;
	memcpy(t->buf + t->w, src, first);
	memcpy(t->buf, src + first, n - first);
	t->w = (t->w + n) % t->cap;
	pthread_mutex_unlock(&t->mu);
}

