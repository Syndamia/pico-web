#include "util.h"
#include "server-connection.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LEN_COMMAND 16
#define COMMAND_FORMAT ": %16s"

void handleCLI(sds **vhosts, int vhostsc) {
	// Is stdin available
	
	// Get a line
	char line[256];
	if (fgets(line, 256, stdin) == NULL) {
		printf("Couldn't read from standard input! User input can not and will not be handled!\nYou'll have to manually kill the server process to shut it down!\n");
		return;
	}

	// Get command name and it's arguments
	// Currently no command takes arguments
	char name[MAX_LEN_COMMAND+1];
	int argsAssigned = sscanf(line, COMMAND_FORMAT, name); // Flawfinder: ignore

	while (!streq(name, "q") && !streq(name, "e") && !streq(name, "quit") && !streq(name, "exit")) {
		if (argsAssigned < 1) {
			printf("Bad command syntax!\n");
		}
		else if (streq(name, "vhosts")) {
			for (int i = 0; i < vhostsc; i++) {
				printf("Name: \"%s\" Root dir: \"%s\" Error file: \"%s\"\n",
				        vhosts[i][vh_user],
				        vhosts[i][vh_path],
				        vhosts[i][vh_error]);
			}
		}
		else if (streq(name, "help") || streq(name, "h") || streq(name, "?")) {
			printf("help,h,?\tPrints this message\nvhosts\t\tPrints all registered virtual hosts\nquit,exit,q,e\tExits the program\n");
		}
		else {
			printf("Unknown command %s!\n", name);
		}

		// Get line and divided it into command name and arguments
		fgets(line, 256, stdin);
		argsAssigned = sscanf(line, COMMAND_FORMAT, name); // Flawfinder: ignore
	}

	printf("Exiting...\n");
	kill(getppid(), SIGTERM);
}
