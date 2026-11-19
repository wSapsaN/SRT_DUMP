#include "client_srt.hpp"

void free_structe(std::unique_ptr<Params>& param) {

  if (param->URL != NULL) { delete[] param->URL; }
  if (param->filename != NULL) { delete[] param->filename; }
  if (param->ip_address != NULL) { delete[] param->ip_address; }
  
}

void tutorial() {
  printf("Тулз для проверки и снятия дампа srt источника без шифрования. (не работает с DNS)\n");
  printf("Обязательный параметр который принимает тулз:\n");
  printf("\t-u Ссылка на источник типа srt://127.0.0.1:2020\n");
  printf("\nНе обязательные флаги:\n");
  printf("\t-c Количество принимаемых чанков. - По умолчанию 5\n");
  printf("\t-f Имя файла в который записывается полезная нагрузка. - По умолчанию /dev/null\n");
}

bool check_port_range(char* port) {
  int dig_port = std::stoi(port);
  if (dig_port > MAX_PORT_RANGE) {
    printf("Число порта выходит за рамки общих допустимых диапазон сетевых портов.\n");
    return 1;
  }

  return 0;
}

bool check_octet_range(char* ip) {
  // check octet range
  // if octet > 8 bit return 1

  char octet[4];
  unsigned int count = 0;
  unsigned int size_ip = std::strlen(ip) + 1;
  memset(octet, 0, sizeof(octet));
  
  if (!std::isdigit(static_cast<unsigned char>(ip[0]))) {
    printf("Неверный IP: %s\n", ip);
    return 1;
  }

  for (unsigned int i = 0; i < size_ip; i++) {

    if (std::isdigit(static_cast<unsigned char>(ip[i]))) {
      if (count >= 3)
      {
        printf("Октет превышает 8 бит: %s\n", ip);
        return 1;
      }
      
      octet[count] = ip[i];
      count++;
    } else {
      if (octet[0] == '\0') {
        printf("Ошибка в ip адресе: %s\n", ip);
        return 1;
      }

      int dig_octet = std::stoi(octet);
      memset(octet, 0, sizeof(octet));
      
      count = 0;

      if (dig_octet > 255) {
        printf("Октет превышает 8 бит: %s\n", ip);
        return 1;
      }
    }
  }

  return 0;
}

bool check_url(std::unique_ptr<Params>& param, char* link) {
  int size_url = strlen(link);
  
  char srt[7]; // srt://
  char ip[15];
  char port[6];
  unsigned int count_ip = 0;
  unsigned int count_port = 0;

  // FLAGS
  bool f_srt = 1;
  bool f_ip = 0;
  bool f_port = 0;

  // clear array char
  memset(srt, 0, sizeof(srt));
  memset(port, 0, sizeof(port));
  memset(ip, 0, sizeof(ip));

  for (int i = 0; i < size_url; i++) {
    // check srt protocol
    if (f_srt) {
      if (i > 5) {
        printf("Неверный протокол.\n");
        return 1;
      }

      srt[i] = link[i];
      
      if (std::strcmp(srt, "srt://") == 0) {
        f_ip = 1;
        f_srt = 0;
        continue;
      }

    }
    //######################//

    // check ip

    if (f_ip) {
      if (link[i] == ':') {
        f_ip = 0;
        f_port = 1;

        ip[std::strlen(ip) + 1] = '\0';
        
        // call function for check octet
        if (check_octet_range(ip)) return 1;
        // param->URL = new char[std::strlen(argv[i+1]) + 1];
        // std::strcpy(param->URL, argv[i+1]);
        
        param->ip_address = new char[std::strlen(ip) + 1];
        std::strcpy(param->ip_address, ip);
        
        continue;
      }
      if (count_ip > 14) {
        printf("Неверный формат ссылки: %s\n", link);
        printf("Верный формат: srt://<ip>:<port>\n");
        return 1;
      }

      ip[count_ip] = link[i];
      count_ip++;
    }

    //######################//
    
    // check range port

    if (f_port) {
      if (count_port >= 5) {
        printf("Порт выходит из общего допустимого диапозона сетевых портов.\n");
        return 1;
      }

      if (!std::isdigit(link[i])) {
        printf("Порт должен состоять только из целых чисел: %s\n", link);
        return 1;
      }

      port[count_port] = link[i];
      count_port++;
    }
  }
  
  if (port[0] == '\0')
  {
    printf("Вы не указали порт.\n");
    return 1;
  }
  
  port[count_port] = '\0';
  if (check_port_range(port)) return 1;

  return 0;
}

bool wrParamsPort(std::unique_ptr<Params>& param) {
  if (param->URL == NULL) { return 1; }

  std::string tmp_port = "";
  uint16_t result = 0;

  unsigned int pcount = 0;
  bool trigger = 0;
  size_t url_size = std::strlen(param->URL);
  for (size_t i = 5; i < url_size; i++)
  {
    if (trigger)
    {
      if (pcount > SIZE_PORT)
      {
        printf("Большое значение порта.\n");
        return 1;
      }
      
      tmp_port += param->URL[i];
      pcount++;
    }
    
    if (param->URL[i] == ':') { trigger = 1; }
  }

  auto [ptr, ec] = std::from_chars(tmp_port.data(), tmp_port.data() + tmp_port.size(), result);

  if (ec == std::errc() && ptr == tmp_port.data() + tmp_port.size()) {
    param->port = result;
    return 0;
  }

  return 1;
}

bool pars_argv(std::unique_ptr<Params>& param, int argc, char* argv[]) {
 
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "-u") == 0) {
      if (argc == (i+1) || argv[i+1][0] == '-') continue;

      if (check_url(param, argv[i+1]) == 0) {
        param->URL = new char[std::strlen(argv[i+1]) + 1];
        std::strcpy(param->URL, argv[i+1]);
        
        if (wrParamsPort(param)) {
          param->err_state = 1;
          return 1;
        }
        continue;
      } else { 
        param->err_state = 1;
        return 1;
      }
    } else if (std::strcmp(argv[i], "-c") == 0) {
      if (argc == (i+1) || argv[i+1][0] == '-') continue;

      char* end = nullptr;
      long value = std::strtol(argv[i+1], &end, 10);
      if (end != argv[i+1] && *end == '\0') {
        param->counter = static_cast<int>(value);
      }
    } else if (std::strcmp(argv[i], "-f") == 0) {
      if (argc == (i+1) || argv[i+1][0] == '-') continue;

      param->filename = new char[std::strlen(argv[i+1]) + 1];
      std::strcpy(param->filename, argv[i+1]);
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {

  if (argc == 1) {
    tutorial();
    return 0;
  }

  auto p = std::make_unique<Params>();
  p->err_state = 0;
 
  pars_argv(p, argc, argv); // if ret == 1; then it ERROR


  if (p->err_state)
  {
    free_structe(p);
    return 1;
  }

  if (p->URL == nullptr) {
    printf("Вы не указали ссылку на источник srt.\n");
    printf("Вызовите тулз без параметров для получения справки.\n");
    
    return 1;
  }

  if (p->filename == nullptr) {
    const char defaultFilename[] = "/dev/null";
    p->filename = new char[std::strlen(defaultFilename) + 1];
    std::strcpy(p->filename, defaultFilename);
  }

  if (p->counter == 0) { p->counter = 5; }

  printf("%s -> %s\n", p->URL, p->filename);
  printf("%d\n", p->counter);

  clinet_pull(p);
  free_structe(p);

  return 0;
}