#include "hash.h"
#include "device.h"

unsigned int hash(char* reference) {
	unsigned int i = 0, x = 0;
	while (i < strlen(reference))
		x += reference[i++];
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x);
	return x % size_of_arreglo;
}

int init() {
	int i;
	
	//inicializamos todo el arreglo en NULL
	for (i = 0; i < size_of_arreglo; i++)
		arreglo[i] = NULL;
	return 0;
}

int add(char* name, char* user_reference) {
	
	//guardamos el valor retornado y si se abrio el archivo
	int internal_reference = dev_open(name);
	if (internal_reference < 0)
		return internal_reference;

	//lo agregamos a la tabla de abiertos
	int index = hash(user_reference);
	struct hash_node * new_node = (struct hash_node *) malloc(sizeof(struct hash_node));
	memcpy(new_node->name, name, 32);
	new_node->block_count = get_block_count(internal_reference);
	new_node->buffer_size = get_buffer_size(internal_reference);
	new_node->internal_reference = internal_reference;
	memcpy(new_node->user_reference, user_reference, 32);
	new_node->next = arreglo[index];
	arreglo[index] = new_node;
	//free(new_node);

	return SUCCESS;
}

struct hash_node* lookup(char* user_reference) {
	int index = hash(user_reference);
	struct hash_node * temp = arreglo[index];

	while (temp != NULL)
	{
		if (!strcmp(temp->user_reference, user_reference))
			return temp;
		temp = temp->next;
	}
	return NULL;
}

int rem_node(char* user_reference) {
	int index = hash(user_reference);
	struct hash_node * temp = arreglo[index];

	if (temp == NULL)
		return SUCCESS;

	if (arreglo[index]->user_reference == user_reference)
	{
		int result = dev_close(arreglo[index]->internal_reference);
		if (result == 0)
			arreglo[index] = arreglo[index]->next;
		return result;
	}

	while (temp->next != NULL)
	{
		if (temp->next->user_reference == user_reference)
		{
			int result = dev_close(temp->next->internal_reference);
			if (result == 0)
				temp->next = temp->next->next;
			return result;
		}
		temp = temp->next;
	}
	return SUCCESS;
}