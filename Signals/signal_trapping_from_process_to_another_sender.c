#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char **argv )
{
	kill(3576,SIGUSR1); //the processID of the recipient process ps -aux
	scanf("\n");
	return 0;
}
