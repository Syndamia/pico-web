#ifndef H_SERVER_CONNECTION
#define H_SERVER_CONNECTION

#include "sds/sds.h"

#define vh_user  0
#define vh_path  1
#define vh_error 2

void on_connection(const char* client, const int fd_client, sds **vhosts, const int vhostsc);

#endif
