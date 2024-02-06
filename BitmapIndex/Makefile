CC = /usr/bin/g++
CFLAGS = -fdiagnostics-color=always -O3 -Wall -Wextra 
SRC = cRowHeapTable.cpp bitMapIndex.cpp main.cpp
OUT = main

all: release

release:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
