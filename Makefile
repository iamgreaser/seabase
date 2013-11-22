OBJDIR = build
SRCDIR = src

INCLUDES = src/common.h
OBJS = \
	$(OBJDIR)/file.o \
	$(OBJDIR)/map.o \
	$(OBJDIR)/lua.o \
	$(OBJDIR)/lua_turf.o \
	$(OBJDIR)/main.o

CFLAGS = -g -O2 `sdl-config --cflags`
LDFLAGS = -g
LIBS = -lm `sdl-config --libs` -llua-5.1

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

