#include "IOMngr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define TRUE 1
#define FALSE 0

FILE* listingFile = NULL;
FILE* sourceFile = NULL;
int listingValid = 0;
char line[MAXLINE + 1];
int curLine = 0;
int curCol = 0;
int prevLine = 0;

//open the source file and listing file if provided one
int openFiles(char* sourceName, char* listingName){
	if(listingName != NULL){
		listingValid = 1;
	}

	//open listing file is provided one
	if(listingValid == 1){
		listingFile = fopen(listingName, "w+");
		if(listingFile == NULL){
			return 0;
		}
	}

	//open the source file
	sourceFile = fopen(sourceName, "r");
	if(sourceFile == NULL){
		return 0;
	}

	return 1;
}

//close the source file and listing file if provided one
void closeFiles(){
	fclose(sourceFile);
	if(listingValid != 0) fclose(listingFile);
}

//gets the next char in the source file
char getNextSourceChar(){
	//if we are at the end of the line, set current column to 0
	if(line[curCol + 1] == '\0'){
		curCol = 0;
	}

	//if we are at the start of a line
	if(curCol == 0){
		curLine++;
		memset(line, 0, sizeof(line));

		//return EOF if we are at the end of the file
		if(fgets(line, MAXLINE, sourceFile) == NULL){
			return EOF;
		}

		//print to the listing file
		if(listingValid == 1){
			fprintf(listingFile, "%d", curLine);
			fputs(": ", listingFile);
			fputs(line, listingFile);	
		}
	}
	//return the next char in the file
	return line[curCol++];
}

//prints the current line to the listing file is provided, otherwise stdout
void writeIndicator(int column){	
		if(listingValid == 1){
			int i;
			for(i = 0; i < column + getOffSet(getCurrentLineNum()); i++){
				fprintf(listingFile, "%s", " ");
			}
			fprintf(listingFile, "%s", "^\n");
		} else {
			int i;
			if(prevLine != curLine){
				printf("%d: %s", curLine, line);
				prevLine = curLine;
			}
			for(i = 0; i < column + getOffSet(getCurrentLineNum()); i++){
				printf(" ");
			}
			printf("^\n");
		}
}

//returns the number of spaces to start printing 
int getOffSet(int line){
	int num = 0;
	while(line != 0){
		num++;
		line = line / 10;
	}	
	return num + 1;
}

int getCurrentLineNum(){
	return curLine;
}

int getCurrentColumnNum(){
	return curCol;
}

//right the message to the listing file or stdout
void writeMessage(char* message){
	if(listingValid == 1){
		fprintf(listingFile, "%s\n", message);
	} else {
		printf("%s\n", message);
	}
}
