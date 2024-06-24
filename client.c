#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include <stdbool.h> 
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[])
{
    int sockfd;
    char *message;
    char *response;
    char is_logged_in = 0;
    char is_in_library = 0;

    char *cookies[100];
    int cookie_count = 0;

    char *tokens[100];
    int token_count = 0;

    JSON_Value *json_val;
    JSON_Object *json_obj;

    while(1) {
        char command[50];
        scanf("%s", command);

        if(strcmp(command, "register") == 0) {
            if(is_logged_in) {
                printf("EROARE: Sunteti deja logat!\n");
                continue;
            }
            char username[50], password[50];
            printf("username=");
            scanf("%s", username);
            int is_alpha = 1;
            for (int i = 0; i < strlen(username); i++) {
                if (!isalnum(username[i])) {
                    printf("EROARE: Username-ul poate contine doar litere si cifre!\n");
                    is_alpha = 0;
                    continue;
                }
            }
            if (!is_alpha) {
                continue;
            }
            is_alpha = 1;
            printf("password=");
            scanf("%s", password);
            for (int i = 0; i < strlen(password); i++) {
                if (!isalnum(password[i])) {
                    printf("EROARE: Parola poate contine doar litere si cifre!\n");
                    is_alpha = 0;
                    continue;
                }
            }
            if (!is_alpha) {
                continue;
            }

            json_val = json_value_init_object();
            json_obj = json_value_get_object(json_val);
            json_object_set_string(json_obj, "username", username);
            json_object_set_string(json_obj, "password", password);

            char *pretty_message = json_serialize_to_string_pretty(json_val);
            message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/register", "application/json", &pretty_message, 0, NULL, 0, NULL, 0);
            // printf("%s\n", pretty_message);
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (!strstr(response, "error")) {
                printf("SUCCES: Utilizator înregistrat cu succes!\n");
                close_connection(sockfd);
                continue;
            }
            printf("%s\n", "EROARE: Username-ul exista deja in baza de date!");
            close_connection(sockfd);
            continue;
        } else if (strcmp(command, "login") == 0) {
            if(is_logged_in) {
                printf("EROARE: Sunteti deja logat!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char username[50], password[50];
            printf("username=");
            scanf("%s", username);
            int is_alpha = 1;
            for (int i = 0; i < strlen(username); i++) {
                if (!isalnum(username[i])) {
                    printf("EROARE: Username-ul poate contine doar litere si cifre!\n");
                    is_alpha = 0;
                    continue;
                }
            }
            if (!is_alpha) {
                continue;
            }
            is_alpha = 1;
            printf("password=");
            scanf("%s", password);
            for (int i = 0; i < strlen(password); i++) {
                if (!isalnum(password[i])) {
                    printf("EROARE: Parola poate contine doar litere si cifre!\n");
                    is_alpha = 0;
                    continue;
                }
            }
            if (!is_alpha) {
                continue;
            }
            json_val = json_value_init_object();
            json_obj = json_value_get_object(json_val);
            json_object_set_string(json_obj, "username", username);
            json_object_set_string(json_obj, "password", password);

            char *pretty_message = json_serialize_to_string_pretty(json_val);
            message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/login", "application/json", &pretty_message, 0, NULL, 0, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (strstr(response, "error")) {
                printf("EROARE: Parola sau username gresit!\n");
                close_connection(sockfd);
                continue;
            }
            char *cookie = strtok(strstr(response, "Set-Cookie:"), " ");
            cookie = strtok(NULL, ";");
            cookies[cookie_count] = cookie;
            cookie_count++;
            is_logged_in = 1;
            close_connection(sockfd);
            printf("SUCCES: Utilizatorul a fost logat cu succes\n");
            continue;
        } else if (strcmp(command, "enter_library") == 0) {

            if (!is_logged_in) {
                printf("EROARE: Nu sunteti logat!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/library/access", NULL, cookies, cookie_count, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *json_response = basic_extract_json_response(response);

            if (strstr(json_response, "token")) {
                is_in_library = 1;
                json_val = json_parse_string(json_response);
                json_obj = json_value_get_object(json_val);
                char *token = json_object_get_string(json_obj, "token");
                tokens[token_count++] = strdup(token);
                close_connection(sockfd);
                printf("SUCCES: Utilizatorul are acces la biblioteca\n");
                continue;
            }

            close_connection(sockfd);
            printf("EROARE: Nu s-a putut accesa biblioteca!\n");
            continue;
        } else if (strcmp(command, "get_books") == 0) {
            if(!is_logged_in) {
                printf("ERROR: Nu sunteti logat!\n");
                continue;
            }
            if(!is_in_library) {
                printf("ERROR: Nu aveti acces la biblioteca!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", NULL, NULL, 0, tokens, token_count);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("%s\n", strstr(response, "["));
            char *books = strtok(strstr(response, "["), "");
            JSON_Value* values = json_parse_string(books);
            JSON_Array* array = json_value_get_array(values);
            size_t count = json_array_get_count(array);

            for (int i = 0; i < count; i++) {

                JSON_Object* item = json_array_get_object(array, i);
                int id = json_object_get_number(item, "id");
                const char* title = json_object_get_string(item, "title");
                printf("{\"id\": %d, \"title\": \"%s\"}\n", id, title);
            }

            close_connection(sockfd);
            continue;

        } else if (strcmp(command, "get_book") == 0) {
            if(!is_logged_in) {
                printf("EROARE: Nu sunteti logat!\n");
                continue;
            }
            if(!is_in_library) {
                printf("EROARE: Nu aveti acces la biblioteca!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            printf("id=");
            char id[50];
            scanf("%s", id);
            char new_path[100];
            strcpy(new_path, "/api/v1/tema/library/books/");
            strcat(new_path, id);
            message = compute_get_request("34.246.184.49", new_path, 0, cookies, cookie_count, tokens, token_count);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char* book_data = strstr(response, "{");
            JSON_Value* val = json_parse_string(book_data);
            JSON_Object *json = json_value_get_object(val);

            const char *title = json_object_get_string(json, "title");
            const char *author = json_object_get_string(json, "author");
            const char *publisher = json_object_get_string(json, "publisher");
            const char *genre = json_object_get_string(json, "genre");
            int page_count = json_object_get_number(json, "page_count");

            printf("title: %s\nauthor: %s\npublisher: %s\ngenre: %s\npage_count: %d\n", title, author, publisher, genre, page_count);
            close_connection(sockfd);
            continue;

        } else if (strcmp(command, "add_book") == 0) {
            if(!is_logged_in) {
                printf("EROARE: Nu sunteti logat!\n");
                continue;
            }
            if(!is_in_library) {
                printf("EROARE: Nu aveti acces la biblioteca!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char extra_space[10];
            fgets(extra_space, 10, stdin);

            char something_is_empty = 0;

            printf("title=");
            char title[200];
            fgets(title, 200, stdin);
            title[strcspn(title, "\n")] = 0;
            if (strcmp(title, "") == 0) {
                something_is_empty = 1;
            }

            printf("author=");
            char author[200];
            fgets(author, 200, stdin);
            author[strcspn(author, "\n")] = 0;
            if(strcmp(author, "") == 0){
                something_is_empty = 1;
            }

            printf("genre=");
            char genre[200];
            fgets(genre, 200, stdin);
            genre[strcspn(genre, "\n")] = 0;
            if(strcmp(genre, "") == 0){
                something_is_empty = 1;
            }

            printf("publisher=");
            char publisher[200];
            fgets(publisher, 200, stdin);
            publisher[strcspn(publisher, "\n")] = 0;
            if(strcmp(publisher, "") == 0){
                something_is_empty = 1;
            }

            printf("page_count=");
            char page_count[50];
            fgets(page_count, 50, stdin);
            if(strcmp(page_count, "") == 0){
                something_is_empty = 1;
            }

            if (something_is_empty) {
                printf("EROARE: Cartea nu a putut fi adaugata!\n");
                close_connection(sockfd);
                continue;
            }

            char is_digit = 1;
            for (int i = 0; i < strlen(page_count) - 1; i++) {
                if (!isdigit(page_count[i])) {
                    is_digit = 0;
                    break;
                }
            }
            if (!is_digit) {
                printf("EROARE: Numarul de pagini trebuie sa fie un numar intreg!\n");
                close_connection(sockfd);
                continue;
            }
            page_count[strcspn(page_count, "\n")] = 0;

            json_val = json_value_init_object();
            json_obj = json_value_get_object(json_val);
            json_object_set_string(json_obj, "title", title);
            json_object_set_string(json_obj, "author", author);
            json_object_set_string(json_obj, "genre", genre);
            json_object_set_string(json_obj, "publisher", publisher);
            json_object_set_number(json_obj, "page_count", atoi(page_count));

            char *pretty_message = json_serialize_to_string_pretty(json_val);
            message = compute_post_request("34.246.184.49", "/api/v1/tema/library/books", "application/json", &pretty_message, 0, NULL, 0, tokens, token_count);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (basic_extract_json_response(response) == NULL) {
                printf("SUCCES: Cartea a fost adaugata cu succes!\n");
                close_connection(sockfd);
                continue;
            }
            printf("EROARE: Cartea nu a putut fi adaugata!\n");
            close_connection(sockfd);
            continue;

        } else if (strcmp(command, "delete_book") == 0) {
            if(!is_in_library) {
                printf("EROARE: Nu aveti acces la biblioteca!\n");
                continue;
            }
            printf("id=");
            char id[50];
            scanf("%s", id);

            char new_path[100];
            strcpy(new_path, "/api/v1/tema/library/books/");
            strcat(new_path, id);
            message = compute_delete_request("34.246.184.49", new_path, tokens, token_count);
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (strstr(response, "error")) {
                printf("EROARE: Cartea nu a putut fi stearsa!\n");
                close_connection(sockfd);
                continue;
            }
            printf("SUCCES: Cartea cu id %s a fost stearsa cu succes!\n", id);
            close_connection(sockfd);
            continue;
            
        } else if (strcmp(command, "logout") == 0) {
            if(!is_logged_in) {
                printf("EROARE: Nu sunteti logat!\n");
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/auth/logout", NULL, cookies, cookie_count, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (strstr(response, "error")) {
                printf("EROARE: Nu se poate face logout!\n");
                close_connection(sockfd);
                continue;
            }
            
            close_connection(sockfd);
            is_logged_in = 0;
            is_in_library = 0;
            printf("SUCCES: Utilizatorul s-a delogat cu succes!\n");
            continue;

        } else if (strcmp(command, "exit") == 0) {
            printf("SUCCES: Inchidere program!\n");
            break;
        }
    }

    return 0;
}


