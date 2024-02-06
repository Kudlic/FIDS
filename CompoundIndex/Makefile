CC = /usr/bin/g++
CFLAGS = -fdiagnostics-color=always -O3 
SRC = cSchema.cpp bitMapIndex.cpp cRowHeapTable.cpp main.cpp
OUT = main

all: release

release:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
