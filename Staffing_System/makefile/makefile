CC:= gcc
NAME ?= staffing_system_ser
NAME1 ?= ser
APP ?= staffing_system_cli
APP1 ?= cli
OBJ := -lsqlite3 -lpthread 


all:
	$(CC) $(APP).c  -o $(APP1)
	$(CC) $(NAME).c  -o $(NAME1) $(OBJ)

obj-m := $(NAME).o $(APP).o

.PHONY:clean
clean:
	rm $(NAME1) $(APP1) 
	#*.db
