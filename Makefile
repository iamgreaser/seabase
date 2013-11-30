OBJDIR_ROOT = build
OBJDIR = build/posix
SRCDIR = src

INCLUDES = src/common.h
OBJS = \
	$(OBJDIR)/blit.o \
	$(OBJDIR)/file.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/lua.o \
	$(OBJDIR)/lua_draw.o \
	$(OBJDIR)/lua_fetch.o \
	$(OBJDIR)/lua_img.o \
	$(OBJDIR)/lua_input.o \
	$(OBJDIR)/lua_map.o \
	$(OBJDIR)/lua_turf.o \
	$(OBJDIR)/map.o \
	$(OBJDIR)/net.o \
	$(OBJDIR)/png.o \
	\
	$(OBJDIR)/main.o

CFLAGS = -g -O2 `sdl-config --cflags` -Wall -Wextra
LDFLAGS = -g
LIBS_SDL = `sdl-config --libs`
LIBS = -lm -llua-5.1 -lz -lenet $(LIBS_SDL)

BINNAME = seabase

MKDIR = mkdir
MKDIR_F = mkdir -p
RM = rm
RM_F = rm -f

all: $(BINNAME)

clean:
	$(RM_F) $(OBJS)

$(BINNAME): $(OBJDIR) $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR): $(OBJDIR_ROOT)
	$(MKDIR_F) $(OBJDIR)

$(OBJDIR_ROOT):
	$(MKDIR_F) $(OBJDIR_ROOT)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

