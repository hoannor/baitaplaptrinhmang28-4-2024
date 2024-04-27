#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

// kiem tra dinh dang thoi gian ma client yeu cau
// code tham khao tren mang
const char* format_time(const char* format) {
    static char formatted_time[256];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    // printf("%s", format);
    if (strcmp(format, "[dd/mm/yyyy]\n") == 0) {
        strftime(formatted_time, 256, "%d/%m/%Y\n", tm_info);
    } else if (strcmp(format, "[dd/mm/yy]\n") == 0) {
        strftime(formatted_time, 256, "%d/%m/%y\n", tm_info);
    } else if (strcmp(format, "[mm/dd/yyyy]\n") == 0) {
        strftime(formatted_time, 256, "%m/%d/%Y\n", tm_info);
    } else if (strcmp(format, "[mm/dd/yy]\n") == 0) {
        strftime(formatted_time, 256, "%m/%d/%y\n", tm_info);
    } else {
        snprintf(formatted_time, 256, "Invalid format\n");
    }

    return formatted_time;
}

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    char buf[256];

    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            while (1) {
                int client = accept(listener, NULL, NULL);
                printf("New client accepted: %d\n", client);
                const char* form = "Please send GET_TIME [format] (Ex: GET_TIME [dd/mm/yyyy], GET_TIME [dd/mm/yy], GET_TIME [mm/dd/yyyy], GET_TIME [mm/dd/yy])\n";
                send(client, form, strlen(form), 0);
                // Kiem tra xem client co gui dung thong diep khong
                while (true) {
                    int ret = recv(client, buf, sizeof(buf) - 1, 0);
                    if (ret > 0) {
                        buf[ret] = '\0';
                        printf("Received from %d: %s\n", client, buf);

                        // Kiem tra yeu cau dung format khong
                        if (strncmp(buf, "GET_TIME", 8) == 0) {
                            char* format = buf + 9; // lay dinh dang thoi gian tu client
                            // printf("%s\n", format);
                            char* response = format_time(format);
                            send(client, response, strlen(response), 0);
                        } else {
                            char* error_msg = "Invalid command\n";
                            send(client, error_msg, strlen(error_msg), 0);
                        }
                    }
                }

                close(client);
            }
            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}

