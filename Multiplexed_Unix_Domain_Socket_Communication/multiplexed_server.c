#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128    //bytes

#define MAX_CLIENT_SUPPORTED 32  //there are going to be up to 31 possible incoming clients, as 1 file descriptor has to be kept for the master socket file descriptor (the connection socket)


/* *** define an array of all file descriptors the server is maintaining in order to talk with the connected clients. Attention: the master socket file descriptor is also a member of this array
 * example: if there are three connected clients, then the array will contain 4 file descriptors: 3 for the communication file descriptors, and 1 which is the connection socket=master socket file descriptor ****/

int monitored_fd_set[MAX_CLIENT_SUPPORTED];

/* *** The functionality of this application is that the client sends values to the server, server sums these values up until it receives the 0 value from the client. In that case it returns the computed value to the server *** */

/* *** define an array which contains each intermediate result of the sum for each client *** */
int client_result[MAX_CLIENT_SUPPORTED]={0};


/* *** A function to initialize  the file descriptors in the monitored_fd_set array, if any existed before *** */
static void initialize_monitor_fd_set()
{
	int i;
	for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
	{
		monitored_fd_set[i]=-1;
	}
}


/* ***  A function to add a new file descriptor to the monitored_fd_set array *** */
/* *** THis function if it finds an empty file descriptor, then it adds the file descriptor from its argument to the array *** */

static void add_to_monitored_fd_set(int new_file_descriptor)
{
	int i;
	for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
	{
		if(monitored_fd_set[i]!=-1)
		{
			continue;
		}

		monitored_fd_set[i]=new_file_descriptor;
		break;
	}
}

/* *** A function to remove the file descriptor from the monitored_fd_set array *** */
/* *** This function loops through the array and when it finds the file descriptor from the argument of the function, then it removes that file descriptor from the array *** */

static void remove_from_monitored_fd_set(int file_descriptor_to_be_deleted)
{
	int i;
	for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
	{
		if(monitored_fd_set[i]!=file_descriptor_to_be_deleted)
		{
			continue;
		}

		monitored_fd_set[i]=-1;
		break;

	}
}


/* ***  A function that refreshes the fd_set collection containing the file descriptors 
 * *** This function clones the values of the file descriptors from the array to the fd_set collection of file descriptors which  will be used by the select() system call *** */
static void refresh_fd_set(fd_set *fd_set_ptr)
{
	FD_ZERO(fd_set_ptr);
	int i;
	for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
	{
		if(monitored_fd_set[i]!=-1)
		{
			FD_SET(monitored_fd_set[i],fd_set_ptr);
		}
	}
}

/* ***  A function that returns the maximum file descriptor from the collection of file descriptors it is maintaining 
 * For example, if it contains 6,7,8, then this function returns 8 *** */
static int get_max_fd()
{
	int i;
	int max=-1;

	for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
	{
		if(monitored_fd_set[i]>max)
		{
			max=monitored_fd_set[i];
		}
	}

	return max;
}

