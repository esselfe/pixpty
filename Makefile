
CFLAGS = -g -DDEBUG=1 -std=c11 -Wall -D_DEFAULT_SOURCE -O2
LDFLAGS = -lSDL2 -lGL -lGLU -lpng16 -lutil -lpthread
OBJDIR = obj
OBJS = $(OBJDIR)/delta.o $(OBJDIR)/events.o $(OBJDIR)/font.o \
$(OBJDIR)/image.o $(OBJDIR)/render.o $(OBJDIR)/scrollback.o \
$(OBJDIR)/terminal.o $(OBJDIR)/termbuf.o $(OBJDIR)/termpty.o \
$(OBJDIR)/pixpty.o
PROGNAME = pixpty

.PHONY: default all prepare clean

default: all

all: prepare $(PROGNAME)
	@ls -l --color=auto $(PROGNAME) || true

prepare:
	@[ -d "$(OBJDIR)" ] || mkdir -v "$(OBJDIR)"

$(OBJDIR)/delta.o: src/delta.c
	gcc -c $(CFLAGS) src/delta.c -o $(OBJDIR)/delta.o

$(OBJDIR)/events.o: src/events.c
	gcc -c $(CFLAGS) src/events.c -o $(OBJDIR)/events.o

$(OBJDIR)/font.o: src/font.c
	gcc -c $(CFLAGS) src/font.c -o $(OBJDIR)/font.o

$(OBJDIR)/image.o: src/image.c
	gcc -c $(CFLAGS) src/image.c -o $(OBJDIR)/image.o

$(OBJDIR)/render.o: src/render.c
	gcc -c $(CFLAGS) src/render.c -o $(OBJDIR)/render.o

$(OBJDIR)/scrollback.o: src/scrollback.c
	gcc -c $(CFLAGS) src/scrollback.c -o $(OBJDIR)/scrollback.o

$(OBJDIR)/termbuf.o: src/termbuf.c
	gcc -c $(CFLAGS) src/termbuf.c -o $(OBJDIR)/termbuf.o

$(OBJDIR)/terminal.o: src/terminal.c
	gcc -c $(CFLAGS) src/terminal.c -o $(OBJDIR)/terminal.o

$(OBJDIR)/termpty.o: src/termpty.c
	gcc -c $(CFLAGS) src/termpty.c -o $(OBJDIR)/termpty.o

$(OBJDIR)/pixpty.o: src/pixpty.c
	gcc -c $(CFLAGS) src/pixpty.c -o $(OBJDIR)/pixpty.o

$(PROGNAME): $(OBJS)
	gcc $(OBJS) -o $(PROGNAME) $(LDFLAGS)

clean:
	@rm -rfv $(OBJDIR) $(PROGNAME) || true

