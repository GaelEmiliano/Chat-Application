#include "client.h"
#include "cJSON.h"

/*
  define constants to improve readability
*/
#define BUFFER_SIZE 1024
#define USAGE_ERROR 1
#define SOCKET_ERROR 2

int main(int argc, char ** argv)
{
    int port;
    int sock = -1;
    struct sockaddr_in address;
    struct hostent * host;
    
    /* checking commandline parameter */
    if (argc != 3)
    {
        printf("usage: %s | hostname | port\n", argv[0]);
        return USAGE_ERROR;
    }

    /* obtain port number */
    if (sscanf(argv[2], "%d", &port) <= 0)
    {
        fprintf(stderr, "%s: error: wrong argument: port\n", argv[0]);
        return USAGE_ERROR;
    }

    /* create TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0)
    {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return SOCKET_ERROR;
    }

    /* connect to server */
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    host = gethostbyname(argv[1]);
    if (!host)
    {
        fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
        return HOST_NOT_FOUND;
    }
    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
        return HOST_NOT_FOUND;
    }

    /* request username from client */
    char username[USERNAME_MAX_LEN + 1];
    printf("Enter your username (max %d characters): ", USERNAME_MAX_LEN);
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // remove new line

    /* truncate username if it exceeds 8 characters */
    if (strlen(username) > USERNAME_MAX_LEN)
    {
        username[USERNAME_MAX_LEN] = '\0';
        printf("username too long, truncated to: %s\n", username);
    }

    /* call function to make user identify */
    identify_user(sock, username);

    /* wait for actions */
    while (1)
    {
        sleep(1); // mantain an active client
    }

    /* close socket */
    close(sock);

    return 0;
}
