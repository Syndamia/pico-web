/* Receives a markdown file and "renders" it
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include <sds/sds.h>
#include <util.h>

sds get_page(const int fd_socket) {
	sds page = sdsempty();

	char buff[512];
	memset(buff, 0, 512);
	while (read(fd_socket, buff, 512)) {
		page = sdscat(page, buff);
		memset(buff, 0, 512);
	}

	return page;
}

void renderPage(const sds page) {
	sds toPrint = gsub(page, "\\*[^*]*\\*", "emph");
	write(1, toPrint, sdslen(toPrint));
	sdsfree(toPrint);
}

int main(int argc, char* argv[]) {
	int fd_socket;
	herr(fd_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");

	struct sockaddr_in sa_server = {
		.sin_family = AF_INET,
		.sin_port = inet_atop("8080"),
	};
	herr(inet_aton("127.0.0.1", &sa_server.sin_addr.s_addr), "inet_aton");

	herr(connect(fd_socket, (struct sockaddr*)&sa_server, sizeof(struct sockaddr_in)), "connect");

	/* char msg[] = "hello@/test.txt"; */
	write(fd_socket, argv[1], strlen(argv[1]));

	sds page = get_page(fd_socket);
	renderPage(page);
	sdsfree(page);

	close(fd_socket);
}
