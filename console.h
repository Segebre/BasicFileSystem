#ifndef CONSOLE_H
#define CONSOLE_H

#include "device.h"
#include "hash.h"
#include "file.h"
#include <editline/readline.h>

int c_init(char *p);
static int c_process_line(char *line);

#endif // CONSOLE_H