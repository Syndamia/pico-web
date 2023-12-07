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
#include <util.h>

struct se_vhost {
	char *username;
	char *path_root;
	char *path_error;
};

char* constructFilePath(const char* root, const char* file) {
	if (root == NULL || file == NULL) return NULL;

	int rootLen = strlen(root), fileLen = strlen(file);

	int rootEndOnSlash = root[rootLen - 1] == '/' || file[0] == '/';
	int fileEndOnSlash = file[fileLen - 1] == '/';

	int pathLen = rootLen + !rootEndOnSlash + fileLen + (fileEndOnSlash * 8) + 1;
	char* path = malloc(pathLen * sizeof(char));
	memset(path, 0, pathLen);
	strncpy(path, root, rootLen);
	if (!rootEndOnSlash) strcat(path, "/");
	strcat(path, file);
	if (fileEndOnSlash)  strcat(path, "index.md");

	return path;
}

void on_connection(const int fd_client, const struct se_vhost *vhosts, const int vhostsc) {
	printf("[%d] Connected successfully!\n", fd_client);

	char address[256];
	read(fd_client, address, 256);
	printf("[%d] Requested %s\n", fd_client, address);

	int usernameLen = strchr(address, '@') - address;

	const struct se_vhost *vhost = NULL;
	for (int i = 0; i < vhostsc; i++) {
		if (strncmp(vhosts[i].username, address, usernameLen) == 0) {
			vhost = vhosts + i;
			break;
		}
	}

	if (vhost == NULL) {
		fprintf(stderr, "[%d] Unknown username in address %s\n", fd_client, address);
		return;
	}

	char* filePath = constructFilePath(vhost->path_root, address + usernameLen + 1);

	int fd = open(filePath, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "[%d] Error opening %s\n", fd_client, filePath);

		free(filePath);
		filePath = constructFilePath(vhost->path_root, vhost->path_error);
		fd = open(filePath, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "[%d] Error opening %s\n", fd_client, filePath);
			free(filePath);
			return;
		}
	}

	printf("[%d] Serving %s\n", fd_client, filePath);
	free(filePath);

	char buff[256];
	memset(buff, 0, sizeof(buff));
	while (read(fd, buff, 256)) {
		write(fd_client, buff, strlen(buff));
		memset(buff, 0, sizeof(buff));
	}
	close(fd);

	printf("[%d] Served!\n", fd_client);
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

	struct se_vhost vhosts[1] = {
	{
		.username = "hello",
		.path_root = ".",
		.path_error = NULL,
	} };

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
			on_connection(fd_client, vhosts, 1);
			close(fd_client);
			return 0;
		}
		close(fd_client);
	}
	close(fd_socket);

	printf("Exiting");
	while(wait(NULL) > 0);
}
