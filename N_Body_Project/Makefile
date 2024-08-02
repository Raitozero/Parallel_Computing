#Compiler
CC = g++

TARGET = Nbody
CFLAGS = -std=c++20 -c
OBJ = main.o Force.o QuadTree.o Image.o

$(TARGET): $(OBJ)
	$(CC) -std=c++20 -o $@ $^

%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONEY: clean
clean:
	rm -f *.o *.bmp $(TARGET)