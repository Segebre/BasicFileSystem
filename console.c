#include "console.h"

int is_digit(char *p);

const char *prompt;
char *line;

int c_init(char *p)
{
	hash_init();
	file_table_init();

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
		case CANNOT_ACCESS_DEVICE:
			printf("Error: El archivo no se puede accesar!\n");
			break;
		case CANNOT_CREATE_DEVICE:
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
			if (hash_lookup(imput[4]) != NULL)
					return EXTERNAL_INVALID_PARAMETERS;

			//lo agregamos a la tabla de abiertos
			return hash_add(imput[2], imput[4]);

		}

		//si se ingreso "read block"
		else if (!strcmp(imput[0], "read") && !strcmp(imput[1], "block"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[3]) || !is_digit(imput[4]) || atoi(imput[4]) <= 0)
				return INVALID_PARAMETERS;

			//buscamos el archivo
			struct hash_node* node = hash_lookup(imput[2]);

			//revisamos que se encontro
			if (node == NULL)
				return EXTERNAL_INVALID_PARAMETERS;

			//la cantidad que vamos a leer es length
			int length = atoi(imput[4]);

			//si quiere leer mas de lo que se puede solo se lee el maximo
			if (length > node->buffer_size)
				length = node->buffer_size;

			//creamos un buffer para leer, leemos y revisamos que no hay errores
			unsigned char *buffer = malloc(node->buffer_size);
			int result = dev_read_block(node->internal_reference, buffer, atoi(imput[3]));
			if (result != SUCCESS)
			{
				free(buffer);
				return result;
			}

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
			//buscamos el archivo
			struct hash_node* node = hash_lookup(imput[3]);

			//revisamos que se encontro
			if (node == NULL)
				return EXTERNAL_INVALID_PARAMETERS;

			//lo imprimimos
			printf("Reference:\t%s\nName:\t\t%s\nBlock size:\t%d\nBlock count:\t%d\n", node->user_reference, node->name, node->buffer_size, node->block_count);
			return SUCCESS;

		}

		//si se ingreso "read block"
		else if (!strcmp(imput[0], "read") && !strcmp(imput[1], "block"))
		{
			//chequeamos que los digitos sean digitos
			if (!is_digit(imput[3]))
				return INVALID_PARAMETERS;

			//buscamos el archivo
			struct hash_node* node = hash_lookup(imput[2]);

			//revisamos que se encontro
			if (node == NULL)
				return EXTERNAL_INVALID_PARAMETERS;

			//la cantidad que vamos a leer es length
			int length = 32;

			//si quiere leer mas de lo que se puede solo se lee el maximo
			if (length > node->buffer_size)
				length = node->buffer_size;

			//creamos un buffer para leer, leemos y revisamos que no hayan errores
			unsigned char *buffer = (char *) malloc(node->buffer_size);
			int result = dev_read_block(node->internal_reference, buffer, atoi(imput[3]));
			if (result != SUCCESS)
			{
				free(buffer);
				return result;
			}

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
			for (i = 0; i < size_of_arreglo; i++)
			{
				struct hash_node* node = get_first(i);
				if (node == NULL)
					continue;
				for (; node != NULL; node = node->next)
					printf("%s\t\t\t%s\n", node->user_reference, node->name);
			}
			return SUCCESS;
		}

		//si se ingreso "close device"
		else if (!strcmp(imput[0], "close") && !strcmp(imput[1], "device"))
		{
			//buscamos el archivo
			struct hash_node* node = hash_lookup(imput[2]);

			//revisamos que se encontro
			if (node == NULL)
				return EXTERNAL_INVALID_PARAMETERS;
			
			//lo eliminamos
			return hash_remove_node(node->user_reference);
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