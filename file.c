#include "file.h"

#define MIN(a, b) ((a) > (b))? (b) : (a)
#define MAX_FILES	13

struct file_metadata{
	char name[32];
	int file_size;
	int block_list[64];
};

struct file_opened_in_table{
	int dh;
	char name[32];
	int file_size;
	int block_list[64];
};

static struct file_opened_in_table file_table[MAX_OPENED];

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

	//lo escribimos en el arreglo y luego en el archivo
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
	char* byte_map = (char *)malloc(BLOCK_SIZE);
	int block_iterator = 0, byte_iterator = 0;

	//buscamos el metadata a borrar
	int metadata_to_erase = file_metadata_lookup(dh, file_name);
	if (metadata_to_erase < 0)
		return metadata_to_erase;

	//guardamos el metadata en un arreglo y luego en un struct
	char* buffer = (char *)malloc(BLOCK_SIZE);
	dev_read_block(dh, buffer, ((metadata_to_erase) / 3) + 4);

	//lo guardamos en el struct
	struct file_metadata temp;
	memcpy(&temp, (void *)(buffer + ((metadata_to_erase % 3) * sizeof(struct file_metadata))), sizeof(struct file_metadata));
	

	//llenamos el byte_map
	dev_read_block(dh, byte_map, (temp.block_list[byte_iterator] / BLOCK_SIZE));
	while ( byte_iterator < 64)
	{
		//cambia el bloque que se esta leyendo
		if ((temp.block_list[byte_iterator] / BLOCK_SIZE) != block_iterator)
		{
			dev_write_block(dh, byte_map, block_iterator);
			dev_read_block(dh, byte_map, (temp.block_list[byte_iterator] / BLOCK_SIZE));
			block_iterator = temp.block_list[byte_iterator] / BLOCK_SIZE;
		}

		//cambia el inodo que se esta leyendo
		if(temp.block_list[byte_iterator++])
			byte_map[temp.block_list[byte_iterator-1] % BLOCK_SIZE] = 0;
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
	memcpy((void *)(buffer + ((file_to_rename % 3) * sizeof(struct file_metadata))), file_name2, MIN(strlen(file_name2), 32));

	//escribimos en la metadata del archivo
	dev_write_block(dh, buffer, (file_to_rename / 3) + 4);

	free(buffer);
	return SUCCESS;
}

int file_open(int dh, char* file_name)
{
	//buscamos el metadata
	int file_to_open = file_metadata_lookup(dh, file_name);
	if (file_to_open < 0)
		return file_to_open;

	//busca cual esta vacio
	int free_position = file_table_next();
	if (free_position < 0)
		return free_position;

	char* buffer = (char *)malloc(BLOCK_SIZE);
	dev_read_block(dh, buffer, ((free_position) / 3) + 4);

	//lo copiamos a un file_metadtata temporal y lo guardamos en el arreglo
	struct file_metadata temp;
	memcpy(&temp, (void *)(buffer + ((file_to_open % 3) * sizeof(struct file_metadata))), sizeof(struct file_metadata));
	file_table[free_position].dh = dh;
	memcpy(file_table[free_position].name, temp.name, MIN(strlen(temp.name), 32));
	file_table[free_position].file_size = temp.file_size;
	memcpy(file_table[free_position].block_list, temp.block_list, MIN(strlen(temp.name), 64 * 4));

	free(buffer);
	return free_position;
}

int file_close(int fh)
{
	//chequea si el dh es aceptable
	if (fh < 0 || fh >= MAX_FILES)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (file_table[fh].dh == -1)
		return CANNOT_ACCESS_DEVICE;

	//lo cerramos y lo ponemos en -1 en el arreglo
	file_table[fh].dh = -1;
	return SUCCESS;
}

