pgn2web: pgn2web.c
	gcc -Wall -o pgn2web pgn2web.c

clean:
	rm pgn2web
	rm *.html

debug: pgn2web.c
	gcc -DDEBUG -Wall -g -o pgn2web pgn2web.c

install:
	if [ ! -e /usr/local/pgn2web ]; then mkdir /usr/local/pgn2web; fi
	cp -r images /usr/local/pgn2web/
	cp -r templates /usr/local/pgn2web/
	cp pgn2web /usr/local/pgn2web/
	ln -fs /usr/local/pgn2web/pgn2web /usr/bin/

windows:  pgn2web.c
	gcc -DWINDOWS -Wall -o pgn2web pgn2web.c