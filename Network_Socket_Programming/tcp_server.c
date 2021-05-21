#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"

/* ***  Server process is running at this port number. Client has to send data to this port number *** */
#define SERVER_PORT 2000

test_struct_t test_struct;
result_struct_t result_struct;
char data_buffer[1024];

void setup_tcp_server_communication()
{
	/* ***  STEP 1. INITIALIZATION
	* ***  Socket handle and other variables
	* *** Master socket file descriptor, used only to accept new connection initiation request messages, does not accept service request messages *** */
	int master_socket_tcp_fd=0;
	int sent_recv_bytes=0;
	int addr_len=0;
	int opt=1;

	/* *** Client communication file descriptor
	* *** It is used only for data exchange and communications between client and server *** */
	int comm_socket_fd=0;

	/* ***  Define and initialize the data type collection that will contain all the file descriptors in the program ***  */
	fd_set readfds;

	/* *** Define the variable to hold the information of addresses associated with the server and the client *** */
	/* *** This a structure to hold server and client address and port number information *** */
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	/* *** STEP 2. CREATE MASTER SOCKET FILE DESCRIPTOR *** */
	if((master_socket_tcp_fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	/* *** 1st argument of socket() system call  defines what is the address family you are using. For IPV4 always AF_INET, for IPV6 AF_INET6
	* *** 2nd argument is SOCK_STREAM or SOCK_DGRAM
	* *** 3rd argument defines the protocol you want to run on the top of the network layer IPPROTO_TCP
	for TCP communication (SOCK_STREAM, IPPROTO_TCP) for UDP communication (SOCK_DGRAM,IPPROTO_UDP)
	* ***/
	{
		printf("Socket creation has failed.\n");
		exit(1);
	}


	/* *** BIND() system call *** */
	/* *** Binding means that you are telling the Operating System that any data you receive with the destination IP Address=a1 and TCP port=p1 will be sent to this process 
	/* *** Bind() system call is a mechanism to tell the operating system what kind of data is the server interested to receive.
	/* *** Note: A server machine can run multiple server processes simultaneously to process data from multiple clients.
	/* *** bind() system call is only used on the server side, and not on the client side *** */

	/* *** Specify the server information : address and port number *** */
	server_address.sin_family=AF_INET; // this server is willing to interprete only IPv4 network packages AF_INET (AF_INET6 for IPv6)
	server_address.sin_port= SERVER_PORT; // this server will process only network packages meant for port SERVER_PORT defined as constant earlier as 2000

    /* ***  Add the server ip address.
	/* ***  Operating system will send all the packages meant for the address to the process
	/* *** Note: the IPv4 have to be transformed into an integer numbers e.g. 192.168.56.101 => 3232249957
	/* *** In the case of localhost one can use the constant INADDR_ANY *** */
	/* *** INADDR_ANY means that because a server can have multiple IP Addresses, one address for each local interface of the server, server is interested in receiving all packages to any local interfaces of the server *** */
	server_address.sin_addr.s_addr=INADDR_ANY;

    addr_len=sizeof(struct sockaddr);

    if(bind(master_socket_tcp_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr))== -1 )
	{
		printf("Socket bind has failed.\n");
		return;
	}


    /* ***  STEP 4. THE LISTEN() system call *** */
    /* *** With the listen() system call, the server instructs the operating system to maining a queue of packages of maximum length as specified in arguments *** */
	/* ***  Basically, this means that the server is telling the operating system to maintain a queue of maximum number of clients , and neglect the others *** */
	if(listen(master_socket_tcp_fd,5)<0)
	{		
		printf("Listen has failed.\n");
		return;
    }

	/* *** STEP 5. INITIALIZE AND ADD THE MASTER SOCKET FILE DESCRIPTOR TO THE READFDS collection of file descriptors *** */
	/* ***  Enter and infinite loop to service the client. This way the server logic will be implemented inside the infinite loop *** */

	while(1)
	{
		FD_ZERO(&readfds); //initializes the file descriptors set using the FD_ZERO macro to empty the readfds
		FD_SET(master_socket_tcp_fd, &readfds); //add the master socket file descriptor to the readfds collection

		printf("Now the server is blocked on the select() system call ...\n");

   /* *** STEP 6. WAIT FOR CLIENT CONNECTION *** */
   /* *** Call the select() system call, server process blocks now for incoming connections from new clients or existing ones. *** */
   /* *** The select() system call remains blocked until data arrives for any file descriptors stored in the readfds collection *** */
   select(master_socket_tcp_fd+1,&readfds, NULL, NULL, NULL);
   /* *** select() 1st argument is 1+maximum file descriptor existing in the readfds *** */
   
   /* *** select() system call blocks the execution at line 101 until data arrives on a file descriptor stored in the readfds collection *** */

   /* ***  Finally, some data has arrived for the file descriptors stored in the readfds collection. Verify where such data is targeted and run *** */
		if(FD_ISSET(master_socket_tcp_fd, &readfds)) //the FD_ISSET macro checks whether the 1st argument is activated in the readfds collection
		{
		/* *** Data arrives on the master socket file descriptor only when a new client sends a connection initiation request message. That is the client emitted a connect() system call on the client side *** */
			printf("New connection received, accept the connection. Client and server completes TCP-3 way handshake at this  point.\n");


	/* *** STEP 7. ACCEPT() SYSTEM CALL GENERATES A NEW FILE DESCRIPTOR COMM_SOCKET_FD *** */
	/* *** The server uses the new file descriptor comm_socket_fd  for the rest of the lifetime of the connection with that client to send and receive all messages *** */
	/* *** The master socket file descriptor is used only for new client connections and not for data exchange with existing clients *** */
		comm_socket_fd= accept(master_socket_tcp_fd, (struct sockaddr *) &client_address, &addr_len);
	
			if(comm_socket_fd<0)
			{
		/* *** if accept failed to return a communication file descriptor, display a relevant error and exit *** */
				printf("Accept error: errno=%d\n",errno);
				exit(0);
    		}

	        while(1)
			{
				printf("The server is ready to receive messages.\n");
				memset(data_buffer,0, sizeof(data_buffer)); //drain the memory which will contain the message data

				/* *** STEP 8. SERVICE THE REQUEST *** */
				/* ***  Server receiving the data from the Client, client IP Address and Port Number will be stored in the client_address struct *** */
				/* *** Server will use the client_address struct to reply back to the client. *** */


			    /* *** Following there is also a blocking system call, meaning the server process halts its execution here until data arrives on this comm_socket_fd from a client whose connection request has been previously accepted *** */
				sent_recv_bytes=recvfrom(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer),0, (struct sockaddr *) &client_address, &addr_len);

				printf("The server has received %d bytes from the client %s at port %u.\n" , sent_recv_bytes,inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));

				if(sent_recv_bytes == 0)
				{
					/* *** if a server receives an empty message from the client, the server may close the connection from the client and wait for a new connection from that client *** */
					close(comm_socket_fd);
					break; //go to STEP 5 which is UPDATE AND FILL THE readfds collection and continue the next steps
				}	

				test_struct_t *client_data=(test_struct_t *) data_buffer;

			/* ***  STEP 9. *** */
				if(client_data->a==0 && client_data->b==0)
				{
					close(comm_socket_fd);
					printf("Server closes connection with the client: %s on port %u.\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
					break; //get out of the while loop, server is done with this client, it should check for new connections requestes by running select()
				}

				result_struct_t result;
				result.c = client_data->a + client_data->b;

				/* *** Server replying back to client now *** */
				sent_recv_bytes=sendto(comm_socket_fd, (char *) &result, sizeof(result_struct_t),0,(struct sockaddr *) &client_address, sizeof(struct sockaddr));

				printf("Server sent %d bytes in reply to the client.\n",sent_recv_bytes);
			}
		}
	printf("Connection accepted by the client %s: %u\n", inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));

	} 

}


int main(int argc, char **argv[])
{
	setup_tcp_server_communication();
	return 0;
}
