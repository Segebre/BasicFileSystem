#include "console.h"

int main(int argc, char *argv[])
{
	format_device("fish");
	int dh = dev_open("fish"), i = 0;
	//while (!file_delete(dh, "hoa"))
		//i++;
	while (!file_create(dh, "hoa"))
		i++;

	/*file_rename(dh, "hoa", "123456789-123456789-123456789-123456789");
	file_delete(dh, "123456789-123456789-123456789-123456789");
	file_create(dh, "hoa");*/

	file_table_init();
	char* buffer = "HOLAAAAAAAAAAAAAAA me dicen quique pero me llamo juan enrique, que cosas verdad?";
	int fh = file_open(dh, "hoa");
	//file_write(fh, 0, buffer, 100);
	//file_close(fh);
	file_delete(dh, "hoa");
	//fh = file_open(dh, "hoa");
	//buffer = "hol";
	//file_write(fh, 0, buffer, 100);
	printf("%d\n", fh);
	printf("%d\n", file_open(dh, "hoa"));
	printf("%d\n", file_open(dh, "hoa"));
	printf("%d\n", file_close(1));
	printf("%d\n", file_open(dh, "hoa"));


	printf("%d\n", i);
	//c_init("$ ");
}

/*char *buffer = (char *) malloc(10);
memset(buffer, 0, 10);
dev_format("hola", 10, 5);
int i = dev_open("hola");
buffer[0] = 1;
buffer[9] = 1;
dev_write_block(i, buffer, 0);
dev_write_block(i, buffer, 0);
dev_write_block(i, buffer, 1);
dev_write_block(i, buffer, 1);
dev_write_block(i, buffer, 2);
dev_write_block(i, buffer, 2);
dev_write_block(i, buffer, 3);
dev_write_block(i, buffer, 3);
dev_write_block(i, buffer, 4);
dev_write_block(i, buffer, 4);
free(buffer);*/

/*#include <stdio.h>
#include <stdlib.h>		//for free()
#include <string.h>
#include <editline/readline.h>

void processLine(char *line)
{
	char *p;

	if (strcmp(line, "clear history") == 0) {
		clear_history();
	}
	else {
		add_history(line);
	}

	p = strtok(line, " ");
	printf("Command is '%s'\n", p);

	p = strtok(NULL, " ");
	while (p != NULL) {
		printf("Argument: '%s'\n", p);
		p = strtok(NULL, " ");
	}

	free(line);
}

int main(int argc, char *argv[])
{
	const char *prompt = "$ ";
	char *line;

	if (argc > 1) {
		++argv, --argc; //The first argument is the program name
	}

	while (1) {
		line = readline(prompt);

		if (line == NULL) {
			//printf("\n");
			//continue;
			break;
		}

		if (*line == 0) {
			free(line);
			continue;
		}

		if (strcmp(line, "exit") == 0 ||
			strcmp(line, "quit") == 0) {
			break;
		}

		processLine(line);
	}

	printf("\nExiting ...\n");

	return 0;
}*/