#include <server-connection.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>

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
	memset(address, 0, 256);

	read(fd_client, address, 256);
	printf("[%s@%d] Requested %s\n", client, fd_client, address);

	/* Does vhosts contain an address with the username? */
	int usernameLen = strchr(address, '@') - address;

	const sds *vhost = NULL;
	for (int i = 0; i < vhostsc; i++) {
		if (strncmp(vhosts[i][vh_user], address, usernameLen) == 0) {
			vhost = *vhosts + i;
			break;
		}
	}

	if (vhost == NULL) {
		fprintf(stderr, "[%s@%d] Unknown username in address %s\n", client, fd_client, address);
		return;
	}

	/* Try to open the requested file or the error file */
	sds filePath = constructFilePath(vhost[vh_path], address + usernameLen + 1);

	int fd = open(filePath, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "[%s@%d] Error opening %s\n", client, fd_client, filePath);

		sdsfree(filePath);
		filePath = constructFilePath(vhost[vh_path], vhost[vh_error]);
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
