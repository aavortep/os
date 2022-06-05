#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>

#define BUF_LEN 256
#define SERVER_SOCK_PORT 42317
#define MAX_COUNT 8

int sock;
int sock_flag = 0;

void catch_ctrlc(int signum)
{
    printf("Получен сигнал Ctrl+C - остановка работы сервера\n");
    sock_flag = 1;
    close(sock);
}

// прочитать сообщение от клиента
void read_message(unsigned int client_id, int *client_sockets)
{
    char message[BUF_LEN];
    memset(message, 0, BUF_LEN);

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    //getpeername возвращает имя подключившегося сокета
    //int getpeername(int s, struct sockaddr *name, socklen_t *namelen);    
    getpeername(client_sockets[client_id], (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);

    // читаем сообщение (при неудаче считаем, что клиент отключился)
    int rec_BUF_LEN = recv(client_sockets[client_id], message, BUF_LEN, 0);
    if (rec_BUF_LEN == 0)
    {
        printf("Клиент отключился: адрес = %s:%d!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        client_sockets[client_id] = 0;
        close(client_sockets[client_id]);
    }
    else
    {
        message[rec_BUF_LEN] = '\0';
        printf("Сообщение: %s от клиента %s:%d\n",message, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
}

int main(void)
{
    struct sockaddr_in server_addr;

    // создать сокет
    // 0 - протокол по умолчанию
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // инициализировать адрес сокета-сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_SOCK_PORT); //Host To Network Short
    //программа сервер зарегистрируется на всех адресах той машины, на которой она выполняется.
    server_addr.sin_addr.s_addr = INADDR_ANY; 
 
    // привязать сокет приложения-сервера к адресу
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    //listen переводит сервер в режим ожидания запроса на соединение
    if (listen(sock, MAX_COUNT) < 0)
    {
        printf("%s", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }

    int client_sockets[MAX_COUNT] = {0};
    while (!sock_flag)
    {
        fd_set rfds;

        //очищает набор
        FD_ZERO(&rfds);

        //добавляет заданный дескриптор из набора.
        FD_SET(sock, &rfds);

        int max_fd = sock;

        for (int i = 0; i < MAX_COUNT; i++)
        {
            int fd = client_sockets[i];
            if (fd > 0)
                FD_SET(fd, &rfds);
            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }

        //  int select(int n, fd_set *readfds, fd_set *writefds, fd_set *excepfds, const struct timeval *timeout);
        // Первый параметр функции – количество проверяемых дескрипторов. Второй, третий и четвертый параметры - массивы дескрипторов
        //ждать вечно - timeout = NULL
        int updates_cl_count = select(max_fd + 1, &rfds, NULL, NULL, NULL); //процесс блокируется в ожидании обновления, 
		// выходим из селекта когда в каком-либо из файлов соотв дескр появились новые символы 
        
        //после выхода из селект в rfds остались только готовые к вводу-выводу дескрипторы

        // если новое соединение есть
        if (FD_ISSET(sock, &rfds))
        {
            // принять соединение с клиентом
            struct sockaddr_in client_addr;
            int client_addr_size = sizeof(client_addr);

            //используется для получения нового сокета для нового входящего соединения
            int accepted_sock = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
            // возвращает новый сокет, открытый для обмена данными с клиентом, запросившим соединение

            if (accepted_sock < 0)
            {
                printf("%s", strerror(errno));
                close(sock);
                return EXIT_FAILURE;
            }

            // обновляем информацию о наборе клиентов
            int added_flag = 0; // 0 - есть свободное место в таблице клиентов, 1 - нет свободного места
            for (int i = 0; i < MAX_COUNT && !added_flag; i++)
            {
                // находим свободное место в таблице клиентов
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = accepted_sock;
                    added_flag = 1;
                }
            }

            
            printf("Новое подключение: fd = %d, address = %s:%d\n", accepted_sock, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // проверяем есть ли полученные сообщения
        for (int i = 0; i < MAX_COUNT; i++)
        {
            int fd = client_sockets[i];
            if ((fd > 0) && FD_ISSET(fd, &rfds))
            {
                read_message(i, client_sockets);
            }
        }
    }

    return 0;
}