#ifndef H_UTIL
#define H_UTIL

#include <inttypes.h>
#include <sds/sds.h>
#include <arpa/inet.h>
#include <regex.h>

uint16_t atop(const char *port);
struct in_addr aton(const char* cp, int* output);

void herr(int output, const char* funcName);
void herrc(int output, const char* funcName);

sds gsub(sds str, const regex_t* regex, const char* repl);
sds gsub_getm(sds str, const regex_t *regex, const char* repl, int* *matches, int *matchesCount);

int digits(int num);
#define clear_arr(arr) memset(arr, 0, sizeof(arr)/sizeof(*arr))

#endif
