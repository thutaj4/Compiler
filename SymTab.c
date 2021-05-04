#include "SymTab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//creates a new symbol table that has a prime number as the size
SymTab* createSymTab(int size){
	SymTab* newTable = (SymTab*)malloc(sizeof(SymTab));
	newTable->size = nextPrime(size);
	newTable->contents = (SymEntry**)malloc(sizeof(SymEntry) * size);
	newTable->current = NULL;
	return newTable;
}

//destroys the symbol table
void destroySymTab(SymTab* table){
	int i;
	for(i = 0; i < table->size; i++){
		if(table->contents[i] != NULL){
			SymEntry* temp;
			//delete all contents of the table in the current row
			while(table->contents[i] != NULL){	
				temp = table->contents[i];
				table->contents[i] = table->contents[i]->next;
				temp->name = NULL;
				free(temp->name);
				free(temp);
			}
		}
		table->contents[i] = NULL;
		free(table->contents[i]);
	}
	table->contents = NULL;
	//free the remaining space of contents
	free(table->contents);
	table = NULL;
	free(table);
}

//finds the next available prime if value entered is not prime
int nextPrime(int size){
	
	if(size <= 1) return 2;
	if(size == 2 || size == 3) return size;
	if(size % 2 == 0) size++;

	int found = 0;
	while(found == 0){
		if(validPrime(size) == 1){
			found = 1;
		} else {
			size += 2;
		}
	}

	return size;
}

//checks if x is a prime
int validPrime(int x){
	
	if(x <= 1) return 0;
	if(x <= 3) return 1;

	if(x % 2 == 0 || x % 3 == 0) return 0;

	int i;
	for(i = 5; i * i <= x; i += 6){
		if(x % i == 0 || x % (i + 2) == 0) return 0;
	}

	return 1;

	
}

//enter a name into the symbol table
int enterName(SymTab* table, char* name){
	//return 0 if the name is alreay in the table
	if(findName(table, name) == 1) return 0;
//	printf("%s entered\n", name);
	//get the hash value for the name
	char* hashName = strdup(name);
	int hashVal = gethash(hashName) % table->size;
	SymEntry* newEntry = (SymEntry*)malloc(sizeof(SymEntry));

	//set all values for the symentry
	newEntry->name = hashName;
	newEntry->attribute = NULL;
	newEntry->next = table->contents[hashVal];

	table->contents[hashVal] = newEntry;
	table->current = newEntry;

	return 1;
}

//determine a hash value for the input
int gethash(char* name){
	int size = strlen(name);
	int i;
	int hashVal = 0;
	for(i = 0; i < size; i++){
		hashVal += name[i];
	}
	
	return hashVal;
}

//return 1 if the name is found in the table, otherwise 0
int findName(SymTab* table, char* name){
	//get the hash value for the name
	int hashVal = gethash(name) % table->size;
	SymEntry* start = table->contents[hashVal];

	if(start == NULL) return 0;

	while(start != NULL){
		//loop over all symentries at that hash value, return 1 if found
		if(strcmp(start->name, name) == 0){
			table->current = start;
			return 1;
		}
		start = start->next;
	}

	return 0;
}

//iterates over the contents of the symbol table
int startIterator(SymTab* table){

	if(table->current == NULL) return 0;
	
	int i;
	for(i = 0; i < table->size; i++){
		if(table->contents[i] != NULL){
			table->current = table->contents[i];
			return 1;
		}
	}

	return 0;
}

int nextEntry(SymTab* table){
	//return 0 if there are no entries
	if(table->current == NULL) return 0;
	
	//return the next entry at the current hashvalue if there is a next entry
	if(table->current->next != NULL){
		table->current = table->current->next;
		return 1;
	} else {
		//get current hash value of the name
		int hashVal = gethash(table->current->name) % table->size;
		int i;
		//go to the next hashvalue and start iterating over contents at that hashvalue
		for(i = hashVal + 1; i < table->size; i++){
			if(table->contents[i] != NULL){
				table->current = table->contents[i];
				return 1;
			}
		}

		return 0;
	}
}

//check if we have a current
int hasCurrent(SymTab* table){
	return table->current == NULL ? 0 : 1;
}

//set the current attribute
void setCurrentAttr(SymTab* table, void* attr){
	table->current->attribute = attr;
}

//get the current attribute
void* getCurrentAttr(SymTab* table){
	return table->current->attribute;
}

//get the name of the entry
char* getCurrentName(SymTab* table){
	return table->current->name;
}
