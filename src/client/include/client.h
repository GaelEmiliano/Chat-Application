#ifndef CLIENT_H
#define CLIENT_H

#include "cJSON.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define USERNAME_MAX_LEN 8

void identify_user(int socket, const char * username);

#endif // CLIENT_H
