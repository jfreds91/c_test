CC := gcc
CFLAGS := -Wall

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

blah: blah.o
	$(CC) $(CFLAGS) blah.o -o blah # Runs third. Can delete this line entirely to use implicit compilation

blah.o: blah.c
	$(CC) $(CFLAGS) -c blah.c -o blah.o # Runs second. -c means no linking

# Typically blah.c would already exist, but I want to limit any additional required files
blah.c:
	echo "int main() { return 0; }" > blah.c # Runs first

clean:
	rm -f blah.c blah.o blah