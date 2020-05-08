CC := gcc
CFLAGS := -g -Wall -Wextra

main:  cpuboard.o main.o

cpuboard.o main.o: cpuboard.h

