/* The server recieves connections and passes files to the clients
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <string.h>
#include <util.h>

void on_connection(int fd_client) {
	char buff[256];
	read(fd_client, buff, 256);

	int fd;
	herr(fd = open(buff, O_RDONLY), "open");
	memset(buff, 0, sizeof(buff));
	while (read(fd, buff, 256)) {
		write(fd_client, buff, strlen(buff));
		memset(buff, 0, sizeof(buff));
	}
	close(fd);
}

int main(int argc, char* argv[]) {
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
	while (count < 2) {
		herr(fd_client = accept(fd_socket, (struct sockaddr*)&sa_client, &sa_client_size), "accept");
		count++;
		int fp = fork();

		if (fp == 0) {
			close(fd_socket);
			on_connection(fd_client);
			close(fd_client);
			return 0;
		}
		close(fd_client);
	}
	close(fd_socket);

	printf("Exiting");
	while(wait(NULL) > 0);
}
