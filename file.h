#ifndef FILE_H
#define FILE_H

#include "device.h"

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 4096

int format_device(char* dev_name);
int file_create(int dh, char* file_name);
int file_delete(int dh, char* file_name);
int file_rename(int dh, char* file_name, char* file_name2);
int file_open(int dh, char* file_name);
int file_close(int fh);
int file_write(int fh, int pos, char* buffer, int size);
int file_read(int fh, int pos, char* buffer, int size);

int file_table_init();
static int file_get_available_block(int dh);
static int file_table_next();
static int file_metadata_lookup(int dh, char* file_name);

#endif // FILE_H