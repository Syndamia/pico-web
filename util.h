#ifndef H_UTIL
#define H_UTIL

#include <inttypes.h>
#include <sds/sds.h>
#include <regex.h>

uint16_t inet_atop(const char *port);
void herr(int output, const char* funcName);
void herrc(int output, const char* funcName);

sds gsub(sds str, const regex_t* regex, const char* repl);
sds gsub_getm(sds str, const regex_t *regex, const char* repl, int* *matches, int *matchesCount);

int digits(int num);

#endif
