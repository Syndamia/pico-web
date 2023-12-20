/* Receives a markdown file and "renders" it
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <sds/sds.h>
#include <util.h>
#include <browser-stdio.h>

#define READ_BUFFER_SIZE 512

sds get_page(const char* ip, const char* port, const char* URL) {
	/*
	 * Create socket for connecting with server
	 */
	int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	herr(fd_socket, "socket");

	int aton_status = 0;
	struct sockaddr_in sa_server = {
		.sin_family = AF_INET,
		.sin_port   = atop(port),
		.sin_addr   = aton(ip, &aton_status),
	};
	herr(aton_status, "inet_aton");

	/*
	 * Request page
	 */

	herr(connect(fd_socket, (struct sockaddr*)&sa_server, sizeof(struct sockaddr_in)), "connect");
	write(fd_socket, URL, strlen(URL));

	/*
	 * Receive page
	 */

	sds page = sdsempty();

	char buff[READ_BUFFER_SIZE];
	clear_arr(buff);
	while (read(fd_socket, buff, READ_BUFFER_SIZE)) {
		page = sdscat(page, buff);
		clear_arr(buff);
	}
	
	/*
	 * Final
	 */

	close(fd_socket);
	return page;
}

int main(int argc, char* argv[]) {
	initRendering();

	/*
	 * Server-client communication
	 */

	sds page;
	sds authority = sdsnew(argv[1]);
	sds address = sdsdup(authority);

	int stopProgram = 0;
	while (!stopProgram) {
		/*
		 * Get the page
		 */

		printf("\033[30;107m%s\033[0m\n", address);
		page = get_page("127.0.0.1", "8080", address);
		renderPage(page);

		/*
		 * Handle user input
		 */
		stopProgram = handleCLI(authority, &address, page);
		sdsfree(page);
	}

	freeRendering();
	sdsfree(address);
	sdsfree(authority);
}
