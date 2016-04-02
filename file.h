#ifndef FILE_H
#define FILE_H

#include "device.h"

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 4096

int format_device(char* dev_name);
int file_create(int dh, char* file_name);
int file_delete(int dh, char* file_name);
int file_rename(int dh, char* file_name, char* file_name2);

#endif // FILE_H