#include "device.h"

struct file_opened_in_table{
	int buffer_size;
	int block_count;
	int format_status;
	FILE *file;
};

struct device_metadata{
	char check[6];
	int buffer_size;
	int block_count;
	int format_status;
};

static struct file_opened_in_table device_file_table[MAX_OPENED];

int dev_format(char *name, int block_size, int block_count)
{
	//no puede haber un block size o cantidad de bloques negativos
	if (block_size <= 0 || block_count < 0)
		return INVALID_PARAMETERS;

	int i;

	//creamos el archivo en blanco y si ya existe se formatea
	FILE *f;
	f = fopen(name, "w");

	//chequeamos que se pudo crear
	if (f == NULL)
		return CANNOT_CREATE_FILE;//printf("Cannot create file %s\n", name);

	//printf("%d", sizeof(struct file_inodo));

	//creamos el inodo de el archivo en especifico
	struct device_metadata metadata;
	strncpy(metadata.check, "Hola!", 6);//en el espacio del nombre ponemos la palabra "Hola!" para poder revisar que es un archivo formateado por este api
	metadata.buffer_size = block_size;
	metadata.block_count = block_count;
	metadata.format_status = 0;

	//escribimos el inodo especifico del archivo
	fwrite(&metadata, 1, sizeof(struct device_metadata), f);

	//creamos un buffer de tamano especificado y creamos una cantidad de bloques especificada
	char *buffer = (char *) malloc(block_size);
	memset(buffer, 0, block_size);
	for (i = 0; i < block_count; i++)
		fwrite(buffer, 1, block_size, f);

	fclose(f);
	free(buffer);
	return SUCCESS;
}

int dev_open(char *name)
{
	int i;

	//abrimos el archivo en modo lectura y escritura
	FILE *f = fopen(name, "r+");
	struct device_metadata metadata;

	//revisamos que no hayan problemas
	if (f == NULL)
		return CANNOT_ACCESS_FILE;

	//leemos el inodo especifico del archivo
	fread(&metadata, 1, sizeof(struct device_metadata), f);

	//chequea si tiene la palabra clave de nuestro formateo, "Hola!"
	if (strcmp(metadata.check, "Hola!"))
	{
		fclose(f);
		return FORMAT_NOT_ALLOWED;
	}

	//busca cual esta vacio
	for (i = 0; i < MAX_OPENED; i++)
	{
		if (device_file_table[i].file == NULL)
			break;

		//si la tabla esta llena no lo guarda y cierra el FILE
		if (i == MAX_OPENED - 1)
		{
			fclose(f);
			return EXTERNAL_INVALID_PARAMETERS;
		}
	}

	//reescribe el inodo a la tabla
	device_file_table[i].block_count = metadata.block_count;
	device_file_table[i].buffer_size = metadata.buffer_size;
	device_file_table[i].format_status = metadata.format_status;
	device_file_table[i].file = f;

	return i;
}

int dev_write_block(int dh, char *buffer, int block_index)
{
	//chequea si el dh es aceptable, si el archivo esta abierto y si el bloque existe
	if (dh < 0 || dh >= MAX_OPENED)
		return INVALID_PARAMETERS;//printf("File %s\n not opened", device_file_table[dh].name);

	if (block_index < 0 || device_file_table[dh].block_count <= block_index || device_file_table[dh].file == NULL)
		return EXTERNAL_INVALID_PARAMETERS;

	//se mueve al bloque que va a escribir incluyendo el sizeof(struct file_inodo) del inodo personal de cada archivo
	fseek(device_file_table[dh].file, (block_index*device_file_table[dh].buffer_size) + sizeof(struct device_metadata), SEEK_SET);
	fwrite(buffer, 1, device_file_table[dh].buffer_size, device_file_table[dh].file);
	return SUCCESS;
}

int dev_read_block(int dh, char *buffer, int block_index)
{
	//chequea si el dh es aceptable, si el archivo esta abierto y si el bloque existe
	if (dh < 0 || dh >= MAX_OPENED)
		return INVALID_PARAMETERS;//printf("File %s\n not opened", device_file_table[dh].name);

	if (block_index < 0 || device_file_table[dh].block_count <= block_index || device_file_table[dh].file == NULL)
		return EXTERNAL_INVALID_PARAMETERS;

	//se mueve al bloque que va a leer incluyendo el sizeof(struct file_inodo) del inodo personal de cada archivo
	fseek(device_file_table[dh].file, (block_index*device_file_table[dh].buffer_size) + sizeof(struct device_metadata), SEEK_SET);
	fread(buffer, 1, device_file_table[dh].buffer_size, device_file_table[dh].file);
	return SUCCESS;
}

int dev_close(int dh)
{
	//chequea si el dh es aceptable
	if (dh < 0 || dh >= MAX_OPENED)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (device_file_table[dh].file == NULL)
		return CANNOT_ACCESS_FILE;

	//lo cerramos y lo ponemos en NULL en el arreglo
	fclose(device_file_table[dh].file);
	device_file_table[dh].file = NULL;
	return SUCCESS;
}

int get_buffer_size(int dh)
{
	return device_file_table[dh].buffer_size;
}

int get_block_count(int dh)
{
	return device_file_table[dh].block_count;
}

FILE* get_file(int dh)
{
	fseek(device_file_table[dh].file, 0, SEEK_CUR);
	return device_file_table[dh].file;
}

int is_format(int dh)
{
	//chequea si el dh es aceptable
	if (dh < 0 || dh >= MAX_OPENED)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (device_file_table[dh].file == NULL)
		return CANNOT_ACCESS_FILE;

	return device_file_table[dh].format_status;
}

int set_format(int dh)
{
	//chequea si el dh es aceptable
	if (dh < 0 || dh >= MAX_OPENED)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (device_file_table[dh].file == NULL)
		return CANNOT_ACCESS_FILE;

	//creamos nuestro struct para cambiar el format status
	struct device_metadata metadata;

	//leemos el inodo especifico del archivo
	fseek(device_file_table[dh].file, 0, SEEK_SET);
	fread(&metadata, 1, sizeof(struct device_metadata), device_file_table[dh].file);

	//escribimos que esta formateado en el archivo y el 
	metadata.format_status = 1;
	device_file_table[dh].format_status = 1;

	//escribimos en el archivo
	fseek(device_file_table[dh].file, 0, SEEK_SET);
	fwrite(&metadata, 1, sizeof(struct device_metadata), device_file_table[dh].file);

	return SUCCESS;
}