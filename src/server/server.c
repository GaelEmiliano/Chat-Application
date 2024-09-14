#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include "cJSON.h"

#define STD_PORT 8080

#define PORT_ERROR -1
#define SOCKET_ERROR -2
#define BIND_ERROR -3
#define LISTEN_ERROR -4

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;
} connection_t;

void * process(void * ptr)
{
    char * buffer;
    int len;
    connection_t *conn;
    long addr = 0;

    if (!ptr)
        pthread_exit(0);
    
    conn = (connection_t *)ptr;

    /* read length of the message */
    read(conn->sock, &len, sizeof(int));
    if (len > 0)
    {
        addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
        buffer = (char *)malloc((len + 1) * sizeof(char));
        buffer[len] = 0;

        /* read the message */
        read(conn->sock, buffer, len);

        /* Parsear el mensaje JSON recibido */
        cJSON *json_message = cJSON_Parse(buffer);
        if (json_message == NULL)
        {
            printf("error: error parsing client message\n");
        }
        else
        {
            cJSON *type = cJSON_GetObjectItem(json_message, "type");
            cJSON *content = cJSON_GetObjectItem(json_message, "content");

            if (type && content)
            {
                printf("message from: %d.%d.%d.%d: Type: %s, Content: %s\n",
                       (int)((addr) &0xff),
                       (int)((addr >> 8) &0xff),
                       (int)((addr >> 16) &0xff),
                       (int)((addr >> 24) &0xff),
                       type->valuestring, content->valuestring);

                /* create an answear message */
                cJSON *response = cJSON_CreateObject();
                cJSON_AddStringToObject(response, "type", "response");
                cJSON_AddStringToObject(response, "content", "received successfully");

                /* convert JSON response to string */
                char *json_response = cJSON_Print(response);
                int response_len = strlen(json_response);

                /* send response to the client */
                write(conn->sock, &response_len, sizeof(int));
                write(conn->sock, json_response, response_len);

                /* clear used memory */
                cJSON_Delete(response);
                free(json_response);
            }
            else
            {
                printf("incomplete json message\n");
            }

            /* clear memory used by JSON */
            cJSON_Delete(json_message);
        }

        free(buffer);
    }

    /* clear and close socket */
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char ** argv)
{
    int sock = -1;
    struct sockaddr_in address;
    int port = STD_PORT;
    connection_t *connection;
    pthread_t thread;

    /* verificar los argumentos de la línea de comandos */
    if (argc == 2)
    {
        if (sscanf(argv[1], "%d", &port) <= 0)
        {                            
            fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
            return PORT_ERROR;
        }
    }
    else
    {
        printf("No port specified, standard port will be used: %d\n",
               port);
    }

    /* create TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0)
    {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return SOCKET_ERROR;
    }

    /* bind socket to the port */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
        return BIND_ERROR;
    }

    /* listen to port */
    if (listen(sock, 5) < 0)
    {
        fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
        return LISTEN_ERROR;
    }

    printf("%s: ready and listening\n", argv[0]);

    while (1)
    {
        /* accept connections */
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start new thread for conection */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }
    }

    return 0;
}
