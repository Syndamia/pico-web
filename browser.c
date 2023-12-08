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

struct md_syntax {
	regex_t anchor;
};

void renderPage(const sds page, const struct md_syntax* syntax, int* *matches, int *matchesCount) {
	sds toPrint = sdsdup(page);
	
	/*
	 * Parse Markdown constructs
	 */

	/* Substitute and store anchors */
	toPrint = gsub_getm(toPrint, &syntax->anchor, "\033[4m\1\033[0m\16", matches, matchesCount);
	
	sds newPrint;
	for (int i = 0, anchorInd = 0; i < *matchesCount; i++) {
		anchorInd = strchr(toPrint, '\16') - toPrint;

		/* In toPrint, replace '\16' with "\033[30;46m%d\033[0m", where %d is the variable i */
		toPrint[anchorInd] = '\0';
		newPrint = sdsgrowzero(sdsempty(), sdslen(toPrint) + digits(i) + 8 + 4);
		sprintf(newPrint, "%s\033[30;46m%d\033[0m%s", toPrint, i, toPrint + anchorInd + 1);

		sdsfree(toPrint);
		toPrint = newPrint;
	}

	/*
	 * Print page on stdout
	 */
	write(1, toPrint, sdslen(toPrint));

	sdsfree(toPrint);
}

int main(int argc, char* argv[]) {
	/*
	 * Compile regexes used in rendering
	 */

	struct md_syntax md = {
		.anchor = NULL,
	};
	herr(regcomp(&md.anchor, "\\[\\([^]]*\\)\\](\\([^)]*\\))", 0), "regcomp");

	/*
	 * Server-client communication
	 */

	sds page;
	sds address = sdsnew(argv[1]);

	int* anchorIndecies = NULL;
	int anchorCount = 0;

	int gotoIndex = 0;
	int count = 0;

	while (count < 2) {
		/*
		 * Get the page
		 */

		page = get_page("127.0.0.1", "8080", address);
		renderPage(page, &md, &anchorIndecies, &anchorCount);

		/*
		 * Handle user input
		 */

		scanf("%d", &gotoIndex);

		if (gotoIndex < anchorCount) {
			char* newplace = strchr(page + anchorIndecies[gotoIndex], '(') + 1;
			sdsfree(address);
			address = sdscatlen(sdsnew(argv[1]), newplace, strchr(newplace, ')') - newplace);

			free(anchorIndecies);
			anchorCount = 0;
		}
		else {
			printf("error\n");
		}

		sdsfree(page);
		count++;
	}

	regfree(&md.anchor);
	free(anchorIndecies);
	sdsfree(address);
}
