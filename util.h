#ifndef H_UTIL
#define H_UTIL

#include <inttypes.h>
#include <sds/sds.h>

uint16_t inet_atop(const char *port);
void herr(int output, const char* funcName);
void herrc(int output, const char* funcName);
sds gsub(sds str, const char* regex, const char* repl);

#endif
