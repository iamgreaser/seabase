OBJDIR = build
SRCDIR = .

INCLUDES = common.h
OBJS = \
	$(OBJDIR)/file.o \
	$(OBJDIR)/sq.o \
	$(OBJDIR)/main.o

CFLAGS = -I/usr/include/squirrel `sdl-config --cflags`
LDFLAGS = 
LIBS = -lm `sdl-config --libs` -lsquirrel

BINNAME = seabase

MKDIR = mkdir
RM = rm
MKDIR_P = mkdir -p

all: $(BINNAME)

clean:
	rm -f $(OBJS)

$(BINNAME): $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR):
	$(MKDIR_P) $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES) $(OBJDIR)
	$(CC) -c -o $@ $(CFLAGS) $<

