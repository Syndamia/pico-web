#include <server-connection.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include <util.h>

sds constructFilePath(const sds root, const char* file) {
	sds path = sdsdup(root);
	if (root[sdslen(root)-1] != '/' && file[0] != '/')
		path = sdscat(path, "/");
	path = sdscat(path, file);
	if (file[strlen(file)-1] == '/')
		path = sdscat(path, "index.md");
	return path;
}

void sanitizeAddress(char* address) {
	/* Remove host and port */
	char* startPath = strchr(address, '/');
	if (startPath == NULL)
		startPath = strchr(address, '\0');

	char* startHost = strchr(address, '@');
	shiftLeft(startHost + 1, address - startHost, startPath - startHost - 1);

	/* Remove ../ */
	for (char* prev = startHost+1, *i = startHost+1; i != NULL && *i != '\0';) {
		if (i[1] == '.' && i[2] == '.' && i[3] == '/') {
			shiftLeft(prev, strlen(prev), i - prev + 3);
			i = prev;
		}
		else {
			prev = i;
			i = strchr(i+1, '/');
		}
	}
}

int openError(sds* filePath, int* fd, const sds* vhost, const char* client, const int fd_client) {
	sdsfree(*filePath);
	*filePath = constructFilePath(vhost[vh_path], vhost[vh_error]);
	*fd = open(*filePath, O_RDONLY);
	if (*fd < 0) {
		fprintf(stderr, "[%s@%d] Error opening %s\n", client, fd_client, *filePath);
		sdsfree(*filePath);
		return 1;
	}
	return 0;
}

void on_connection(const char* client, const int fd_client, sds **vhosts, const int vhostsc) {
	printf("[%s@%d] Connected successfully!\n", client, fd_client);

	/* Get address request */
	char address[256];
	memset(address, 0, 256);

	read(fd_client, address, 256);
	sanitizeAddress(address);
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

	int fd = 0;

	/* Check if file is directory */
	struct stat buf;
	if (stat(filePath, &buf) == 0) {
		if (S_ISDIR(buf.st_mode)) {
			filePath = sdscat(filePath, "/index.md");
		}
		else if (!(S_ISREG(buf.st_mode))) {
			fprintf(stderr, "[%s@%d] %s is not a regular file!\n", client, fd_client, filePath);

			if (openError(&filePath, &fd, vhost, client, fd_client))
				return;
		}
	}

	if (fd <= 0)
		fd = open(filePath, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "[%s@%d] Error opening %s\n", client, fd_client, filePath);

		if (openError(&filePath, &fd, vhost, client, fd_client))
			return;
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
