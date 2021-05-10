#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

int main(int argc, char *argv[])
{
	struct sockaddr_un addr;
	int i;
	int ret;
	int data_socket;
	char buffer[BUFFER_SIZE];

	/* ***  Create data socket *** */
	data_socket=socket(AF_UNIX,SOCK_STREAM,0); //The communication file descriptor returned is a data socket, and not a connection socket/master socket file descriptor like in the case of server

	if (data_socket==-1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}


	/* *** clear the whole struct *** */
	memset(&addr,0,sizeof(struct sockaddr_un));

	/* *** Connect the socket to the socket address *** */
	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path,SOCKET_NAME);

	ret=connect(data_socket,(const struct sockaddr *) &addr, sizeof(struct sockaddr_un));

	if(ret==-1)
	{
		fprintf(stderr,"The server is down.\n");
		exit(EXIT_FAILURE);
	}


	/* *** Send arguments *** */
	do
	{
		printf("Enter a number to send to the server: \n");
		scanf("%d",&i);
		ret=write(data_socket,&i,sizeof(int));

		if(ret==-1)
		{
			perror("write");
			exit(EXIT_FAILURE);
		}

	} while(i);


	/* *** Request the result *** */
	memset(buffer,0,BUFFER_SIZE);
	ret=read(data_socket,buffer,BUFFER_SIZE);

	if (ret== -1)
	{
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("Received from the Server: %s\n",buffer);


	/* *** Close socket *** */
	close(data_socket);

	exit(EXIT_SUCCESS);

}


