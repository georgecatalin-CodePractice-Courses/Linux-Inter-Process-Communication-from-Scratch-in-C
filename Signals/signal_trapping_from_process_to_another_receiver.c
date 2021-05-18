#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static void signal_handle(int sig)
{
	printf("Signal %d was received.\n",sig);
}

int main(int argc, char **argv)
{
	signal(SIGUSR1,signal_handle);
	scanf("\n"); //this program will not terminate its execution
	return 0;
}


