#include "console.h"

struct metadata{
	char name[32];
	int buffer_size;
	int block_count;
	int user_reference;
};

int is_digit(char *p);

const char *prompt;
char *line;

static struct metadata file_table[MAX_OPENED];

int c_init(char *p)
{
	int i;
	for (i = 0; i < MAX_OPENED; i++)
		file_table[i].user_reference = -1;

	while (1) {
		line = readline(p);
		if (line == NULL) { //si apretas control D
			break;
		}
		if (*line == 0) {
			free(line);
			continue;
		}
		if (!strcmp(line, "exit") || !strcmp(line, "quit")) {
			break;
		}

		switch (c_process_line(line))
		{
		case SUCCESS:
			printf("Comando ejecutado con exito!\n");
			fflush(stdout);
			break;
		case INVALID_PARAMETERS:
			printf("Error: Ingreso de parametro no esperado!\n");
			break;
		case EXTERNAL_INVALID_PARAMETERS:
			printf("Error: Los parametros ingresados causan conflictos de logica!\n");
			break;
		case CANNOT_ACCESS_FILE:
			printf("Error: El archivo no se puede accesar!\n");
			break;
		case CANNOT_CREATE_FILE:
			printf("Error: El archivo no se pudo crear!\n");
			break;
		case FORMAT_NOT_ALLOWED:
			printf("Error: Formato no aceptado!\n");
			break;
		case COMMAND_NOT_FOUND:
			printf("Error: El commando ingresado no existe!\n");
			break;
		default:
			printf("Error indefinido!\n");
		}

	}
	printf("\nExiting ...\n");
	return 0;
}

