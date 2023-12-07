/* The server recieves connections and passes files to the clients
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

	struct sockaddr_in sa_socket = {
		.sin_family = AF_INET,
		.sin_port = inet_atop("8080"),
	};
	herr(inet_aton("127.0.0.1", &sa_socket.sin_addr.s_addr), "inet_aton");

	herr(bind(fd_socket, (struct sockaddr*)&sa_socket, sizeof(struct sockaddr_in)), "bind");

	herr(listen(fd_socket, 50), "listen");

	struct sockaddr_in sa_client;
	socklen_t sa_client_size = sizeof(struct sockaddr_in);
	int fd_client;
	herr(fd_client = accept(fd_socket, (struct sockaddr*)&sa_client, &sa_client_size), "accept");

	char buff[256];
	read(fd_client, buff, 256);
	printf("Received: %s\n", buff);

	char response[] = "Server received message!";
	write(fd_client, response, sizeof(response));

	close(fd_socket);
}
