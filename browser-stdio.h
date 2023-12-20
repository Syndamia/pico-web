#ifndef BROWSER_STDIO
#define BROWSER_STDIO

#include <sds/sds.h>

void initRendering();
void freeRendering();

void renderPage(const sds page);
int handleCLI(sds authority, sds *address, const sds page);

#endif
