#include "file.h"

#define MIN(a, b) ((a) > (b))? (b) : (a)

struct file_metadata{
	char name[32];
	int file_size;
	int block_list[64];
};

int format_device(char* dev_name)
{
	//abrimos el device
	int dh = dev_open(dev_name);

	//compreobamos que se abrio
	if (dh < 0)
		return dh;

	//revisamos si es compatible
	if (dev_get_buffer_size(dh) != BLOCK_SIZE || dev_get_block_count(dh) != BLOCK_COUNT)
		return EXTERNAL_INVALID_PARAMETERS;

	//creamos el iterador y el buffer que vamos a usar para escribir
	int i = 0;
	char *buffer = (char *)malloc(BLOCK_SIZE);

	//inicializamos el buffer en 0, luego ponemos en 1 las primeras 26 posiciones, y luego lo escribimos
	memset(buffer, 0, BLOCK_SIZE);
	memset(buffer, 1, 26);
	dev_write_block(dh, buffer, i++);

	//lo volvemos a set en 0 y escribimos los otros 4095 bloques
	memset(buffer, 0, BLOCK_SIZE);
	while (i < 4095)
		dev_write_block(dh, buffer, i++);

	//escribimos que se formateo y lo cerramos
	dev_set_format(dh);
	dev_close(dh);

	free(buffer);
	return SUCCESS;
}

int file_create(int dh, char* file_name)
{
	//chequiamos si esta formateado
	if (!dev_is_format(dh))
		return DEVICE_NOT_FORMAT;

	//creamos el buffer y el iterador
	char* buffer = (char *)malloc(BLOCK_SIZE);
	int available_metadata = -1;

	while (available_metadata < 64)
	{
		//cambia el bloque que se esta leyendo
		if (!((available_metadata+1) % 3))
			dev_read_block(dh, buffer, ((available_metadata+1) / 3) + 4);

		//cambia el inodo que se esta leyendo
		if (!buffer[(++available_metadata % 3) * sizeof(struct file_metadata)])
			break;
		else if (!buffer[(++available_metadata % 3) * sizeof(struct file_metadata)])
			break;
		else if (!buffer[(++available_metadata % 3) * sizeof(struct file_metadata)])
			break;
	}

	//chequiamos si hay espacios vacios
	if (available_metadata >= 64)
	{
		free(buffer);
		return CANNOT_CREATE_FILE;
	}

	//creamos el struct
	struct file_metadata new_file_metadata;
	memset(new_file_metadata.name, 0, 32);
	memcpy(new_file_metadata.name, file_name, MIN(strlen(file_name), 32));
	new_file_metadata.file_size = 0;
	memset(new_file_metadata.block_list, 0, 64*4);

	//lo escribimos en el arreglo y lueo en el archivo
	memcpy((void *)(buffer + ((available_metadata % 3) * sizeof(struct file_metadata))), &new_file_metadata, sizeof(struct file_metadata)); // (void *)buffer[(available_metadata % 3) * sizeof(struct file_metadata)]
	dev_write_block(dh, buffer, (available_metadata/3)+4);

	free(buffer);
	return SUCCESS;
}

int file_get_available_block(int dh)
{
	//chequiamos si esta formateado
	if (!dev_is_format(dh))
		return DEVICE_NOT_FORMAT;

	//creamos el buffer y el iterador
	char* buffer = (char *)malloc(BLOCK_SIZE);
	int available_block = 0;

	//buscamos la siguiente posicion vacia
	while (available_block < 4096)
	{
		//cambia el bloque que se esta leyendo
		if (!(available_block % 1024))
			dev_read_block(dh, buffer, available_block / BLOCK_SIZE);

		//cambia la posicion que se esta leyendo
		if (!buffer[available_block++ % 1024])
		{
			free(buffer);
			return available_block-1;
		}
	}
	
	//si no hay disponibles
	free(buffer);
	return CANNOT_CREATE_FILE;
}