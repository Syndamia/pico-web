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

#include <string.h>
#include <sds/sds.h>
#include <util.h>

#define username   0
#define path_root  1
#define path_error 2

sds constructFilePath(const sds root, const char* file) {
	sds path = sdsdup(root);
	if (root[sdslen(root)-1] != '/' && file[0] != '/')
		path = sdscat(path, "/");
	path = sdscat(path, file);
	if (file[strlen(file)-1] == '/')
		path = sdscat(path, "index.md");
	return path;
}

void on_connection(const char* client, const int fd_client, sds **vhosts, const int vhostsc) {
	printf("[%s@%d] Connected successfully!\n", client, fd_client);

	/* Get address request */
	char address[256];
	read(fd_client, address, 256);
	printf("[%s@%d] Requested %s\n", client, fd_client, address);

	/* Does vhosts contain an address with the username? */
	int usernameLen = strchr(address, '@') - address;

	const sds *vhost = NULL;
	for (int i = 0; i < vhostsc; i++) {
		if (strncmp(vhosts[i][username], address, usernameLen) == 0) {
			vhost = *vhosts + i;
			break;
		}
	}

	if (vhost == NULL) {
		fprintf(stderr, "[%s@%d] Unknown username in address %s\n", client, fd_client, address);
		return;
	}

	/* Try to open the requested file or the error file */
	sds filePath = constructFilePath(vhost[path_root], address + usernameLen + 1);

	int fd = open(filePath, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "[%s@%d] Error opening %s\n", client, fd_client, filePath);

		sdsfree(filePath);
		filePath = constructFilePath(vhost[path_root], vhost[path_error]);
		fd = open(filePath, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "[%s@%d] Error opening %s\n", client, fd_client, filePath);
			sdsfree(filePath);
			return;
		}
	}

	/* Send the file to the client */
	printf("[%s@%d] Serving %s\n", client, fd_client, filePath);
	sdsfree(filePath);

	char buff[256];
	memset(buff, 0, sizeof(buff));
	while (read(fd, buff, 256)) {
		write(fd_client, buff, strlen(buff));
		memset(buff, 0, sizeof(buff));
	}

	/* Finalize */
	close(fd);
	printf("[%s@%d] Served!\n", client, fd_client);
}


void freeVhosts(sds **vhosts, int argc) {
	for (int i = 1; i < argc; i++) {
		sdsfreesplitres(vhosts[i-1], 3);
	}
	free(vhosts);
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
	herr(fd_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");

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

	/*
	 * Accept connection on the socket
	 */

	struct sockaddr_in sa_client;
	socklen_t sa_client_size = sizeof(struct sockaddr_in);
	int fd_client;
	int count = 0;
	while (count < 5) {
		herr(fd_client = accept(fd_socket, (struct sockaddr*)&sa_client, &sa_client_size), "accept");
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
	close(fd_socket);

	printf("Exiting");
	while(wait(NULL) > 0);

	freeVhosts(vhosts, argc);
}
