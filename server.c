/* The server recieves connections and passes files to the clients
 */
#include <server-connection.h>

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

#define MAX_LEN_COMMAND 16
#define COMMAND_FORMAT ": %16s"

void freeVhosts(sds **vhosts, int argc) {
	for (int i = 1; i < argc; i++) {
		sdsfreesplitres(vhosts[i-1], 3);
	}
	free(vhosts);
}

int streq(const char* first, const char* second) {
	return strcmp(first, second) == 0;
}

int acceptConnections = 1;

void handler_refuseConnections(int signum) {
	acceptConnections = 0;
}

int main(int argc, char* argv[]) {
	/*
	 * Get hosts
	 */

	sds **vhosts = malloc((argc - 1) * sizeof(sds*));
	for (int i = 1, temp = 0; i < argc; i++) {
		vhosts[i-1] = sdssplitlen(argv[i], strlen(argv[i]), ",", 1, &temp);
	}
	
	/*
	 * Create socket for accepting connections
	 */
	int fd_socket;
	herr(fd_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0), "socket");

	struct sockaddr_in sa_socket = {
		.sin_family = AF_INET,
		.sin_port = inet_atop("8080"),
	};
	herr(inet_aton("127.0.0.1", &sa_socket.sin_addr.s_addr), "inet_aton");

	// Reuse address when in TIME_WAIT state, after listening socket was closed
	// https://stackoverflow.com/a/10651048/12036073
	// https://superuser.com/questions/173535/what-are-close-wait-and-time-wait-states#comment951880_173543
	int true = 1;
	herr(setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)), "setsockopt");

	herr(bind(fd_socket, (struct sockaddr*)&sa_socket, sizeof(struct sockaddr_in)), "bind");

	herr(listen(fd_socket, 50), "listen");
	
	printf("Listening on %s:%s\n", "127.0.0.1", "8080");

	/*
	 * Accept connection on the socket
	 */

	int pid_connections = fork();
	if (pid_connections == 0) {
		// Client address
		struct sockaddr_in sa_client = {
			.sin_family = AF_INET,
			.sin_port = 0,
			.sin_addr.s_addr = 0,
		};
		socklen_t sa_client_size = sizeof(struct sockaddr_in);

		// Data for pselect
		fd_set rfds;
		struct timespec tv = {
			.tv_sec = 0,
			.tv_nsec = 100000,
		};

		int fd_client;
		int count = 0;
		int pselectStat = 0;

		// Effecitvely stops the cycle on SIGTERM
		signal(SIGTERM, handler_refuseConnections);

		while (acceptConnections) {
			/* Check if fd_socket has something we can read */
			FD_ZERO(&rfds);
			FD_SET(fd_socket, &rfds);

			herrc(pselect(fd_socket + 1, &rfds, NULL, NULL, &tv, NULL), "pselect");
			if (!FD_ISSET(fd_socket, &rfds)) continue;

			/* If so, accept the connection and pass execution to the "main" logic */
			fd_client = accept(fd_socket, (struct sockaddr*)&sa_client, &sa_client_size);
			herrc(fd_client, "accept");
			if (fd_client < 0) continue;

			char* strAddr = inet_ntoa(sa_client.sin_addr);
			count++;

			int fp = fork();
			if (fp == 0) {
				close(fd_socket);
				on_connection(strAddr, fd_client, vhosts, argc - 1);
				close(fd_client);
				freeVhosts(vhosts, argc);
				return 0;
			}
			close(fd_client);
		}

		while(wait(NULL) > 0);
		freeVhosts(vhosts, argc);
		close(fd_socket);

		return 0;
	}

	/*
	 * Server command-line interface
	 */

	close(fd_socket);

	// Get a line
	char line[256];
	fgets(line, 256, stdin);

	// Get command name and it's arguments
	// Currently no command takes arguments
	char name[MAX_LEN_COMMAND+1];
	int argsAssigned = sscanf(line, COMMAND_FORMAT, name);

	while (name[0] != 'q' && name[0] != 'e' && !streq(name, "quit") && !streq(name, "exit")) {
		if (argsAssigned < 1) {
			printf("Bad command syntax!\n");
		}
		else if (streq(name, "vhosts")) {
			for (int i = 1; i < argc; i++) {
				printf("Name: \"%s\" Root dir: \"%s\" Error file: \"%s\"\n",
				        vhosts[i-1][vh_user],
				        vhosts[i-1][vh_path],
				        vhosts[i-1][vh_error]);
			}
		}
		else if (streq(name, "help")) {
			printf("help\tPrints this message\nvhosts\tPrints all registered virtual hosts\n");
		}
		else {
			printf("Unknown command %s!\n", name);
		}

		// Get line and divided it into command name and arguments
		fgets(line, 256, stdin);
		argsAssigned = sscanf(line, COMMAND_FORMAT, name);
	}

	/*
	 * Upon termination
	 */

	printf("Exiting...\n");
	kill(pid_connections, SIGTERM);
	while(wait(NULL) > 0);

	freeVhosts(vhosts, argc);
}
