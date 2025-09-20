
CFLAGS = -std=c11 -Wall -D_DEFAULT_SOURCE -O2
LDFLAGS = -lSDL2 -lGL -lGLU
OBJDIR = obj
OBJS = $(OBJDIR)/delta.o $(OBJDIR)/events.o $(OBJDIR)/render.o \
$(OBJDIR)/terminal.o $(OBJDIR)/pixpty.o
PROGNAME = pixpty

.PHONY: default all prepare clean

default: all

all: prepare $(PROGNAME)
	@ls -l --color=auto $(LIBNAME) $(PROGNAME) || true

prepare:
	@[ -d "$(OBJDIR)" ] || mkdir -v "$(OBJDIR)"

$(OBJDIR)/delta.o: src/delta.c
	gcc -c $(CFLAGS) src/delta.c -o $(OBJDIR)/delta.o

$(OBJDIR)/events.o: src/events.c
	gcc -c $(CFLAGS) src/events.c -o $(OBJDIR)/events.o

$(OBJDIR)/render.o: src/render.c
	gcc -c $(CFLAGS) src/render.c -o $(OBJDIR)/render.o

$(OBJDIR)/terminal.o: src/terminal.c
	gcc -c $(CFLAGS) src/terminal.c -o $(OBJDIR)/terminal.o

$(OBJDIR)/pixpty.o: src/pixpty.c
	gcc -c $(CFLAGS) src/pixpty.c -o $(OBJDIR)/pixpty.o

$(PROGNAME): $(OBJS)
	gcc $(OBJS) -o $(PROGNAME) $(LDFLAGS)

clean:
	@rm -rfv $(OBJDIR) $(PROGNAME) || true

