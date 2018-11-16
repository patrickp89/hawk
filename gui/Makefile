SHELL = /bin/sh



all: clean build

build: .o
	gcc missile_control.o -o missile_control -lncurses

.o:
	gcc -c missile_control.c
	
clean: 
	-@$(RM) missile_control.o
	-@$(RM) missile_control
