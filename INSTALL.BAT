@ECHO OFF
MD C:\PROGRA~1\pgn2web
MD C:\PROGRA~1\pgn2web\images
COPY images C:\PROGRA~1\pgn2web\images
COPY template.html C:\PROGRA~1\pgn2web
COPY pgn2web.exe C:\PROGRA~1\pgn2web
SET PATH=%PATH%;C:\PROGRA~1\pgn2web
ECHO SET PATH=%%PATH%%;C:\PROGRA~1\pgn2web >> C:\AUTOEXEC.BAT
ECHO Finished