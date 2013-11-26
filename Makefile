OBJDIR = build
SRCDIR = src

INCLUDES = src/common.h
OBJS = \
	$(OBJDIR)/blit.o \
	$(OBJDIR)/file.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/lua.o \
	$(OBJDIR)/lua_img.o \
	$(OBJDIR)/lua_map.o \
	$(OBJDIR)/lua_turf.o \
	$(OBJDIR)/map.o \
	$(OBJDIR)/png.o \
	\
	$(OBJDIR)/main.o

CFLAGS = -g -O2 `sdl-config --cflags` -Wall -Wextra
LDFLAGS = -g
LIBS = -lm `sdl-config --libs` -llua-5.1 -lz

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

