pgn2web: pgn2web.c
	gcc -Wall -o pgn2web pgn2web.c

clean:
	rm pgn2web
	rm *[0-9]*.html

debug: pgn2web.c
	gcc -DDEBUG -Wall -g -o pgn2web pgn2web.c

install:
	mkdir /usr/local/pgn2web/
	cp -r images /usr/local/pgn2web/
	cp template.html /usr/local/pgn2web
	cp pgn2web /usr/local/pgn2web/
	ln -s /usr/local/pgn2web/pgn2web /usr/bin/

windows:  pgn2web.c
	gcc -DWINDOWS -Wall -o pgn2web pgn2web.c