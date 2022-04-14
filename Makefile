CC=g++
CFLAGS=-Iinclude -std=c++11 -g
LIBS=-lglut -lGLEW -lGL -lGLU

# Default target executed when no arguments are given to make.
default_target: as3
.PHONY : default_target

as3: as3.o InitShader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

as3.o: as3.cc
	$(CC) $(CFLAGS) -c $^

InitShader.o: common/InitShader.cc
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f as3 *~ *.o
