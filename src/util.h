#ifndef H_UTIL
#define H_UTIL

#include <inttypes.h>
#include <sds/sds.h>
#include <arpa/inet.h>
#include <regex.h>

/* Networking */
uint16_t atop(const char *port);
struct in_addr aton(const char* cp, int* output);

/* Error handling */
void herr(int output, const char* funcName);
void herrc(int output, const char* funcName);

/* sds string substition */
sds gsub(sds str, const regex_t* regex, const char* repl);
sds gsub_getm(sds str, const regex_t *regex, const char* repl, int* *matches, int *matchesCount);

/* Other */
#define clear_arr(arr) memset(arr, 0, sizeof(arr)/sizeof(*arr))
int digits(int num);
int streq(const char* first, const char* second);
void shiftLeft(char* str, size_t size, size_t shift);
int isNumber(char* str);
int charCount(char* str, char cmp);

#endif
