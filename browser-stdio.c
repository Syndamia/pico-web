#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <sds/sds.h>
#include <regex.h>
#include <util.h>

struct md_syntax {
	regex_t anchor;
};

struct md_syntax syntax = {
	.anchor = NULL,
};

int* anchorsIndecies;
int anchorsCount = 0;

void initRendering() {
	/*
	 * Compile regexes used in rendering
	 */

	 herr(regcomp(&syntax.anchor, "\\[\\([^]]*\\)\\](\\([^)]*\\))", 0), "regcomp");
}

void freeRendering() {
	regfree(&syntax.anchor);

	free(anchorsIndecies);
}

void renderPage(const sds page) {
	if (sdslen(page) == 0) {
		printf("Server didn't return page!\n");
		return;
	}

	sds toPrint = sdsdup(page);
	
	/*
	 * Parse Markdown constructs
	 */

	/* Substitute and store anchorsIndecies */
	if (anchorsIndecies != NULL) {
		free(anchorsIndecies);
		anchorsCount = 0;
		anchorsIndecies = NULL;
	}
	toPrint = gsub_getm(toPrint, &syntax.anchor, "\033[4m\1\033[0m\16", &anchorsIndecies, &anchorsCount);
	
	sds newPrint;
	for (int i = 0, anchorInd = 0; i < anchorsCount; i++) {
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

int handleCLI(sds authority, sds *address, const sds page) {
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

			if (gotoIndex < 0 || gotoIndex >= anchorsCount) {
				printf("Invalid anchor index!\n");
				return 0;
			}

			char* newplace = strchr(page + anchorsIndecies[gotoIndex], '(') + 1;
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

	if (streq(name, "q") || streq(name, "e") || streq(name, "quit") || streq(name, "exit")) {
		return 1;
	}

	printf("Invalid command %s!\n", name);
	return 0;
}
