#include <stdio.h>
#include <string.h>
#include <srt/srt.h>
#include "main.hpp"

// Функция которая скачивает пакеты по srt
int clinet_pull(std::unique_ptr<Params>& param);