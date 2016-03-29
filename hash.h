#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "device.h"

struct hash_node{
	char name[32];
	int buffer_size;
	int block_count;
	int format_status;
	int internal_reference;
	char user_reference[32];
	struct hash_node *next;
};

#define size_of_arreglo	13

unsigned int hash(char* reference);
int init();
int add(char* name, char* user_reference);
struct hash_node* lookup(char* user_reference);
int rem_node(char* user_reference);

struct hash_node* get_first(int index);

#endif // HASHTABLE_H