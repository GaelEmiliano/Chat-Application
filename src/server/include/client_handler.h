#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "cJSON.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <pthread.h>

/*
  max number of users and length name
 */
#define MAX_USERS 100
#define USERNAME_MAX_LEN 8

/*
  definition of user structure
 */
typedef struct
{
    char username[USERNAME_MAX_LEN + 1]; // 8 chars + null character
    int socket;
} user_t;

/*
  functions declarations
 */
int username_exists(const char * username);
void broadcast_new_user(const char * username, int sender_sock);
void add_user(const char * username, int sock);

/*
  deinition of connection structure
 */
typedef struct
{
    int sock;
    struct sockaddr address;
    socklen_t addr_len;
} connection_t;

/*
  handle identification
 */
void handle_identify(connection_t * conn, cJSON * json_message);

#endif // CLIENT_HANDLER_H
