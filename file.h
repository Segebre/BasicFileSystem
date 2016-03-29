#ifndef FILE_H
#define FILE_H

#include "device.h"

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 4096

int format_device(char* dev_name);
int file_create(int dh, char* dev_name);

#endif // FILE_H