int c_process_line(char *line)
{
	if (!strcmp(line, "clear history"))
		clear_history();
	else
		add_history(line);

	//creamos un arreglo de ingresados
	char *imput[5];
	int i = 0;
	char *p = strtok(line, " "); //printf("Command is ’%s’\n", p);
	imput[i++] = p;

	//llenamos nuestro arreglo de ingresados
	for (p = strtok(NULL, " "); p != NULL && i < 5; p = strtok(NULL, " "))
		imput[i++] = p;

	//chequeamos cuantas palabras fueron ingresadas
	if (i == 5)
	{
		//si se ingreso "create device"
		if (!strcmp(imput[0], "create") && !strcmp(imput[1], "device"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[3]) || !is_digit(imput[4]))
				return INVALID_PARAMETERS;

			//creamos el archivo como se esecifica
			return dev_format(imput[2], atoi(imput[3]), atoi(imput[4]));
		}

		//si se ingreso "open device"
		else if (!strcmp(imput[0], "open") && !strcmp(imput[1], "device") && !strcmp(imput[3], "as"))
		{
			//chequeamos que no exista la referencia que nos dieron
			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference == atoi(imput[4]))
					return EXTERNAL_INVALID_PARAMETERS;
			}

			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[4]))
				return INVALID_PARAMETERS;

			//guardamos el valor retornado y si se abrio el archivo
			int internal_reference = dev_open(imput[2]);
			if (internal_reference < 0)
				return internal_reference;

			//lo agregamos a la tabla de abiertos
			strcpy(file_table[internal_reference].name, imput[2]);
			file_table[internal_reference].buffer_size = get_buffer_size(internal_reference);
			file_table[internal_reference].block_count = get_block_count(internal_reference);
			file_table[internal_reference].user_reference = atoi(imput[4]);
			return SUCCESS;
		}

		//si se ingreso "read block"
		else if (!strcmp(imput[0], "read") && !strcmp(imput[1], "block"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[2]) || !is_digit(imput[3]) || !is_digit(imput[4]) || atoi(imput[4]) <= 0)
				return INVALID_PARAMETERS;

			//buscamos el archivo
			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference == atoi(imput[2]))
					break;
			}

			//revisamos que se encontro
			if (i == MAX_OPENED)
				return CANNOT_ACCESS_FILE;

			//la cantidad que vamos a leer es length
			int length = atoi(imput[4]);

			//si quiere leer mas de lo que se puede solo se lee el maximo
			if (length > file_table[i].buffer_size)
				length = file_table[i].buffer_size;

			//creamos un buffer para leer, leemos y revisamos que no hay errores
			char *buffer = malloc(file_table[i].buffer_size);
			if (dev_read_block(i, buffer, atoi(imput[3])) != SUCCESS)
				return dev_read_block(i, buffer, atoi(imput[3]));

			//se imprime
			for (i = 0; i < length; i++)
			{
				printf("%0*x", 2, buffer[i]);
				fflush(stdout);
			}
			printf("\n");
			free(buffer);
			return SUCCESS;
		}
	}
	else if (i == 4)
	{
		//si se ingreso "show metadata from"
		if (!strcmp(imput[0], "show") && !strcmp(imput[1], "metadata") && !strcmp(imput[2], "from"))
		{
			//chequeamos que imput[3] sea un digito
			if (!is_digit(imput[3]))
				return INVALID_PARAMETERS;

			//buscamos el archivo
			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference == atoi(imput[3]))
					break;
			}

			//revisamos que se encontro
			if (i == MAX_OPENED)
				return CANNOT_ACCESS_FILE;

			//lo imprimimos
			printf("Reference:\t%d\n", file_table[i].user_reference);
			printf("Name:\t\t%s\n", file_table[i].name);
			printf("Block size:\t%d\n", file_table[i].buffer_size);
			printf("Block count:\t%d\n", file_table[i].block_count);
			return SUCCESS;

		}

		//si se ingreso "read block"
		else if (!strcmp(imput[0], "read") && !strcmp(imput[1], "block"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[2]) || !is_digit(imput[3]))
				return INVALID_PARAMETERS;

			//buscamos el archivo
			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference == atoi(imput[2]))
					break;
			}

			//revisamos que se encontro
			if (i == MAX_OPENED)
				return CANNOT_ACCESS_FILE;

			//la cantidad que vamos a leer es length
			int length = 32;

			//si quiere leer mas de lo que se puede solo se lee el maximo
			if (length > file_table[i].buffer_size)
				length = file_table[i].buffer_size;

			//creamos un buffer para leer, leemos y revisamos que no hayan errores
			char *buffer = malloc(file_table[i].buffer_size);
			if (dev_read_block(i, buffer, atoi(imput[3])) != SUCCESS)
				return dev_read_block(i, buffer, atoi(imput[3]));

			//se imprime
			for (i = 0; i < length; i++)
			{
				printf("%0*x", 2, buffer[i]);
				fflush(stdout);
			}
			printf("\n");
			free(buffer);
			return SUCCESS;

		}
	}
	else if (i == 3)
	{
		//si se ingreso "show open devices"
		if (!strcmp(imput[0], "show") && !strcmp(imput[1], "open") && !strcmp(imput[2], "devices"))
		{
			//recorremos el arreglo de abiertos y los imprimimos
			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference != -1)
				{
					printf("%d\t", file_table[i].user_reference);
					printf("%s\n", file_table[i].name);
				}
			}
			return SUCCESS;
		}

		//si se ingreso "close device"
		else if (!strcmp(imput[0], "close") && !strcmp(imput[1], "device"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[2]))
				return INVALID_PARAMETERS;

			for (i = 0; i < MAX_OPENED; i++)
			{
				if (file_table[i].user_reference == atoi(imput[2]))
				{
					int result = dev_close(i);
					if (result == 0)
						file_table[i].user_reference = -1;
					return result;
				}
			}
			return CANNOT_ACCESS_FILE;
		}
	}

	free(line);
	return COMMAND_NOT_FOUND;
}


int is_digit(char *p)
{
	unsigned int iterator;
	for (iterator = 0; iterator < strlen(p); iterator++)
	{
		if (!isdigit(p[iterator]))
			return 0;
	}
	return 1;
}