int file_write(int fh, int pos, char* buffer, int size)
{
	//chequea si el fh y los parametros son aceptable
	if (fh < 0 || fh >= MAX_FILES || pos < 0 || size < 0)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (file_table[fh].dh == -1)
		return CANNOT_ACCESS_DEVICE;

	//chequiamos si el tama�o del archivo es suficiente
	if (file_table[fh].file_size < pos || pos + size > BLOCK_SIZE * 64)
		return UNSUFFICIENT_SPACE;

	//si el arreglo de bloques esta vacio entonces agregamos el primero
	if (!(file_table[fh].block_list[(pos) / BLOCK_SIZE]))
	{
		int available_block = file_get_available_block(file_table[fh].dh);
		if (available_block > 0)
			file_table[fh].block_list[(pos) / BLOCK_SIZE] = available_block;
	}

	//hacemos un arreglo para escribir el buffer recibido
	char* buffer_interno = (char *)malloc(BLOCK_SIZE);
	dev_read_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[pos / BLOCK_SIZE]);

	//escribimos lo que menos espacio tome
	size = MIN((int)strlen(buffer), size);

	//recorremos todo el arreglo a escribir
	int i;
	for (i = 0; i < size; i++)
	{
		//revisamos si hay que cambiar de bloque
		if (!((pos + i) % BLOCK_SIZE))
		{
			//escribimos el bloque
			if ((pos + i))
				dev_write_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[(pos + i - 1) / BLOCK_SIZE]);

			//si no hay siguiente bloque entonces usamos uno nuevo
			if (!(file_table[fh].block_list[(pos + i) / BLOCK_SIZE]))
			{
				//revisamos si hay bloques disponibles, sino entonces guardamos lo que tenemos y retornamos el error
				int available_block = file_get_available_block(file_table[fh].dh);
				if (available_block < 0)
				{
					struct file_metadata temp_metadata;
					memcpy(temp_metadata.name, file_table[fh].name, strlen(file_table[fh].name));
					temp_metadata.file_size = pos + MIN((int)strlen(buffer), size);
					memcpy(temp_metadata.block_list, file_table[fh].block_list, 64 * 4);

					memset(buffer_interno, 0, BLOCK_SIZE);
					int file_to_write = file_metadata_lookup(file_table[fh].dh, file_table[fh].name);
					dev_read_block(file_table[fh].dh, buffer_interno, ((file_to_write) / 3) + 4);

					memcpy((void *)(buffer_interno + ((file_to_write % 3) * sizeof(struct file_metadata))), &temp_metadata, sizeof(struct file_metadata));
					dev_write_block(file_table[fh].dh, buffer_interno, (file_to_write / 3) + 4);

					return available_block;
				}
				//lo agregamos a la lista de bloques
				file_table[fh].block_list[(pos + i) / BLOCK_SIZE] = available_block;
			}
			//leemos el nuevo bloque
			dev_read_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[(pos + i) / BLOCK_SIZE]);
		}

		//copiamos de un buffer a otro
		buffer_interno[(pos + i) % BLOCK_SIZE] = buffer[i];
	}
	//escribimos el ultimo bloque
	dev_write_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[(pos + i) / BLOCK_SIZE]);

	//creamos el struct y lo llenamos
	struct file_metadata temp_metadata;
	memset(temp_metadata.name, 0, 32);
	memcpy(temp_metadata.name, file_table[fh].name, strlen(file_table[fh].name));
	temp_metadata.file_size = pos + MIN((int)strlen(buffer), size);
	memcpy(temp_metadata.block_list, file_table[fh].block_list, 64 * 4);

	//reiniciamos el buffer_interno en 0 y leemos la metadata hacia el
	memset(buffer_interno, 0, BLOCK_SIZE);
	int file_to_write = file_metadata_lookup(file_table[fh].dh, file_table[fh].name);
	dev_read_block(file_table[fh].dh, buffer_interno, ((file_to_write) / 3) + 4);

	//copiamos los datos y los escribimos en disco
	memcpy((void *)(buffer_interno + ((file_to_write % 3) * sizeof(struct file_metadata))), &temp_metadata, sizeof(struct file_metadata));
	dev_write_block(file_table[fh].dh, buffer_interno, (file_to_write / 3) + 4);

	return SUCCESS;
}

