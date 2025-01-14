CC := gcc
CFLAGS := -Wall -Werror -g -pthread

# files and object variables. Object is regex replace
TARGET = reader_writer
SRC = main.c reader.c writer.c
OBJ = $(SRC:.c=.o)

# AUTOMATIC VARIABLES
# $@ target name
# $^ all prerequisites
# $? all prerequisites newer than target
# $< first prerequisite

# IMPLICIT COMPILATION RULE VARIABLES
# CC: Program for compiling C programs; default cc
# CXX: Program for compiling C++ programs; default g++
# CFLAGS: Extra flags to give to the C compiler
# CXXFLAGS: Extra flags to give to the C++ compiler
# CPPFLAGS: Extra flags to give to the C preprocessor
# LDFLAGS: Extra flags to give to compilers when they are supposed to invoke the linker

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)