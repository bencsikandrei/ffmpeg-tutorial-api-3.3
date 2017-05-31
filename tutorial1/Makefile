## Makefile for tutorial1
CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=-I/local/include -I./include -L/local/lib -L./lib

EXEC=bin/main

FFMPEG=avdevice avformat avfilter avcodec swscale avutil
OTHERS=m z swresample

LIBS=$(addprefix -l,$(FFMPEG) $(OTHERS))

SRC=$(wildcard src/*.c)

all: clean
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(SRC) $(LIBS)

clean:
	rm -rf $(EXEC)
run:
	./$(EXEC)

mrproper: clean
	rm -rf frame*

.PHONY: all clean mrproper
