pgn2web: pgn2web.o chess.o
	gcc -Wall -o pgn2web pgn2web.o chess.o

chess.o: chess.c chess.h
	gcc -Wall -c chess.c

pgn2web.o: pgn2web.c chess.h
	gcc -Wall -c pgn2web.c

clean:
	rm pgn2web *.o *.do *~ *.html

debug: pgn2web.do chess.do
	gcc -Wall -g -o pgn2web pgn2web.do chess.do 

chess.do: chess.c chess.h
	gcc -Wall -c -g -DDEBUG -o chess.do chess.c

pgn2web.do: pgn2web.c chess.h
	gcc -Wall -c -g -DDEBUG -o pgn2web.do pgn2web.c

install:
	if [ ! -e /usr/local/pgn2web ]; then mkdir /usr/local/pgn2web; fi
	cp -r images /usr/local/pgn2web/
	cp -r templates /usr/local/pgn2web/
	cp pgn2web /usr/local/pgn2web/
	ln -fs /usr/local/pgn2web/pgn2web /usr/bin/

windows:  pgn2web.c
	gcc -DWINDOWS -Wall -o pgn2web pgn2web.c