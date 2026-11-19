#include "client_srt.hpp"

#define BUFFER_SIZE 1316 // Стандартный размер пакета для MPEG-TS/SRT

int clinet_pull(std::unique_ptr<Params>& param) {
    // Инициализация библиотеки SRT
    if (srt_startup() < 0) {
        fprintf(stderr, "Ошибка инициализации SRT\n");
        return 1;
    }

    // Создание SRT сокета
    SRTSOCKET sock = srt_create_socket();
    if (sock == SRT_INVALID_SOCK) {
        fprintf(stderr, "Не удалось создать сокет: %s\n", srt_getlasterror_str());
        srt_cleanup();
        return 1;
    }

    // Настройка адреса сервера (куда подключаемся)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(param->port);
    if (inet_pton(AF_INET, param->ip_address, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Неверный IP-адрес\n");
        srt_close(sock);
        srt_cleanup();
        return 1;
    }

    // Подключение к серверу (Режим Caller включен по умолчанию при srt_connect)
    printf("Подключение к %s:%d...\n", param->ip_address, param->port);
    if (srt_connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SRT_ERROR) {
        fprintf(stderr, "Ошибка подключения: %s\n", srt_getlasterror_str());
        srt_close(sock);
        srt_cleanup();
        return 1;
    }
    printf("Успешно подключено! Принимаем поток...\n");

    // Открытие файла для записи
    FILE *file = fopen(param->filename, "wb");
    if (!file) {
        perror("Не удалось создать файл для записи");
        srt_close(sock);
        srt_cleanup();
        return 1;
    }

    char buffer[BUFFER_SIZE];
    long long total_bytes = 0;

    // Цикл чтения из сокета и записи в файл
    for (unsigned int i = 0; i < param->counter; i++) {
        // srt_recvmsg4 принимает данные. Для потокового видео этого достаточно
        int bytes_received = srt_recvmsg(sock, buffer, BUFFER_SIZE);
        
        if (bytes_received > 0) {
            fwrite(buffer, 1, bytes_received, file);
            total_bytes += bytes_received;
            // Выводим прогресс в консоль без спама (каждые ~1 МБ)
            if (total_bytes % (1024 * 1024) < BUFFER_SIZE) {
                printf("Принято: %.2f МБ\r", (double)total_bytes / (1024 * 1024));
                fflush(stdout);
            }
        } else if (bytes_received == SRT_ERROR) {
            int reason = srt_getlasterror(NULL);
            // Если поток просто закончился (EOF) или сервер отключился
            if (reason == SRT_ECONNLOST || reason == SRT_ENOCONN) {
                printf("\nТрансляция завершена (соединение закрыто сервером).\n");
            } else {
                fprintf(stderr, "\nОшибка при приеме данных: %s\n", srt_getlasterror_str());
            }
            break;
        }
    }

    // Очистка ресурсов
    fclose(file);
    srt_close(sock);
    srt_cleanup();
    printf("Файл сохранен как %s. Всего байт: %lld\n", param->filename, total_bytes);

    return 0;
}