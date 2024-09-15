#include "client.h"
#include "cJSON.h"

void identify_user(int socket, const char * username)
{
    /* create IDENTIFY JSON object */
    cJSON * root = cJSON_CreateObject();
    if (!root)
    {
        printf("error: cannot create JSON object\n");
        close(socket);
        return;
    }
    
    cJSON_AddStringToObject(root, "type", "IDENTIFY");
    cJSON_AddStringToObject(root, "username", username);

    /* convert JSON to string */
    char * json_string = cJSON_Print(root);
    if (!json_string)
    {
        printf("error: cannot serialize JSON object\n");
        cJSON_Delete(root);
        close(socket);
        return;
    }
    
    int len = strlen(json_string) + 1; // +1 for the null character

    /* send JSON message length to server */
    if (write(socket, &len, sizeof(int)) != sizeof(int))
    {
        printf("error: failed to send message length\n");
        free(json_string);
        cJSON_Delete(root);
        close(socket);
        return;
    }

    /* send JSON message to server */    
    if (write(socket, json_string, len) != len)
    {
        printf("error: failed to send message\n");
        free(json_string);
        cJSON_Delete(root);
        close(socket);
        return;
    }

    /* clear memory used by cJSON */
    free(json_string);
    cJSON_Delete(root);

    /* read server response length */
    if (read(socket, &len, sizeof(int)) <= 0)
    {
        printf("error: failed to read response length\n");
        close(socket);
        return;
    }
    
    char * buffer = (char *)malloc(len);
    if (buffer == NULL)
    {
        perror("error: cannot allocate memory for response");
        close(socket);
        return;
    }

     /* read server response */
    if (read(socket, buffer, len) <= 0)
    {
        printf("error: failed to read response\n");
        close(socket);
        return;
    }

    /* parse JSON message */
    cJSON * response = cJSON_Parse(buffer);
    if (response == NULL)
        printf("error: failed to parse server response\n");
    else
    {
     cJSON * type = cJSON_GetObjectItem(response, "type");
     cJSON * request = cJSON_GetObjectItem(response, "request");
     cJSON * result = cJSON_GetObjectItem(response, "result");
     cJSON * extra = cJSON_GetObjectItem(response, "extra");

     /* check if all exists */
     if (type && request && result && extra)
     {
         printf("Server response: type: %s, request: %s, result: %s, extra: %s\n",
                type->valuestring,
                request->valuestring,
                result->valuestring,
                extra->valuestring);
     }
     else
         printf("invalid JSON response\n");
     
     /* clear memory */
     cJSON_Delete(response);   
    }

    free(buffer);
}
