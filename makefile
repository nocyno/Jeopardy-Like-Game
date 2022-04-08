jeopardy: jeopardy.o parser.o
	gcc -o jeopardy jeopardy.o parser.o `pkg-config --libs gtk+-3.0` `pkg-config --libs json-c`
jeopardy.o: jeopardy.c
	gcc -g -c jeopardy.c `pkg-config --cflags gtk+-3.0` `pkg-config --cflags json-c`
parser.o: parser.c
	gcc -g -c parser.c `pkg-config --cflags --libs json-c`
