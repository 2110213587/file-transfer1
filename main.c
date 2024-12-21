#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <sys/stat.h>

#pragma comment(lib, "ws2_32.lib")

#define HOST "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void upload_file(SOCKET client_socket) {
    char file_path[BUFFER_SIZE];
    FILE *file;
    while(1){
        printf("Enter the full file path to upload: ");
        scanf("%s", file_path);

        file = fopen(file_path, "rb");
        if (file == NULL) {
            printf("File not found!\n");
            continue;
        }
        break;
    }

    send(client_socket, file_path, strlen(file_path), 0);

    struct stat file_stat;
    stat(file_path, &file_stat);
    long file_size = file_stat.st_size;
    char file_size_str[BUFFER_SIZE];
    sprintf(file_size_str, "%ld", file_size);
    send(client_socket, file_size_str, strlen(file_size_str), 0);

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(file);

    printf("File '%s' uploaded successfully.\n", file_path);
}

void download_file(SOCKET client_socket) {
    char files_name[BUFFER_SIZE] = {0};
    recv(client_socket, files_name, BUFFER_SIZE, 0);
    printf("Available files on server:\n%s\n", files_name);

    char chosen_file[BUFFER_SIZE];
    printf("Enter the file name to download: ");
    scanf("%s", chosen_file);

    send(client_socket, chosen_file, strlen(chosen_file), 0);

    char server_response[BUFFER_SIZE] = {0};
    recv(client_socket, server_response, BUFFER_SIZE, 0);

    if (strcmp(server_response, "OK") == 0) {
        char file_size_str[BUFFER_SIZE];
        recv(client_socket, file_size_str, BUFFER_SIZE, 0);
        long file_size = atol(file_size_str);
        printf("Downloading file of size: %ld bytes.\n", file_size);

        char download_dir[BUFFER_SIZE];
        printf("Enter the directory to save the file: ");
        scanf("%s", download_dir);

        if (mkdir(download_dir) == 0) {
            printf("Directory '%s' created.\n", download_dir);
        }

        char file_path[BUFFER_SIZE];
        sprintf(file_path, "%s/downloaded_%s", download_dir, chosen_file);

        FILE *file = fopen(file_path, "wb");
        if (file == NULL) {
            printf("Error creating file '%s'.\n", file_path);
            return;
        }

        char buffer[BUFFER_SIZE];
        long received = 0;
        while (received < file_size) {
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            fwrite(buffer, 1, bytes_received, file);
            received += bytes_received;
        }
        fclose(file);

        printf("File '%s' downloaded successfully to '%s'.\n", chosen_file, download_dir);
    } else {
        printf("File not found on server.\n");
    }
}

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(HOST);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connect failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Connected to server.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);



        while (1) {
            char choice[2];
            scanf("%s", choice);

            if (strcmp(choice, "1") != 0 && strcmp(choice, "2") != 0 && strcmp(choice, "3") != 0) {
                printf("Invalid option! Please choose either '1', '2', or '3'.\n");
                printf("Choose another option:\n");
                continue;
            }

            send(client_socket, choice, strlen(choice), 0);


         if (strcmp(choice, "1") == 0) {
            upload_file(client_socket);
            break;
        } else if (strcmp(choice, "2") == 0) {
            download_file(client_socket);
            break;
        } else if (strcmp(choice, "3") == 0) {
            printf("Exiting client.\n");
            closesocket(client_socket);
            WSACleanup();
            return 0;
        }
      }
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
