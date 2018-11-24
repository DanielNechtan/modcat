CFLAGS = -std=c99 -pedantic -Wall -Wextra -O0 -g
LDFLAGS += -lsndio 

modcat: modcat.o
	gcc -o modcat modcat.o $(CFLAGS) $(LDFLAGS)

modcat.o: modcat.c
	gcc -o modcat.o modcat.c -c $(CFLAGS)

clean:
	rm -f modcat *.o
