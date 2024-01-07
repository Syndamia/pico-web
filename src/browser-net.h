#ifndef BROWSER_NET
#define BROWSER_NET

#include "sds/sds.h"

sds get_page(const char* ip, const char* port, const char* URL);
void run(int argc, char* argv[]);

#endif
