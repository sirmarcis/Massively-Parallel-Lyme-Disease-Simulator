all: clcg4.h clcg4.c lyme.c
	gcc -I. -Wall -O3 -c clcg4.c -o clcg4.o
	mpicc -I. -Wall -O3 lyme.c config.c mouse_list.c nest_list.c clcg4.o -o lyme -lpthread
