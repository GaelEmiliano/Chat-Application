#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#define USAGE_ERROR 1
#define SOCKET_ERROR 2
#define JSON_OBJECT_ERROR 3
#define SEND_ERROR 4
#define RECEIVE_ERROR 5

int main(int argc, char ** argv)
{
    int port;
    int sock = -1;
    struct sockaddr_in address;
    struct hostent *host;
    int len;
    char buffer[1024]; // save server response
    
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

    /* create object JSON */
    cJSON * root = cJSON_CreateObject();
    if (!root)
    {
        fprintf(stderr, "%s: error: cannot create JSON object\n", argv[0]);
        close(sock);
        return JSON_OBJECT_ERROR;
    }
    
    cJSON_AddStringToObject(root, "type", "message");
    cJSON_AddStringToObject(root, "content", argv[3]);

    /* convert JSON to string */
    char * json_string = cJSON_Print(root);
    if (!json_string)
    {
        fprintf(stderr, "%s: error: cannot serialize JSON object\n", argv[0]);
        cJSON_Delete(root);
        close(sock);
        return JSON_OBJECT_ERROR;
    }
    
    len = strlen(json_string);

    /* send JSON message length to server */
    if (write(sock, &len, sizeof(int)) != sizeof(int))
    {
        fprintf(stderr, "%s: error: failed to send message length\n", argv[0]);
        free(json_string);
        cJSON_Delete(root);
        close(sock);
        return SEND_ERROR;
    }

    /* send JSON message to server */    
    if (write(sock, json_string, len) != len)
    {
        fprintf(stderr, "%s: error: failed to send message\n", argv[0]);
        free(json_string);
        cJSON_Delete(root);
        close(sock);
        return SEND_ERROR;
    }

    /* clear memory used by cJSON */
    free(json_string);
    cJSON_Delete(root);

    /* read server response length */
    if (read(sock, &len, sizeof(int)) <= 0)
    {
        fprintf(stderr, "%s: error: failed to read response length\n", argv[0]);
        close(sock);
        return RECEIVE_ERROR;
    }

    /* read server response */
    if (read(sock, buffer, len) <= 0)
    {
        fprintf(stderr, "%s: error: failed to read response\n", argv[0]);
        close(sock);
        return RECEIVE_ERROR;
    }
    
    buffer[len] = '\0'; // end received line

    /* parse JSON response */
    cJSON *response = cJSON_Parse(buffer);
    if (response == NULL)
        printf("error: error parsing server response\n");
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
