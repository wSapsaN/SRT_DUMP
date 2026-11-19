#include <stdio.h>
#include <memory>
#include <cstring>
#include <cctype>
#include <string>
#include <cstdint>
#include <charconv>
#include <string_view>

#define MAX_PORT_RANGE 65535 // максимальное значение порта
#define FILE_OUT "/dev/null" // файл записи данных по умолчанию
#define COUNT_CHUNKS 5       // число пакетов приниамемой программы по умолчанию
#define SIZE_PORT 6          // максимальный размер порта в символах

// Структура данных которые передает клиент
struct Params {
  char* URL;            // srt ссылка
  char* filename;       // имя файла
  char* ip_address;     // адрес сервера
  uint16_t port;        // порт сервера
  unsigned int counter; // число пакетов которые примет программа
  bool err_state;       // состояние ошибки
};