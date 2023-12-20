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
	if (sdslen(page) == 0) {
		printf("Server didn't return page!\n");
		return;
	}

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

#define MAX_LEN_COMMAND 16
#define COMMAND_FORMAT ": %16s"

int handleCLI(sds authority, sds *address, const sds page, int* *anchorIndecies, int *anchorCount) {
	// Get a line
	char line[1024];
	fgets(line, 1024, stdin);

	// Nothing
	if (line[0] == '\0') {
		printf("Please enter a valid command!\n");
		return 0;
	}

	// Number or URL
	if (line[0] != ':') {
		// Index of anchor
		if (strchr(line, '/') == NULL) {
			int gotoIndex = 0;
			sscanf(line, "%d", &gotoIndex);

			if (gotoIndex < 0 || gotoIndex >= *anchorCount) {
				printf("Invalid anchor index!\n");
				return 0;
			}

			char* newplace = strchr(page + (*anchorIndecies)[gotoIndex], '(') + 1;
			sdsfree(*address);
			*address = sdscatlen(sdsdup(authority), newplace, strchr(newplace, ')') - newplace);
		}
		// New address
		else {
			sdsfree(*address);
			*address = sdsnewlen(line, strlen(line)-1 /* skip newline */);
		}
		return 0;
	}

	// Special command
	
	// Get command name and it's arguments
	// Currently no command takes arguments
	char name[MAX_LEN_COMMAND+1] = { '\0' };
	int argsAssigned = sscanf(line, COMMAND_FORMAT, name);

	if (streq(name, "quit") || streq(name, "exit")) {
		return 1;
	}

	printf("Invalid command %s!\n", name);
	return 0;
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
	sds authority = sdsnew(argv[1]);
	sds address = sdsdup(authority);

	int* anchorIndecies = NULL;
	int anchorCount = 0;

	int stopProgram = 0;
	while (!stopProgram) {
		/*
		 * Get the page
		 */

		printf("\033[30;107m%s\033[0m\n", address);
		page = get_page("127.0.0.1", "8080", address);
		renderPage(page, &md, &anchorIndecies, &anchorCount);

		/*
		 * Handle user input
		 */
		stopProgram = handleCLI(authority, &address, page, &anchorIndecies, &anchorCount);
		free(anchorIndecies);
		anchorIndecies = NULL;
		anchorCount = 0;
		sdsfree(page);
	}

	regfree(&md.anchor);
	sdsfree(address);
	sdsfree(authority);
}
