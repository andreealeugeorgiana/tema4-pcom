#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params, char **cookies, int cookies_count, char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        strcat(line, " \0");
        strcat(line, cookies[cookies_count - 1]);   
        compute_message(message, line);
    }

    if (tokens != NULL) {
        sprintf(line, "Authorization: Bearer %s", tokens[tokens_count - 1]);
        compute_message(message, line); 
    }

    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count, char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));


    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    strcpy(body_data_buffer, body_data[0]);
    for(int i = 1; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, "&\0");
        strcat(body_data_buffer, body_data[i]);
    }
    
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie:");
        for(int i = 0; i < cookies_count; i++) {
            strcat(line, " \0");
            strcat(line, cookies[i]);
        }
        compute_message(message, line);
    }

    if (tokens != NULL) {
        sprintf(line, "Authorization: Bearer %s", tokens[tokens_count - 1]);
        compute_message(message, line); 
    }
    
    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (tokens != NULL) {
        sprintf(line, "Authorization: Bearer %s", tokens[tokens_count - 1]);
        compute_message(message, line); 
    }
    
    compute_message(message, "");
    return message;
}
