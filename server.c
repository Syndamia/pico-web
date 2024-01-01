/* The server recieves connections and passes files to the clients
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#include <string.h>
#include <sds/sds.h>
#include <util.h>

#include <server-connection.h>
#include <server-cli.h>

int createCommunicationSocket(const char* ip, const char* port) {
	int fd_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	herr(fd_socket, "socket");

	int aton_status = 0;
	struct sockaddr_in sa_socket = {
		.sin_family = AF_INET,
		.sin_port   = atop(port),
		.sin_addr   = aton(ip, &aton_status),
	};
	herr(aton_status, "inet_aton");

	// Reuse address when in TIME_WAIT state, after listening socket was closed
	// https://stackoverflow.com/a/10651048/12036073
	// https://superuser.com/questions/173535/what-are-close-wait-and-time-wait-states#comment951880_173543
	int true = 1;
	herr(setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)), "setsockopt");

	herr(bind(fd_socket, (struct sockaddr*)&sa_socket, sizeof(struct sockaddr_in)), "bind");

	herr(listen(fd_socket, 50), "listen");

	return fd_socket;
}

void freeVhosts(sds **vhosts, int vhostsc) {
	for (int i = 0; i < vhostsc; i++) {
		sdsfreesplitres(vhosts[i], 3);
	}
	free(vhosts);
}

int acceptConnections = 1;

void handler_refuseConnections(int signum) {
	acceptConnections = 0;
}

int main(int argc, char* argv[]) {
	/*
	 * Get server parameters
	 */

	sds host = sdsnew("127.0.0.1");
	sds port = sdsnew("8080");

	int argvOffset = 1;
	if (charCount(argv[1], ',') == 1) {
		argvOffset++;
		char* sep = strchr(argv[1], ',');
		sdsfree(host);
		host = sdsnewlen(argv[1], sep - argv[1]);
		sdsfree(port);
		port = sdsnew(sep + 1);
	}

	/*
	 * Get hosts
	 */

	int vhostsc = argc - argvOffset;
	sds **vhosts = malloc(vhostsc * sizeof(sds*));
	for (int i = 0, temp = 0; i < vhostsc; i++) {
		vhosts[i] = sdssplitlen(argv[i+argvOffset], strlen(argv[i+argvOffset]), ",", 1, &temp);
	}
	
	/*
	 * Create socket for accepting connections
	 */
	
	int fd_socket = createCommunicationSocket(host, port);
	printf("Listening on %s:%s\n", host, port);

	/*
	 * Server command-line interface
	 */

	int pid_cli = fork();
	if (pid_cli == 0) {
		close(fd_socket);
		handleCLI(vhosts, vhostsc);
		freeVhosts(vhosts, vhostsc);
		//while(wait(NULL) > 0);
		return 0;
	}

	/*
	 * Define variables
	 */

	// Client address
	struct sockaddr_in sa_client = {
		.sin_family = AF_INET,
		.sin_port = 0,
		.sin_addr.s_addr = 0,
	};
	socklen_t sa_client_size = sizeof(struct sockaddr_in);

	int fd_client;
	int count = 0;
	int pselectStat = 0;

	/*
	 * Handle connection requests
	 */

	// Data for pselect
	fd_set rfds;
	struct timespec tv = {
		/* How long should we block (wait) for fd_socket to have a request */
		.tv_sec  = 0,
		.tv_nsec = 100000,
	};

	// Stop accepting connections on SIGTERM
	signal(SIGTERM, handler_refuseConnections);

	while (acceptConnections) {
		/*
		 * Check if fd_socket has something we can read
		 */

		FD_ZERO(&rfds);
		FD_SET(fd_socket, &rfds);

		herrc(pselect(fd_socket + 1, &rfds, NULL, NULL, &tv, NULL), "pselect");
		if (!FD_ISSET(fd_socket, &rfds)) continue;

		/* 
		 * Accept the connection
		 */

		fd_client = accept(fd_socket, (struct sockaddr*)&sa_client, &sa_client_size);
		herrc(fd_client, "accept");
		if (fd_client < 0) continue;

		char* strAddr = inet_ntoa(sa_client.sin_addr);
		count++;

		/*
		 * Logic when connected with client
		 */

		int fp = fork();
		if (fp == 0) {
			close(fd_socket);
			on_connection(strAddr, fd_client, vhosts, argc - 1);
			close(fd_client);
			freeVhosts(vhosts, vhostsc);
			return 0;
		}
		close(fd_client);
	}

	while(wait(NULL) > 0);
	freeVhosts(vhosts, vhostsc);
	close(fd_socket);
}