int main(int argc, char *argv[])
{
	struct sockaddr_un name;

#if 0
	struct sockaddr_un
	{
		sa_family_t sun_family;  /* AF_UNIX */
		char sunt_path[108]; /* pathname */
	};
#endif

	int ret;
	int connection_socket;
	int data_socket;
	int result;
	int data;
	char buffer[BUFFER_SIZE];

	fd_set readfds;
	int communication_socket_fd;
	int i;

	initialize_monitor_fd_set();

	/* *** in case the program exited inadvertedly on the last run, remove the socket *** */
	unlink(SOCKET_NAME);

	/* *** Create master socket *** */
	/* *** SOCK_DGRAM used for Datagram based communication *** */
	connection_socket=socket(AF_UNIX,SOCK_STREAM,0);

	if(connection_socket==-1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	printf("Master socket created!!!\n");

	/* *** Initialize *** */
	memset(&name, 0, sizeof(struct sockaddr_un));

	/* *** Specify the socket credentials *** */
	name.sun_family=AF_UNIX;
	strcpy(name.sun_path, SOCKET_NAME);

	/* *** Bind socket to socket name *** */
	/* *** Purpose of the bind() system call is that application dictates the underlying operating system the criteria for receiving the data. In this case, bind() system call tells the OS that is the sender sends the data destinated to socket "/tmp/DemoSocket" , then these pieces of data need to be delivered to the server *** */
	ret=bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));

	if (ret==-1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	printf("bind() system call was successful\n");

	/* *** Prepare for accepting the connections. The backlog size is set to 20. So while the request is processed , there can be other 19 requests waiting *** */
	ret=listen(connection_socket,20);

	if(ret==-1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* *** Add the master socket to the array monitored_set_fd *** */
	add_to_monitored_fd_set(connection_socket);

	/* *** This is the infinite loop for handling connections.
	 * All Server processes usually run 24x7. *** */

	for(;;)
	{
		refresh_fd_set(&readfds); /* *** copy the file descriptors of the array to the fd_set collection that is monitored by the select() system call *** */

		/* ***  Wait for incoming connections *** */
		printf("Waiting on select() system call\n");


		/* *** Call the select() system call, and server process blocks at this point.
		 * Linux Operating System holds the server process blocked until:
		 * 1. either a connection initiation request message is received or
		 * 2. a service request message is received from any file descriptor in the typed  fd_set readfds collection *** */

		select(get_max_fd()+1, &readfds,NULL,NULL, NULL);
		/* ***  The 1st argument is the max number of file descriptor +1 *** */
		/* *** so if the the file descriptors are 6,7,8, then the maximum is 8 and thus the 1st argument is going to be 8+1=9 *** */

		/* *** the select() system call unblocks the code flow if a connection initiation request is received (means the master socket file descriptor is activated) or it receives a status request message *** */
		/* *** Use the FD_ISSET macro to verify if the connection socket is activated *** */

		if(FD_ISSET(connection_socket, &readfds))
		{
			/* ***  Data arrives on master socket only when a new client connects with the server (that is the connect() system call is invoked on the client side) *** */
			printf("A new connection is being received, invoking accept() system call for this connection.\n");

			data_socket=accept(connection_socket,NULL,NULL);

			if (data_socket==-1)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("Connection accepted from the client.\n");

			add_to_monitored_fd_set(data_socket);
		}
		else //case of service request message from already connected client
		{
			/* *** find which is the client that sent the data request *** */
			communication_socket_fd=-1;
			for(i=0;i<MAX_CLIENT_SUPPORTED;i++)
			{
				if(FD_ISSET(monitored_fd_set[i],&readfds))
				{
					communication_socket_fd=monitored_fd_set[i];

					/* *** Prepare the buffer to receive the data *** */
					memset(buffer,0, BUFFER_SIZE);

					/* *** Wait for the next data package *** */
					/* *** Server process blocks here waiting for new service request message to arrive as read() is a blocking system call *** */
					printf("Waiting for data from the client.\n");
					ret=read(communication_socket_fd,buffer,BUFFER_SIZE);

					if(ret==-1)
					{
						perror("read");
						exit(EXIT_FAILURE);
					}

					/* ***  Add received sum *** */
					memcpy(&data, buffer, sizeof(int));
					if(data==0)
					{
						/* ***  Send result *** */
						memset(buffer,0, BUFFER_SIZE);
						sprintf(buffer,"Result=%d", client_result[i]);

						printf("Sending final result back to client.\n");
						ret=write(communication_socket_fd,buffer, BUFFER_SIZE);

						if(ret==-1)
						{
							perror("write");
							exit(EXIT_FAILURE);
						}

						/* *** Close the socket *** */
						close(communication_socket_fd);
						client_result[i]=0;
						remove_from_monitored_fd_set(communication_socket_fd);
						continue; /* *** goto select() and block *** */
					}
					client_result[i]+=data;
				}
			}
		}

	} /* go to select() and block */

	/* *** close the market socket *** */
	close(connection_socket);
        remove_from_monitored_fd_set(connection_socket);
	printf("Connection closed...\n");

	/* *** Server should release resources before getting terminated. Unlink the socket *** */
	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);

}
