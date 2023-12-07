/* Receives a markdown file and "renders" it
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

#include <util.h>

int main(int argc, char* argv[]) {
	int fd_socket;
	herr(fd_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");

	struct sockaddr_in sa_server = {
		.sin_family = AF_INET,
		.sin_port = inet_atop("8080"),
	};
	herr(inet_aton("127.0.0.1", &sa_server.sin_addr.s_addr), "inet_aton");

	herr(connect(fd_socket, (struct sockaddr*)&sa_server, sizeof(struct sockaddr_in)), "connect");

	char msg[] = "Hello from browser";
	write(fd_socket, msg, sizeof(msg));

	char buff[256];
	read(fd_socket, buff, 256);
	printf("Received: %s\n", buff);

	close(fd_socket);
}
