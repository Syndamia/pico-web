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
#include <regex.h>
#include <util.h>

int create_socket(const char* ip, const char* port) {
	/*
	 * Create socket for connecting with server
	 */
	int fd_socket;
	herr(fd_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");

	struct sockaddr_in sa_server = {
		.sin_family = AF_INET,
		.sin_port = inet_atop(port),
	};
	herr(inet_aton(ip, &sa_server.sin_addr.s_addr), "inet_aton");

	herr(connect(fd_socket, (struct sockaddr*)&sa_server, sizeof(struct sockaddr_in)), "connect");

	return fd_socket;
}

sds get_page(const int fd_socket, const char* URL) {
	write(fd_socket, URL, strlen(URL));

	sds page = sdsempty();

	char buff[512];
	memset(buff, 0, 512);
	while (read(fd_socket, buff, 512)) {
		page = sdscat(page, buff);
		memset(buff, 0, 512);
	}

	return page;
}

struct md_syntax {
	regex_t anchor;
};

void renderPage(const sds page, const struct md_syntax* syntax, int* *matches, int *matchesCount) {
	sds toPrint = sdsdup(page);

	/* Substitute and register anchors */
	toPrint = gsub_getm(toPrint, &syntax->anchor, "\033[4m\1\033[0m\16", matches, matchesCount);
	
	int anchorInd = 0;
	sds newPrint;
	for (int i = 0; i < *matchesCount; i++) {
		anchorInd = strchr(toPrint, '\16') - toPrint;

		toPrint[anchorInd] = '\0';
		newPrint = sdsgrowzero(sdsempty(), sdslen(toPrint) + digits(i) + 8 + 4);
		sprintf(newPrint, "%s\033[30;46m%d\033[0m%s", toPrint, i, toPrint + anchorInd + 1);

		sdsfree(toPrint);
		toPrint = newPrint;
	}

	write(1, toPrint, sdslen(toPrint));
	sdsfree(toPrint);
}

int main(int argc, char* argv[]) {
	/*
	 * Preparation for rendering
	 */

	struct md_syntax md = {
		.anchor = NULL,
	};
	herr(regcomp(&md.anchor, "\\[\\([^]]*\\)\\](\\([^)]*\\))", 0), "regcomp");

	/*
	 * Server-client communication
	 */

	sds address = sdsnew(argv[1]);
	int count = 0;
	sds page;
	int* anchorIndecies = NULL;
	int anchorCount = 0;

	int gotoIndex = 0;
	int fd_socket;
	while (count < 2) {
		fd_socket = create_socket("127.0.0.1", "8080");
		page = get_page(fd_socket, address);
		free(anchorIndecies);
		anchorCount = 0;
		renderPage(page, &md, &anchorIndecies, &anchorCount);

		scanf("%d", &gotoIndex);
		sdsfree(address);
		char* newplace = strchr(page + anchorIndecies[gotoIndex], '(') + 1;
		address = sdscatlen(sdsnew(argv[1]), newplace, strchr(newplace, ')') - newplace);
		count++;

		close(fd_socket);
	}

	sdsfree(page);
	sdsfree(address);
	free(anchorIndecies);
	regfree(&md.anchor);
}
