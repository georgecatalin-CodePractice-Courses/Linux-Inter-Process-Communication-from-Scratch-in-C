#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/* *** function prototyping *** */
void function_catch_signal(int signal);

int main(int argc, char **argv)
{
	int ret;

	ret=signal(SIGINT,function_catch_signal); //whenever SIGINT signal is caught, then the function_catch_signal() is invoked

	if ( ret== SIG_ERR)
	{
		printf("Error: unable to set signal handler.\n");
		exit(0);
	}

	printf("I am going to raise a signal now.\n");

	ret=raise(SIGINT); //Passing the signal number as an argument to the raise() system call. Since it raises the SIGINT signal to itself, process does not need CTRL+C to fire up the signal

	if(ret!=0)
	{
		printf("Error: unable to raise SIGINT signal to the process itself.\n");
		exit(0);
	}

	printf("Exiting...\n");
	return 0;
}


void function_catch_signal(int signal)
{	
	printf("!! signal caught !! \n");
}


