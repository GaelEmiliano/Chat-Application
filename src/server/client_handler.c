#include "client_handler.h"

/* structure of the users and global variables */
user_t users[MAX_USERS];
int num_users = 0;
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

/* check if name does exists */
int username_exists(const char * username)
{
    pthread_mutex_lock(&user_mutex);
    for (int i = 0; i < num_users; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            pthread_mutex_unlock(&user_mutex);
            return 1; // name of the user exists
        }
    }
    pthread_mutex_unlock(&user_mutex);
    return 0; // name of the user does not exists
}

/* broadcast the message to all clients except the sender */
void broadcast_new_user(const char * username, int sender_sock)
{
    cJSON * new_user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(new_user_msg, "type", "NEW_USER");
    cJSON_AddStringToObject(new_user_msg, "username", username);

    char * json_msg = cJSON_Print(new_user_msg);
    int msg_len = strlen(json_msg) + 1;

    pthread_mutex_lock(&user_mutex);
    for (int i = 0; i < num_users; i++)
    {
        if (users[i].socket != sender_sock)
        {
            write(users[i].socket, &msg_len, sizeof(int));
            write(users[i].socket, json_msg, msg_len);
        }
    }
    pthread_mutex_unlock(&user_mutex);

    cJSON_Delete(new_user_msg);
    free(json_msg);
}

/* add user to the list */
void add_user(const char * username, int sock)
{
    pthread_mutex_lock(&user_mutex);
    strncpy(users[num_users].username, username, USERNAME_MAX_LEN);
    users[num_users].username[USERNAME_MAX_LEN] = '\0';
    users[num_users].socket = sock;
    num_users++;
    pthread_mutex_unlock(&user_mutex);
}

/* handles identification of the client */
void handle_identify(connection_t * conn, cJSON * json_message)
{
    cJSON * username = cJSON_GetObjectItem(json_message, "username");
    if (username && username->valuestring)
    {
        /* check if the user already exists */
        if (username_exists(username->valuestring))
        {
            /* response USER_ALREADY_EXISTS */
            cJSON * response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "type", "RESPONSE");
            cJSON_AddStringToObject(response, "request", "IDENTIFY");
            cJSON_AddStringToObject(response, "result", "USER_ALREADY_EXISTS");
            cJSON_AddStringToObject(response, "extra", username->valuestring);
            
            /* convert JSON response to string */
            char * json_response = cJSON_Print(response);
            int response_len = strlen(json_response) + 5;
            
            /* send response to the client */
            if (write(conn->sock, &response_len, sizeof(int)) <= 0 ||
                write(conn->sock, json_response, response_len) <= 0)
                printf("error: sending response to client failed\n");
            
            /* clear used memory */
            cJSON_Delete(response);
            free(json_response);
        }
        else
        {
            /* add user to the list */
            add_user(username->valuestring, conn->sock);
            
            /* response SUCCESS */
            cJSON * response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "type", "RESPONSE");
            cJSON_AddStringToObject(response, "request", "IDENTIFY");
            cJSON_AddStringToObject(response, "result", "SUCCESS");
            cJSON_AddStringToObject(response, "extra", username->valuestring);

            /* convert JSON response to string */
            char * json_response = cJSON_Print(response);
            int response_len = strlen(json_response) + 5;

            /* send response to the client */
            if (write(conn->sock, &response_len, sizeof(int)) <= 0 ||
                write(conn->sock, json_response, response_len) <= 0)
                printf("error: sending response to client failed\n");
            
            /* clear used memory */
            cJSON_Delete(response);
            free(json_response);

            /* broadcast another users */
            broadcast_new_user(username->valuestring, conn->sock);
        }
    }
    else
        printf("invalid IDENTIFY message: no username found\n");
}
