/* Receives a markdown file and "renders" it
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "sds/sds.h"
#include "util.h"
#include "browser-cli.h"

#define READ_BUFFER_SIZE 512

sds get_page(const char* ip, const char* port, const char* URL) {
	if (streq(URL, "blank")) return sdsnew("\n");

	/*
	 * Create socket for connecting with server
	 */
	int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	herrc(fd_socket, "socket");

	int aton_status = 0;
	struct sockaddr_in sa_server = {
		.sin_family = AF_INET,
		.sin_port   = atop(port),
		.sin_addr   = aton(ip, &aton_status),
	};
	herrc(aton_status, "inet_aton");

	/*
	 * Request page
	 */

	int connectStatus = connect(fd_socket, (struct sockaddr*)&sa_server, sizeof(struct sockaddr_in));
	herrc(connectStatus, "connect");
	if (connectStatus < 0) return sdsnew("Couldn't connect to server!\n");

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

void run(int argc, char* argv[]) {
	initRendering();

	/*
	 * Server-client communication
	 */

	sds page;
	sds host = sdsnew("127.0.0.1");
	sds port = sdsnew("8080");
	sds uri  = sdsnew("blank");

	int stopProgram = 0;
	while (!stopProgram) {
		/*
		 * Get the page
		 */

		printf("\033[30;107m%s\033[0m\n", uri);
		page = get_page(host, port, uri);
		renderPage(page);

		/*
		 * Handle user input
		 */
		stopProgram = handleBrowserCLI(&host, &port, &uri, page);
		sdsfree(page);
	}

	freeRendering();
	sdsfree(host);
	sdsfree(port);
	sdsfree(uri);
}
