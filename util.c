#include <util.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>
#include <regex.h>

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

sds gsub(const sds str, const char* regex, const char* repl) {
	regex_t preg;
	regcomp(&preg, regex, 0);

	int strInd = 0;
	regmatch_t pmatch[1] = {
		{ .rm_so = 0, .rm_eo = 0, },
	};

	sds ret = sdsempty();
	// sdslen is in O(1) time
	while (strInd < sdslen(str)) {
		if (regexec(&preg, str + strInd, 1, pmatch, 0) > 0) {
			ret = sdscat(ret, str + strInd);
			break;
		}
		ret = sdscatlen(ret, str + strInd, pmatch[0].rm_so);
		ret = sdscat(ret, repl);
		strInd += pmatch[0].rm_eo;
	}

	return ret;
}
