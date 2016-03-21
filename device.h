#ifndef DEVICE_H
#define DEVICE_H

#define MAX_OPENED					 3
#define SUCCESS						 0
#define INVALID_PARAMETERS			-1
#define EXTERNAL_INVALID_PARAMETERS -2
#define CANNOT_ACCESS_FILE			-3
#define CANNOT_CREATE_FILE			-4
#define FORMAT_NOT_ALLOWED			-5
#define COMMAND_NOT_FOUND			-6

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

int dev_format(char *name, int block_size, int block_count);//return -1(fail), 0(success)
int dev_open(char *name);//return index
int dev_read_block(int dh, char *buffer, int block_index);
int dev_write_block(int dh, char *buffer, int block_index);
int dev_close(int dh);

int get_buffer_size(int dh);
int get_block_count(int dh);

#endif // DEVICE_H