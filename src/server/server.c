#include "client_handler.h"

#define STD_PORT 8080
#define PORT_ERROR 1
#define SOCKET_ERROR 2
#define BIND_ERROR 3
#define LISTEN_ERROR 4

void * process(void * ptr)
{
    char * buffer;
    int len;
    connection_t * conn;
    long addr = 0;

    if (!ptr)
        pthread_exit(0);
    
    conn = (connection_t *)ptr;

    /* read length of the message */
    if (read(conn->sock, &len, sizeof(int)) <= 0)
    {
        printf("error: error reading from socket failed\n");
        close(conn->sock);
        free(conn);
        pthread_exit(0);
    }
    
    if (len > 0)
    {
        addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
        buffer = (char *)malloc((len + 1) * sizeof(char));
        if (buffer == NULL)
        {
            perror("error: cannot allocate memory for buffer\n");
            close(conn->sock);
            free(conn);
            pthread_exit(0);
        }
        buffer[len] = 0;

        /* read the message */
        if (read(conn->sock, buffer, len) <= 0)
        {
            printf("error: reading message failed\n");
            free(buffer);
            close(conn->sock);
            free(conn);
            pthread_exit(0);
        }

        /* parse received message */
        cJSON * json_message = cJSON_Parse(buffer);
        if (json_message == NULL)
            printf("error: error parsing client message\n");
        else
        {
            cJSON * type = cJSON_GetObjectItem(json_message, "type");
            
            if (type && strcmp(type->valuestring, "IDENTIFY") == 0)
                handle_identify(conn, json_message);
            else
                printf("unknown message type\n");
            
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
    connection_t * connection;
    pthread_t thread;

    /* check commandline */
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
               STD_PORT);
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

    printf("%s: ready and listening on port %d\n", argv[0], port);

    while (1)
    {
        /* accept connections */
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->addr_len = sizeof(struct sockaddr_in); // initialize addr_len
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start new thread for the conection */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }
    }

    return 0;
}
