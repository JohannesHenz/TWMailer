#############################################################################################
# Makefile
#############################################################################################
# G++ is part of GCC (GNU compiler collection) and is a compiler best suited for C++
CC=g++

# Compiler Flags: https://linux.die.net/man/1/g++
#############################################################################################
# -g: produces debugging information (for gdb)
# -Wall: enables all the warnings
# -Wextra: further warnings
# -Werror: treat warnings as errors
# -O: Optimizer turned on
# -std: use the C++ 14 standard
# -c: says not to run the linker
# -pthread: Add support for multithreading using the POSIX threads library. This option sets 
#           flags for both the preprocessor and linker. It does not affect the thread safety 
#           of object code produced by the compiler or that of libraries supplied with it. 
#           These are HP-UX specific flags.
#############################################################################################
CFLAGS=-g -Wall -Wextra -Werror -O -std=c++14 -pthread

rebuild: clean all
all: ./bin/twmailer-client ./bin/twmailer-server ./bin/Server-from-scratch ./bin/Client-from-scratch

clean:
	clear

./obj/twmailer-client.o: twmailer-client.cpp
	${CC} ${CFLAGS} -o obj/twmailer-client.o twmailer-client.cpp -c

./obj/twmailer-server.o: twmailer-server.cpp
	${CC} ${CFLAGS} -o obj/twmailer-server.o twmailer-server.cpp -c

./obj/Client-from-scratch.o: Client-from-scratch.cpp
	${CC} ${CFLAGS} -o obj/Client-from-scratch.o Client-from-scratch.cpp -c

./obj/Server-from-scratch.o: Server-from-scratch.cpp
	${CC} ${CFLAGS} -o obj/Server-from-scratch.o Server-from-scratch.cpp -c


./bin/twmailer-client: ./obj/twmailer-client.o
	${CC} ${CFLAGS} -o bin/twmailer-client obj/twmailer-client.o

./bin/twmailer-server: ./obj/twmailer-server.o
	${CC} ${CFLAGS} -o bin/twmailer-server obj/twmailer-server.o

./bin/Client-from-scratch: ./obj/Client-from-scratch.o
	${CC} ${CFLAGS} -o bin/Client-from-scratch obj/Client-from-scratch.o

./bin/Server-from-scratch: ./obj/Server-from-scratch.o
	${CC} ${CFLAGS} -o bin/Server-from-scratch obj/Server-from-scratch.o

