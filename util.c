#include <util.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

uint16_t inet_atop(const char *port) {
	return htons(atoi(port));
}

void herrc(int output, const char* funcName) {
	if (output < 0 && errno != EINTR) {
		perror(funcName);
	}
}

void herr(int output, const char* funcName) {
	if (output < 0) {
		perror(funcName);
		exit(errno);
	}
}
