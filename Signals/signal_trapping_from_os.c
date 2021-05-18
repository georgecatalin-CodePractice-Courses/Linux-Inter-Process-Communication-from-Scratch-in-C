#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/* *** these are  the SIGNAL HANDLER FUNCTIONS *** */
//REGISTRATION OF THE SIGNAL HANDLER
static void handler_for_ctrl_c(int sig)
{
	printf("Hey,man... CTRL+C was pressed.\n");
	printf("See you soon.\n");
	exit(0);
}


//REGISTRATION OF THE SIGNAL HANDLER
static void handler_for_abort(int sig)
{
	printf("Process is aborted.\n");
	printf("Bye!");
	exit(0);
}


int main(int argc, char **argv)
{
	/* *** register the signal handler functions to the signals *** */
	signal(SIGINT,handler_for_ctrl_c);
	signal(SIGABRT,handler_for_abort);

	char chr;

	printf("Abort process (y/n)? \n");
	scanf("%c", &chr);

	if(chr=='y')
	{
		abort();
	}

	return 0;
}

