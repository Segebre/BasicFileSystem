#ifndef DEVICE_H
#define DEVICE_H

#define MAX_OPENED                   3
#define SUCCESS                      0
#define INVALID_PARAMETERS          -1
#define EXTERNAL_INVALID_PARAMETERS -2
#define CANNOT_ACCESS_FILE          -3

#include "device.h"
#include <stdio.h>
#include <editline/readline.h>

int c_init(char *p);
int c_process_line(char *line);

#endif // PROCESSLINE_H