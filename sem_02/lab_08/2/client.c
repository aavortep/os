#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>

#define MSG_LEN 256

int sock;
int stop_flag = 0;
void sigint_catcher(int signum)
{
    stop_flag = 1;
}

int main(int argc, char **argv)
{
    // создать сокет
    // сетевой, потоковый, протокол по умолчанию
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // преобразование доменного адреса в сетевой
    struct hostent *host = gethostbyname("localhost");
    
    if (!host)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // инициализировать адрес сокета сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(42317);
     // берем номер порта , который больше , чем 1023 - нам не нужен зарезервированный порт.
     //  Он также больше 5000 - для того , чтобы не было конфликта с динамическими портами. 
     // Он меньше 49152 - опять же для того , чтобы избежать конфликта с "правильным" диапазоном динамических портов. 
    server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);

    // установка соединения с сервером
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // отлавливаем cntrl+C
    signal(SIGINT, sigint_catcher);

    char message[MSG_LEN];
    int message_num = 0;
    srand(time(NULL));
    
    while (!stop_flag)
    {
        memset(message, 0, MSG_LEN);
        //Функция inet_ntoa() преобразует IP-адрес in, заданный в сетевом порядке расположения байтов, в стандартный строчный вид
        sprintf(message, "%d %s отправил серверу %s:%d", message_num, "Client message", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
        printf("%s\n", message);

        if (send(sock, message, strlen(message), 0) < 0)
        {
            printf("%s", strerror(errno));
            return EXIT_FAILURE;
        }
        message_num++;
        sleep(4 + rand() % 6);
    }

    return EXIT_SUCCESS;
}