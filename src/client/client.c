#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#define USAGE_ERROR -1
#define SOCKET_ERROR -2

int main(int argc, char ** argv)
{
    int port;
    int sock = -1;
    struct sockaddr_in address;
    struct hostent *host;
    int len;
    char buffer[1024]; // save server answear
    
    /* checking commandline parameter */
    if (argc != 4)
    {
        printf("usage: %s | hostname | port | text\n", argv[0]);
        return USAGE_ERROR;
    }

    /* obtain port number */
    if (sscanf(argv[2], "%d", &port) <= 0)
    {
        fprintf(stderr, "%s: error: wrong argument: port\n", argv[0]);
        return USAGE_ERROR;
    }

    /* create socket */
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
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)))
    {
        fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
        return HOST_NOT_FOUND;
    }

    /* new object JSON */
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "message");
    cJSON_AddStringToObject(root, "content", argv[3]);

    /* convert JSON to string */
    char *json_string = cJSON_Print(root);
    len = strlen(json_string);

    /* send JSON to server */
    write(sock, &len, sizeof(int));  
    write(sock, json_string, len);   // send JSON

    /* clear memory used by cJSON */
    cJSON_Delete(root);
    free(json_string);

    /* read server response */
    read(sock, &len, sizeof(int));      
    read(sock, buffer, len);               
    buffer[len] = '\0';

    /* parse JSON response */
    cJSON *response = cJSON_Parse(buffer);
    if (response == NULL)
    {
        printf("error: error parsing server answear\n");
    }
    else
    {
        cJSON *response_type = cJSON_GetObjectItem(response, "type");
        cJSON *response_content = cJSON_GetObjectItem(response, "content");

        if (response_type && response_content)
        {
            printf("server response: Type: %s, Content: %s\n",
                   response_type->valuestring, response_content->valuestring);
        }
        else
        {
            printf("invalid JSON response\n");
        }

        /* clear memory */
        cJSON_Delete(response);
    }

    /* close socket */
    close(sock);

    return 0;
}
