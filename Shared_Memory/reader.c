#include <stdio.h>
#include <string.h>

extern int read_from_shared_memory(char *mmap_key, char *buffer, unsigned int buffer_size, unsigned int bytes_to_read);
/* *** 
 extern functions can be called anywhere in any of the files of the whole program,provided those files contain a declaration of the function. The compiler knows the definition of the functions exists somewhere else and it goes ahead with the compilation *** */

 int main(int argc, char **argv)
 {
	char *key="/introduction" ; //same key used by writer process
	char read_buffer[128]; //the reader process needs a buffer to read the content of the shared memory into its buffer

	memset(read_buffer,0,128); //flush the read_buffer for whatever is already existing there

	int rc=read_from_shared_memory(key, read_buffer,128,128);

	if(rc<0)
	{
		printf("Error reading from the shared memory.\n");
		return -1;
	}

	printf("Data read=%s\n", (char *) read_buffer);

	return 0;
}







