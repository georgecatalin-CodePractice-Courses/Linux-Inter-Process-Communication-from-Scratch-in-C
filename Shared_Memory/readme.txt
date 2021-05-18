How to compile the files into executable programs

gcc -g -c writer.c -o writer.o
gcc -g -c reader.c -o reader.o
gcc -g -c shared_memory_utilities.c -o shared_memory_utilities.o


gcc -g writer.o shared_memory_utilities.o -o writer -lrt
gcc -g reader.o shared_memory_utilities.o -o reader -lrt
