pgn2web: chess.o cli.o pgn2web.o
	gcc -Wall -o pgn2web chess.o cli.o pgn2web.o

chess.o: chess.c chess.h
	gcc -Wall -c chess.c

cli.o: cli.c pgn2web.h
	gcc -Wall -c cli.c

gui.o : gui.cpp gui.h pgn2web.h
	g++ -Wall -c gui.cpp `wx-config --cxxflags`

pgn2web.o: pgn2web.c pgn2web.h chess.h nag.h
	gcc -Wall -c pgn2web.c

p2wgui: chess.o gui.o pgn2web.o
	g++ -Wall -o p2wgui chess.o gui.o pgn2web.o `wx-config --libs`

debug: chess.do cli.do pgn2web.do
	gcc -Wall -o pgn2web -g -DDEBUG chess.do cli.do pgn2web.do

chess.do: chess.c chess.h
	gcc -Wall -o chess.do -c -g -DDEBUG chess.c

cli.do: cli.c pgn2web.h
	gcc -Wall -o cli.do -c -g -DDEBUG cli.c

gui.do : gui.cpp gui.h pgn2web.h
	g++ -Wall -o gui.do -c -g -DDEBUG gui.cpp `wx-config --cxxflags`

pgn2web.do: pgn2web.c pgn2web.h chess.h nag.h
	gcc -Wall -o pgn2web.do -c -g -DDEBUG pgn2web.c

p2wgui.do: chess.do gui.do pgn2web.do
	g++ -Wall -o p2wgui.do -g -DDEBUG p2wgui.do chess.do gui.do pgn2web.do `wx-config --libs`

clean:
	rm -f pgn2web p2wgui *.o *.do *~ *# *.html

install:
	if [ ! -e /usr/local/pgn2web ]; then mkdir /usr/local/pgn2web; fi
	cp -r images /usr/local/pgn2web/
	cp -r templates /usr/local/pgn2web/
	cp pgn2web /usr/local/pgn2web/
	cp p2wgui /usr/local/pgn2web/
	ln -fs /usr/local/pgn2web/pgn2web /usr/bin/
	ln -fs /usr/local/pgn2web/p2wgui /usr/bin/