#include <arpa/inet.h>		// htons()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), send(), recv()
#include <unistd.h>		// close()

#include "helpers.h"		// make_server_sockaddr(), get_port_number()

static const size_t MAX_MESSAGE_SIZE = 256;

/**
 * Receives a string message from the client and prints it to stdout.
 *
 * Parameters:
 * 		connectionfd: 	File descriptor for a socket connection
 * 				(e.g. the one returned by accept())
 * Returns:
 *		0 on success, -1 on failure.
 */
int handle_connection(int connectionfd) {

	printf("New connection %d\n", connectionfd);

	char msg[MAX_MESSAGE_SIZE + 1];
	memset(msg, 0, sizeof(msg));
	// (1) Receive message from client.
	size_t rval;
	size_t recvd = 0;
	while(rval > 0)
	{
		rval = recv(connectionfd, msg + recvd, MAX_MESSAGE_SIZE - recvd, 0);
		if (rval == -1) {
			perror("Error reading stream message");
			return -1;
		}
		recvd += rval;
	}
	
	// (2) Print out the message
	printf("Client %d says '%s'\n", connectionfd, msg);

	// (3) Close connection
	close(connectionfd);

	return 0;
}

/**
 * Endlessly runs a server that listens for connections and serves
 * them _synchronously_.
 *
 * Parameters:
 *		port: 		The port on which to listen for incoming connections.
 *		queue_size: 	Size of the listen() queue
 * Returns:
 *		-1 on failure, does not return on success.
 */
int run_server(int port, int queue_size) {

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Error opening stream socket");
		return -1;
	}

	// (2) Set the "reuse port" socket option
	int type, size;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &type, size);

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	if (make_server_sockaddr(&addr, port) == -1) {
		return -1;
	}

	// (3b) Bind to the port.
	if(bind(sockfd, (const sockaddr *) &addr, sizeof(addr)) == -1){
		perror("Error binding stream socket");
		return -1;
	}

	// (3c) Detect which port was chosen.
	port = get_port_number(sockfd);
	printf("Server listening on port %d...\n", port);

	// (4) Begin listening for incoming connections.
	listen(sockfd, queue_size); // allow a queue of up to 50 connections

	// (5) Serve incoming connections one by one forever.
	while(1)
	{
		int newsock = accept(sockfd, (struct sockaddr *) 0, (socklen_t *) 0);
		if(newsock == -1)
		{
			perror("error with newsock");
			return -1;
		}
		else
		{
			if(handle_connection(newsock) == -1)
			{
				return -1;
			}
		}
		
	}
}

int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 2) {
		printf("Usage: ./server port_num\n");
		return 1;
	}
	int port = atoi(argv[1]);

	if (run_server(port, 10) == -1) {
		return 1;
	}
	return 0;
}
