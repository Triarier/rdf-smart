CC      = /usr/bin/gcc
CFLAGS  = -I/usr/include/rasqal -I/usr/include/raptor2  
LIBS    = -lraptor2 -lrasqal
roqet: roqet.c
	$(CC) -std=c99 $(CFLAGS) roqet.c -o roqet $(LIBS)
