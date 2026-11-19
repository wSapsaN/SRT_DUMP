#include <stdio.h>
#include <memory>
#include <cstring>
#include <cctype>
#include <string>
#include <cstdint>
#include <charconv>
#include <string_view>

#define MAX_PORT_RANGE 65535
#define FILE_OUT "/dev/null"
#define COUNT_CHUNKS 5
#define SIZE_PORT 6

struct Params {
  char* URL;
  char* filename;
  char* ip_address;
  uint16_t port;
  unsigned int counter;
  bool err_state;
};