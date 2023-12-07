#include <util.h>
#include <arpa/inet.h>
#include <stdlib.h>

uint16_t inet_atop(const char *port) {
	return htons(atoi(port));
}
