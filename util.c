#include <util.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <regex.h>

/*
 * Networking
 */

uint16_t atop(const char *port) {
	return htons(atoi(port));
}

struct in_addr aton(const char* cp, int* output) {
	struct in_addr inp;
	*output = inet_aton(cp, &inp);
	return inp;
}

/*
 * Error handling
 */

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

/*
 * sds string substitution
 */

sds getMatch(const sds str, const regmatch_t match) {
	return sdsnewlen(str + match.rm_so, match.rm_eo - match.rm_so);
}
int validMatch(const regmatch_t match) {
	return match.rm_so > -1;
}

void resizeMatches(int* *matches, int* size) {
	if (*size == 0) {
		*matches = malloc((*size = 8) * sizeof(int*));
		return;
	}

	int* *biggerArr = malloc((*size * 2) * sizeof(int*));
	for (size_t i = 0; i < *size; i++) {
		biggerArr[i] = matches[i];
	}
	*size *= 2;
	free(matches);
	*matches = *biggerArr;
}
void pushBackMatch(int* *matches, int *matchesCount, int *matchesSize, int matchStart) {
	if (*matchesCount >= *matchesSize)
		resizeMatches(matches, matchesSize);

	(*matches)[*matchesCount] = matchStart;
	*matchesCount += 1;
}

#define MATCHSTART str + strInd
sds gsub_getm(sds str, const regex_t *regex, const char* repl, int* *matches, int *matchesCount) {
	regmatch_t pmatch[10] = {
		{ .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, },
		{ .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, },
		{ .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, }, { .rm_so = 0, .rm_eo = 0, },
		{ .rm_so = 0, .rm_eo = 0, },
	};

	int strInd = 0;
	int matchesSize = (matchesCount != NULL) ? *matchesCount : 0;
	size_t replLen = strlen(repl);

	sds ret = sdsempty();
	/*
	 * Substitute all occurences of regex with repl in str
	 */
	// sdslen is in O(1) time
	while (strInd < sdslen(str)) {
		/* Run regex */
		if (regexec(regex, MATCHSTART, 10, pmatch, 0) != 0) {
			/* If there are no matches, return the rest of the string as-is */
			ret = sdscat(ret, MATCHSTART);
			break;
		}

		/* Store everything before the match as-is */
		ret = sdscatlen(ret, MATCHSTART, pmatch[0].rm_so);

		/* Replace match with repl
		 * repl can include matched subexpressions */
		for(size_t i = 0; i < replLen; i++) {
			if (repl[i] <= '\10') {
			if (pmatch[repl[i] % 10].rm_so > -1)
				ret = sdscatsds(ret, getMatch(MATCHSTART, pmatch[repl[i] % 10]));
			}
			else
				ret = sdscatlen(ret, &repl[i], 1);
		}

		/* Add index of current match to matches */
		if (matchesCount != NULL) {
			pushBackMatch(matches, matchesCount, &matchesSize, strInd + pmatch[0].rm_so);
		}

		/* Continute after the current match */
		strInd += pmatch[0].rm_eo;
	}

	sdsfree(str);
	return ret;
}

sds gsub(sds str, const regex_t* regex, const char* repl) {
	return gsub_getm(str, regex, repl, NULL, NULL);
}

/*
 * other
 */

int digits(int num) {
	if (num < 0) num *= -1;
	// This is the fastest way to get the number of digits
	if (num < 10) return 1;
	if (num < 100) return 2;
	if (num < 1000) return 3;
	if (num < 10000) return 4;
	if (num < 100000) return 5;
	if (num < 1000000) return 6;
	if (num < 10000000) return 7;
	if (num < 100000000) return 8;
	if (num < 1000000000) return 9;
	//        2147483647 (2^31-1) is max value of int
	return 10;
}
