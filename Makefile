PROGRAM=test

all: $(PROGRAM)

OBJS = main.o

PKG_CONFIG=
PKG_CONFIG_CFLAGS=`pkg-config --cflags $(PKG_CONFIG) 2>/dev/null`
PKG_CONFIG_LIBS=`pkg-config --libs $(PKG_CONFIG) 2>/dev/null`

CFLAGS= -O2 -g -Wall

LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
LIBS=$(PKG_CONFIG_LIBS) -lsfml-graphics -lsfml-window -lsfml-system

$(PROGRAM): $(OBJS)
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)

%.o: %.cpp
	g++ -o $@ -c $+ $(CFLAGS) $(PKG_CONFIG_CFLAGS)

%.o: %.c
	gcc -o $@ -c $+ $(CFLAGS) $(PKG_CONFIG_CFLAGS)

clean:
	rm -fv $(OBJS)
	rm -fv $(PROGRAM)
	rm -fv *~

