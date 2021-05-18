#include <stdio.h>
#include <string.h>


extern  int create_and_write_shared_memory(char *mmap_key, char *value, unsigned int size); //extern keyword associated to functions make these functions available all over the program ,provided this file is references in the header of the source code


int main(int argc, char *argv[])
{
	char *key="/introduction";
	char *intro="What's up, alligator? Nice to meet you." ;
	create_and_write_shared_memory(key,intro, strlen(intro));


	return 0;
}
