# Compiler
CC = g++

TARGET = Nbody
CFLAGS = -std=c++20 -c -fopenmp
LDFLAGS = -std=c++20 -fopenmp
OBJ = main.o Force.o QuadTree.o Image.o

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJ)

%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.bmp $(TARGET)
