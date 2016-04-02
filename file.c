#include "file.h"

#define MIN(a, b) ((a) > (b))? (b) : (a)

struct file_metadata{
	char name[32];
	int file_size;
	int block_list[64];
};

static struct file_metadata file_table[MAX_OPENED];

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

int file_delete(int dh, char* file_name)
{
	//chequiamos si esta formateado
	if (!dev_is_format(dh))
		return DEVICE_NOT_FORMAT;

	//creamos los arreglos y los iteradores
	char* buffer = (char *)malloc(BLOCK_SIZE);
	char* byte_map = (char *)malloc(BLOCK_SIZE);
	int metadata_to_erase = -1, block_iterator = 0, byte_iterator = 0;

	//buscamos el metadata a borrar
	while (metadata_to_erase < 64)
	{
		//cambia el bloque que se esta leyendo
		if (!((metadata_to_erase + 1) % 3))
			dev_read_block(dh, buffer, ((metadata_to_erase + 1) / 3) + 4);

		//cambia el inodo que se esta leyendo
		if (!memcmp((buffer + ((++metadata_to_erase % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++metadata_to_erase % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++metadata_to_erase % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
	}

	//chequiamos si hay espacios vacios
	if (metadata_to_erase >= 64)
	{
		free(buffer);
		return CANNOT_ACCESS_FILE;
	}

	//creamos el arreglo de bloques a liberar y lo llenamos
	int block_list[64];
	memcpy(block_list, (void *)(buffer + ((metadata_to_erase % 3) * sizeof(struct file_metadata)) + 32 + sizeof(int)), 64 * sizeof(int));

	//llenamos el byte_map
	dev_read_block(dh, byte_map, (block_list[byte_iterator] / BLOCK_SIZE));
	while ( byte_iterator < 64)
	{
		//cambia el bloque que se esta leyendo
		if ((block_list[byte_iterator] / BLOCK_SIZE) != block_iterator)
		{
			dev_write_block(dh, byte_map, block_iterator);
			dev_read_block(dh, byte_map, (block_list[byte_iterator] / BLOCK_SIZE));
			block_iterator = block_list[byte_iterator] / BLOCK_SIZE;
		}

		//cambia el inodo que se esta leyendo
		if(block_list[byte_iterator++])
			byte_map[block_list[byte_iterator-1] % BLOCK_SIZE] = 0;
	}
	dev_write_block(dh, byte_map, block_iterator);


	//lo borramos del arreglo
	buffer[(metadata_to_erase % 3) * sizeof(struct file_metadata)] = 0;

	//escribimos en la metadata del archivo
	dev_write_block(dh, buffer, (metadata_to_erase / 3) + 4);

	free(buffer);
	free(byte_map);
	return SUCCESS;
}

int file_rename(int dh, char* file_name, char* file_name2)
{
	//chequiamos si esta formateado
	if (!dev_is_format(dh))
		return DEVICE_NOT_FORMAT;

	//creamos los arreglos y los iteradores
	char* buffer = (char *)malloc(BLOCK_SIZE);
	int file_to_rename = -1;

	//buscamos el metadata a renombrar
	while (file_to_rename < 64)
	{
		//cambia el bloque que se esta leyendo
		if (!((file_to_rename + 1) % 3))
			dev_read_block(dh, buffer, ((file_to_rename + 1) / 3) + 4);

		//cambia el inodo que se esta leyendo
		if (!memcmp((buffer + ((++file_to_rename % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++file_to_rename % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++file_to_rename % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
	}

	//chequiamos si hay espacios vacios
	if (file_to_rename >= 64)
	{
		free(buffer);
		return CANNOT_ACCESS_FILE;
	}


	//lo renombramos en el arreglo
	memset((void *)(buffer + ((file_to_rename % 3) * sizeof(struct file_metadata))), 0, 32);
	memcpy((void *)(buffer + ((file_to_rename % 3) * sizeof(struct file_metadata))), file_name2, MIN(file_name2, 32));

	//escribimos en la metadata del archivo
	dev_write_block(dh, buffer, (file_to_rename / 3) + 4);

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