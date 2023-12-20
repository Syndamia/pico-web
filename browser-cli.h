#ifndef BROWSER_CLI
#define BROWSER_CLI

#include <sds/sds.h>

void initRendering();
void freeRendering();

void renderPage(const sds page);
int handleCLI(sds authority, sds *address, const sds page);

#endif
