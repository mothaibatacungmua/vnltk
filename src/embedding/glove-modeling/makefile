all: coocc shuffle glove
coocc: coocc.c glove.h
	gcc -fshort-wchar -g -o  coocc coocc.c
	
shuffle: shuffle.c glove.h
	gcc -fshort-wchar -g -o  shuffle shuffle.c
	
glove: glove.c glove.h
	gcc -pthread -fshort-wchar -g -o glove glove.c -lm