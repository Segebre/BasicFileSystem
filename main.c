#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h>

int main(int argc, char *argv[])
{
	c_init("$ ");
}



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