#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

//define the name of the Unix Domain Socket we are going to create. It must be unique on the system
#define SOCKET_NAME "/tmp/DemoSocket"

//define the size of the buffer the server is going to use to receive any data from the client
#define BUFFER_SIZE 128


int main(int argc, char *argv[])
{
	struct sockaddr_un name;

#if 0

	struct sockaddr_un 
	{
		sa_family_t sun_family;  /*AF_UNIX*/
		char sunt_path[108]; /* pathname */
        };
#endif

	int ret;
	int connection_socket;
	int data_socket;
	int result;
	int data;
	char buffer[BUFFER_SIZE];

	//Destroy the Unix Domain socket if was already created using the same name. There can not exist two Unix sockets under the same name
	unlink(SOCKET_NAME);

	//Create a master socket 
	//SOCK_DGRAM will be used for Datagram based communication
	
	/* using socket() system call 1st argument is the the type of socket you are creating AF_UNIX, 2nd argument is the type of communication with the client: a string based communication SOCK_STREAM -SOCK_DGRAM for datagram, 3rd argument 0*/ 
	connection_socket= socket(AF_UNIX, SOCK_STREAM, 0);

	if (connection_socket==-1) //it means the socket communication fails
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	printf("Master socket file descriptor created.");


        //get prepared to call a bind system call: initialize the name variable 
	memset(&name,0, sizeof(struct sockaddr_un));
		
		
	//specify the credentials of the socket
	name.sun_family = AF_UNIX;
	strcpy(name.sun_path, SOCKET_NAME);

	/* *** bind the socket to the socket name * ***/
	/* The purpose of a bind system call() is that by this, the application dictates to the underlying operating system the criteria for receiving the data.
	 * In this case, it tells that if a server sends data to the socket "/tmp/DemoSocket"  then such data needs to be delivered to this process */

	/* Description of the bind() system call: 1st argument is the name of the connection socket=master socket file description, the 2nd argument is the pointer to the credential we already filled with the details of the Unix Domain Socket, the 3rd argument the size of the 2nd argument) */
	ret= bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));

	if( ret == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	printf("bind() system call was successful\n" );

	/* *** Next step: LISTEN() *** */
	ret=listen(connection_socket, 20); 
	
	/* 1st argument is the master socket file descriptor, the 2nd argument is the backlog number of processes which will wait for connection. If there are more than 20 processes waiting, the ones more than 20 will be dropped */
			
	/* So while one request is processed, the other requests can be set to waiting (up to 20 in this case) */
		
	if(ret==-1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* ***  Next step is to enter the server in an infinite loop. All Server Processes usually run 24x7. Good Servers should always be up and running and never go down ***  */

	for (;;)
	{
		printf("Waiting for the accept() system call");

		/* *** Next step: ACCEPT() *** */
		data_socket=accept(connection_socket, NULL, NULL);
		/* for the case of Unix Domain Sockets the 2nd and 3rd arguments are NULL
		 * the accept() system call is a blocking system call() , it means that it blocks the execution of the program until some event occurs, in this case it waits for a client to connect
		 * when the client communicates, then it releases the execution and the next line of the program is executed
		 * after the communication is received from the client the accept() system call returns to a client handler or communication file descriptor.
		 * */

		if (data_socket==-1)
		{
			perror("accept():");
			exit(EXIT_FAILURE);
		}

		printf("Connection accepted from the client.\n");

		/* implementing the practical functionality of the server: sums up all values until a special value is passed */

		result=0;

		for (;;)
		{
			/* *** Prepare the buffer to receive the data. Delete if there is something already there  *** */
			memset(buffer,0, BUFFER_SIZE);


			/* *** Wait for the next data packet *** */
			/* *** The Server is blocked at this stage, waiting for the data from the client to arrive
			 * read() is a blocking system call as well * *** */

			printf("Waiting for data from the client\n");
			ret=read(data_socket,buffer,BUFFER_SIZE); //ret is the number of bytes read by the server

			if(ret==-1)
			{
				perror("read() system call error");
				exit(EXIT_FAILURE);
			}

		        //Add received command
			memcpy(&data, buffer, sizeof(int));
			if (data==0) break;
			result+=data;

		}


		//Send result
		memset(buffer,0, BUFFER_SIZE);
		sprintf(buffer," Result=%d" ,result);

		printf("Sending the final result back to the client.\n");
		ret=write(data_socket,buffer, BUFFER_SIZE); //write() is a non block system call

		if (ret==-1)
		{
			perror("Write()");
			exit(EXIT_FAILURE);
		}

		/* *** Close socket *** */
		close(data_socket);
		
	

	}

	/* *** close the master socket file descriptor *** */
	close(connection_socket);
	printf("connection closed.\n");

	/* *** Server should release the resources before getting terminated. Unlink the socket *** */
	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);


}