int file_read(int fh, int pos, char* buffer, int size)
{
	//chequea si el fh y los parametros son aceptable
	if (fh < 0 || fh >= MAX_FILES || pos < 0 || size < 0)
		return INVALID_PARAMETERS;

	//chequiamos si el archivo esta abierto
	if (file_table[fh].dh == -1)
		return CANNOT_ACCESS_DEVICE;

	//chequiamos si el tama�o del archivo es suficiente
	if (file_table[fh].file_size < pos || pos + size > BLOCK_SIZE * 64)
		return UNSUFFICIENT_SPACE;

	//si el arreglo de bloques esta vacio entonces termino
	if (!(file_table[fh].block_list[(pos) / BLOCK_SIZE]))
		return SUCCESS;

	//hacemos un arreglo para leer al buffer recibido
	char* buffer_interno = (char *)malloc(BLOCK_SIZE);
	dev_read_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[pos / BLOCK_SIZE]);

	//recorremos todo el arreglo a escribir
	int i;
	for (i = 0; i < size; i++)
	{
		//revisamos si hay que cambiar de bloque
		if (!((pos + i) % BLOCK_SIZE))
		{
			//si no hay siguiente bloque entonces usamos uno nuevo
			if (!(file_table[fh].block_list[(pos + i) / BLOCK_SIZE]))
				return SUCCESS;

			//leemos el nuevo bloque
			dev_read_block(file_table[fh].dh, buffer_interno, file_table[fh].block_list[(pos + i) / BLOCK_SIZE]);
		}

		//copiamos de un buffer a otro
		buffer[i] = buffer_interno[(pos + i) % BLOCK_SIZE];
	}

	return SUCCESS;
}

int file_table_init()
{
	int i;
	for (i = 0; i < MAX_FILES; i++)
		file_table[i].dh = -1;
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
			memset(buffer, 0, BLOCK_SIZE);
			dev_write_block(dh, buffer, available_block - 1);
			free(buffer);
			return available_block-1;
		}
	}
	
	//si no hay disponibles
	free(buffer);
	return UNSUFFICIENT_SPACE;
}

int file_table_next()
{
	//recorremos el arreglo
	int free_position;
	for (free_position = 0; free_position < MAX_FILES; free_position++)
	{
		if (file_table[free_position].dh == -1)
			break;
	}

	//si la tabla esta llena no lo guarda
	if (free_position == MAX_FILES)
		return EXTERNAL_INVALID_PARAMETERS;

	return free_position;
}

int file_metadata_lookup(int dh, char* file_name)
{
	//chequiamos si esta formateado
	if (!dev_is_format(dh))
		return DEVICE_NOT_FORMAT;

	//creamos los arreglos y los iteradores
	char* buffer = (char *)malloc(BLOCK_SIZE);
	int file_to_lookup = -1;

	//buscamos el metadata a renombrar
	while (file_to_lookup < 64)
	{
		//cambia el bloque que se esta leyendo
		if (!((file_to_lookup + 1) % 3))
			dev_read_block(dh, buffer, ((file_to_lookup + 1) / 3) + 4);

		//cambia el inodo que se esta leyendo
		if (!memcmp((buffer + ((++file_to_lookup % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++file_to_lookup % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
		else if (!memcmp((buffer + ((++file_to_lookup % 3) * sizeof(struct file_metadata))), file_name, MIN(strlen(file_name), 32)))
			break;
	}

	//chequiamos si se encontro
	if (file_to_lookup >= 64)
	{
		free(buffer);
		return CANNOT_ACCESS_FILE;
	}

	free(buffer);
	return file_to_lookup;
}