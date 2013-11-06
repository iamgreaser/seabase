OBJDIR = build
SRCDIR = src

INCLUDES = src/common.h
OBJS = \
	$(OBJDIR)/file.o \
	$(OBJDIR)/map.o \
	$(OBJDIR)/sq.o \
	$(OBJDIR)/main.o

CFLAGS = -g -O2 -I/usr/include/squirrel `sdl-config --cflags`
LDFLAGS = -g
LIBS = -lm `sdl-config --libs` -lsquirrel -lsqstdlib

BINNAME = seabase

MKDIR = mkdir
RM = rm
MKDIR_P = mkdir -p

all: $(BINNAME)

clean:
	rm -f $(OBJS)

$(BINNAME): $(OBJS) $(OBJDIR)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR):
	$(MKDIR_P) $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

