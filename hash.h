#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hash_node{
	char name[32];
	int buffer_size;
	int block_count;
	int internal_reference;
	char user_reference[32];
	struct hash_node *next;
};

#define size_of_arreglo	13
struct hash_node *arreglo[size_of_arreglo];

unsigned int hash(char* reference);
int init();
int add(char* name, char* user_reference);
struct hash_node* lookup(char* user_reference);
int rem_node(char* user_reference);

#endif // HASHTABLE_H