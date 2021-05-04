/* Semantics.c
   Support and semantic action routines.
   
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CodeGen.h"
#include "Semantics.h"
#include "SymTab.h"
#include "IOMngr.h"

extern SymTab *table;
struct booleans * allBools;
int count = 0;

void declareArray(char * size){
	int space = atoi(size);
	struct intArr * temp = (struct intArr *)malloc(sizeof(struct intArr));
	temp->size = space * 4;
//	temp->print = temp->size + '0';
	setCurrentAttr(table, (void*)temp);
}

extern struct InstrSeq * arrayRead(char* name, struct ExprRes* Res1){

	struct InstrSeq* retCode = (struct InstrSeq*)malloc(sizeof(struct InstrSeq));
	int reg = AvailTmpReg();
	char* beg = "0(";
	char* offreg = TmpRegName(reg);
	char* paren = ")";
	char* off = (char*)malloc(4+ strlen(offreg));
	strcpy(off, beg);
	strcat(off, offreg);
	strcat(off, paren);

	AppendSeq(retCode, Res1->Instrs);
	AppendSeq(retCode, GenInstr(NULL, "la", TmpRegName(reg), name, NULL));
	AppendSeq(retCode, GenInstr(NULL, "sll", TmpRegName(Res1->Reg), TmpRegName(Res1->Reg), "2"));
	AppendSeq(retCode, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(Res1->Reg)));
	AppendSeq(retCode, GenInstr(NULL, "li","$v0", "5", NULL));
	AppendSeq(retCode, GenInstr(NULL, "syscall", NULL, NULL, NULL));
	AppendSeq(retCode, GenInstr(NULL, "sw", "$v0", off, NULL));
	
	ReleaseTmpReg(Res1->Reg);
	ReleaseTmpReg(reg);
	free(Res1);
	
	return retCode;
}

struct ExprRes* getArrayVal(char * name, struct ExprRes* Res1){
	struct ExprRes* retRes = (struct ExprRes*)malloc(sizeof(struct ExprRes));
	
	int offset;
	if(!findName(table, name)){
		writeIndicator(getCurrentColumnNum());
		writeMessage("Undeclared Variable");
	}

	retRes->Reg = AvailTmpReg();
	struct InstrSeq* retSeq = (struct InstrSeq*)malloc(sizeof(struct InstrSeq));
	
	char* beg = "0(";
	char* offreg = TmpRegName(retRes->Reg);
	char* paren = ")";
	char* off = (char*)malloc(4+strlen(offreg));
	strcpy(off, beg);
	strcat(off, offreg);
	strcat(off, paren);
	AppendSeq(retSeq, Res1->Instrs);
	AppendSeq(retSeq, GenInstr(NULL, "la", TmpRegName(retRes->Reg), name, NULL));
	AppendSeq(retSeq, GenInstr(NULL, "sll", TmpRegName(Res1->Reg), TmpRegName(Res1->Reg), "2"));
	AppendSeq(retSeq, GenInstr(NULL, "add", TmpRegName(retRes->Reg), TmpRegName(retRes->Reg), TmpRegName(Res1->Reg)));
	AppendSeq(retSeq, GenInstr(NULL, "lw", TmpRegName(retRes->Reg), off, NULL));

	retRes->Instrs = retSeq;
	retRes->name = strdup(name);

	//ReleaseTmpReg(Res1->Reg);
	//free(Res1);

	return retRes;
	
}

extern struct InstrSeq * arrayAssign(char * name, struct ExprRes* Res1, struct ExprRes* Res2){
	struct InstrSeq * retSeq = (struct InstrSeq *)malloc(sizeof(struct InstrSeq));
	//printf("%s\n", name);
	if(!findName(table, name)){
		writeIndicator(getCurrentColumnNum());
		writeMessage("Undeclared Variable");
	}

	int reg = AvailTmpReg();
	char * first = "0(";
	char * second = TmpRegName(reg);
	char * third = ")";
	char * off = (char*)malloc(4 + strlen(second));
	strcpy(off, first);
	strcat(off, second);
	strcat(off, third);

	AppendSeq(retSeq, Res1->Instrs);
	AppendSeq(retSeq, GenInstr(NULL, "la", TmpRegName(reg), name, NULL));
	AppendSeq(retSeq, GenInstr(NULL, "sll", TmpRegName(Res1->Reg), TmpRegName(Res1->Reg), "2"));
	AppendSeq(retSeq, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(Res1->Reg)));
	AppendSeq(retSeq, Res2->Instrs);
	AppendSeq(retSeq, GenInstr(NULL, "sw", TmpRegName(Res2->Reg), off, NULL));

	ReleaseTmpReg(Res1->Reg);
	ReleaseTmpReg(Res2->Reg);
	ReleaseTmpReg(reg);
	free(Res1);
	free(Res2);

	return retSeq;
	
}

char * enterBoolean(char* name){
	if (count == 0){
		allBools = (struct booleans *) malloc(sizeof(struct booleans));
		allBools->name = strdup(name);
		allBools->v = 0;
		allBools->next = NULL;
	} else {
			struct booleans * tmp = allBools;
			int g = 0;
			while(tmp->next != NULL){
				if(strcmp(tmp->name, name) == 0){
					g = 1;
					break;
				}
				tmp = tmp->next;
			}
			if (g == 0){
				struct booleans * insert = (struct booleans*)malloc(sizeof(struct booleans));
				insert->name = strdup(name);
				insert->v = 0;
				insert->next = NULL;
				tmp->next = insert;
			}
	}
	count = count + 1;
	return name;
}


struct ExprRes *  doIntLit(char * digits)  { 

   struct ExprRes *res;
  
  res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res->Reg = AvailTmpReg();
  res->b = 0;
  res->v = 0;
  res->Instrs = GenInstr(NULL,"li",TmpRegName(res->Reg),digits,NULL);

  return res;
}

struct InstrSeq * whileLOOP(struct ExprRes * Res, struct InstrSeq * seq){
	char * start = GenLabel();
	char * end = GenLabel();
	struct InstrSeq * result = (struct InstrSeq *) malloc(sizeof(struct InstrSeq));

	AppendSeq(result, GenInstr(start, NULL, NULL, NULL, NULL));
	AppendSeq(result, Res->Instrs);
	AppendSeq(result, GenInstr(NULL, "beq", "$zero", TmpRegName(Res->Reg), end));
	AppendSeq(result, seq);
	AppendSeq(result, GenInstr(NULL, "j", start, NULL, NULL));
	AppendSeq(result, GenInstr(end, NULL, NULL, NULL, NULL));

	ReleaseTmpReg(Res->Reg);
	free(Res);
	return result;
}

struct InstrSeq * forLOOP(struct InstrSeq * assign, struct ExprRes * Res, struct InstrSeq * check, struct InstrSeq * seq){
	struct InstrSeq * result = (struct InstrSeq*) malloc(sizeof(struct InstrSeq));
	char * start = GenLabel();
	char * end = GenLabel();

	AppendSeq(result, assign);
	AppendSeq(result, GenInstr(start, NULL, NULL, NULL, NULL));
	AppendSeq(result, Res->Instrs);
	AppendSeq(result, GenInstr(NULL, "beq", "$zero", TmpRegName(Res->Reg), end));
	AppendSeq(result, seq);
	AppendSeq(result, check);
	AppendSeq(result, GenInstr(NULL, "j", start, NULL, NULL));
	AppendSeq(result, GenInstr(end, NULL, NULL, NULL, NULL));
	ReleaseTmpReg(Res->Reg);
	free(Res);
	return result;
}
 
struct ExprResList * addToList(struct ExprRes * Res1, struct ExprResList * Res2){
	struct ExprResList * n = (struct ExprResList*) malloc(sizeof(struct ExprResList));
	n->Next = Res2;
	n->Expr = Res1;
	return n;		
}

extern struct IdList * addTOIDList(char * variable, struct IdList * list){
	int found = findName(table, variable);
	if (found == 1){
		struct SymEntry * x = table->current;
		struct IdList * t = (struct IdList*) malloc(sizeof(struct IdList));
		t->TheEntry = x;
		t->Next = list;
		return t;	
	} else{
		writeIndicator(getCurrentColumnNum());
		writeMessage("Undeclared Variable");
		return NULL;
	}	
}

struct ExprRes *  doRval(char * name)  { 

   int found = 0;
   int boolVal = 0;
   struct booleans * tmp = allBools;
   while(tmp != NULL){
	if(strcmp(tmp->name, name) == 0){
		found = 1;
                boolVal = tmp->v;
		break;
	}
	tmp = tmp->next;
   }

   struct ExprRes *res;
   if (!findName(table, name)) {
		writeIndicator(getCurrentColumnNum());
		writeMessage("Undeclared variable");
   } 

  if(found == 1){		
  	res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  	res->Reg = AvailTmpReg();
	res->b = 1;
	res->v = boolVal;
	res->name = strdup(name);
  	res->Instrs = GenInstr(NULL,"lw",TmpRegName(res->Reg),name,NULL);
  	return res;
  } else {
  	res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  	res->Reg = AvailTmpReg();
  	res->Instrs = GenInstr(NULL,"lw",TmpRegName(res->Reg),name,NULL);
  	return res;
  }
}

struct ExprRes * doBool(int x){
	struct ExprRes * res;
	res = (struct ExprRes*) malloc(sizeof(struct ExprRes));
	res->Reg = AvailTmpReg();
	res->b = 1;
	if (x != 0){
		res->Instrs = GenInstr(NULL, "lw", TmpRegName(res->Reg), "_T", NULL);
		res->v = 1;
	} else {
		res->Instrs = GenInstr(NULL, "lw", TmpRegName(res->Reg), "_F", NULL);
		res->v = 0;
	}
	return res;
}

struct ExprRes *  doAdd(struct ExprRes * Res1, struct ExprRes * Res2)  { 

   int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"add",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

struct ExprRes * doSub(struct ExprRes* Res1, struct ExprRes* Res2){
	
   int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"sub",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

struct ExprRes * doUSub(struct ExprRes* Res1){	
   int reg;
   char * z = "$zero";
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,GenInstr(NULL,"sub",
                                       TmpRegName(reg),
					z,
                                       TmpRegName(Res1->Reg)));
  ReleaseTmpReg(Res1->Reg);
  Res1->Reg = reg;
  return Res1;
}

struct ExprRes *  doMult(struct ExprRes * Res1, struct ExprRes * Res2)  { 

   int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"mul",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

struct ExprRes *  doDiv(struct ExprRes * Res1, struct ExprRes * Res2)  { 

   int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"div",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}


struct ExprRes *  doMod(struct ExprRes * Res1, struct ExprRes * Res2)  { 

  int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"div",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  

  AppendSeq(Res1->Instrs,GenInstr(NULL,"MFHI",
                                       TmpRegName(reg),
                                       NULL,
                                       NULL));

  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

extern struct ExprRes *  doExpo(struct ExprRes * Res1,  struct ExprRes * Res2){
		
	struct InstSeq* seq3;

	char * label = GenLabel();
	char * ending = GenLabel();
	char * zero = GenLabel();
	int reg = AvailTmpReg();
	int answer = AvailTmpReg();

	AppendSeq(Res1->Instrs, Res2->Instrs);
	AppendSeq(Res1->Instrs, GenInstr(NULL, "addi", TmpRegName(reg), "$zero", "1"));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "addi", TmpRegName(answer), TmpRegName(Res1->Reg), "0"));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "beq", "$zero", TmpRegName(Res2->Reg), zero));
	AppendSeq(Res1->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "beq", TmpRegName(reg), TmpRegName(Res2->Reg), ending));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "mul", TmpRegName(answer), TmpRegName(answer), TmpRegName(Res1->Reg)));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "addi", TmpRegName(reg), TmpRegName(reg), "1"));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "j", label, NULL, NULL));
	AppendSeq(Res1->Instrs, GenInstr(zero, NULL, NULL, NULL, NULL));
	AppendSeq(Res1->Instrs, GenInstr(NULL, "addi", TmpRegName(answer), "$zero", "1"));
	AppendSeq(Res1->Instrs, GenInstr(ending, NULL, NULL, NULL, NULL));
	
	ReleaseTmpReg(Res1->Reg);
	ReleaseTmpReg(Res2->Reg);
	ReleaseTmpReg(answer);
	ReleaseTmpReg(reg);
	Res1->Reg = answer;
	free(Res2);
	return Res1;


}
extern struct ExprRes * doNOT(struct ExprRes * Res1){
 char* n;
 if(Res1->name == NULL){
     n = "";
 } else{
	n = Res1->name;
  }
  struct booleans* tmp = allBools;
  int found = 0;
  if(Res1->b == 1){
   	//int found = 0;
   	int boolVal = 0;
   	while(tmp != NULL){
		if(strcmp(tmp->name, Res1->name) == 0){
			found = 1;
			//printf("found in not\n");
			break;
		}
		tmp = tmp->next;
   	}
  }
	int reg;
   	reg = AvailTmpReg();
	if (found == 1){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "xori", TmpRegName(reg), TmpRegName(Res1->Reg), "1"));
	} else{
	//AppendSeq(Res1->Instrs, GenInstr(NULL, "andi", TmpRegName(reg), TmpRegName(Res1->Reg), "1"));
	//AppendSeq(Res1->Instrs, GenInstr(NULL, "subu", TmpRegName(reg), "$zero", TmpRegName(Res1->Reg)));
		AppendSeq(Res1->Instrs, GenInstr(NULL, "not", TmpRegName(reg), TmpRegName(Res1->Reg), NULL));
	}
	ReleaseTmpReg(Res1->Reg);
	if(Res1->b == 1){
		if(Res1->v != 0){
			Res1->v = 0;
			//printf("changed %d\n", Res1->v);
		} else {
			Res1->v = 1;
		}
	}
	Res1->Reg = reg;
	ReleaseTmpReg(reg);
	return Res1;
}


extern struct ExprRes * doAND (struct ExprRes * Res1,  struct ExprRes * Res2){
	
   int reg;
      
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"and",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  if(Res1->b == 1 || Res2->b == 1){
	Res1->b == 1;
	Res1->v = Res1->v && Res2->v;
  }
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

extern struct ExprRes * doOR (struct ExprRes * Res1,  struct ExprRes * Res2){
	
   int reg;
 
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"or",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  if(Res1->b == 1 && Res2->b == 1){
	Res1->b == 1;
	Res1->v = Res1->v || Res2->v;
  }
  Res1->Reg = reg;
  free(Res2);
  return Res1;
}

struct InstrSeq * PrintLines(struct ExprRes * Expr){
	struct InstrSeq * code;

	code = Expr->Instrs;

	char * label = GenLabel();
	char * ending = GenLabel();
	char * zero = GenLabel();
	int reg = AvailTmpReg();
	int answer = AvailTmpReg();

//	AppendSeq(code, Res2->Instrs);
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(reg), "$zero", "0"));
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(answer), TmpRegName(Expr->Reg), "0"));
	//AppendSeq(Res1->Instrs, GenInstr(NULL, "beq", "$zero", TmpRegName(Res2->Reg), zero));
	AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
	AppendSeq(code, GenInstr(NULL, "beq", TmpRegName(reg), TmpRegName(Expr->Reg), ending));
	AppendSeq(code, GenInstr(NULL, "li", "$v0", "4", NULL));
	AppendSeq(code, GenInstr(NULL, "la", "$a0", "_nl", NULL));
   	AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(reg), TmpRegName(reg), "1"));
	AppendSeq(code, GenInstr(NULL, "j", label, NULL, NULL));
	AppendSeq(code, GenInstr(zero, NULL, NULL, NULL, NULL));
	//AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(answer), "$zero", "1"));
	AppendSeq(code, GenInstr(ending, NULL, NULL, NULL, NULL));
	
	ReleaseTmpReg(Expr->Reg);
	ReleaseTmpReg(answer);
	ReleaseTmpReg(reg);
	return code;
}


struct InstrSeq * PrintSpaces(struct ExprRes * Expr){
	struct InstrSeq * code;

	code = Expr->Instrs;

	char * label = GenLabel();
	char * ending = GenLabel();
	char * zero = GenLabel();
	int reg = AvailTmpReg();
	int answer = AvailTmpReg();

//	AppendSeq(code, Res2->Instrs);
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(reg), "$zero", "1"));
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(answer), TmpRegName(Expr->Reg), "0"));
	//AppendSeq(Res1->Instrs, GenInstr(NULL, "beq", "$zero", TmpRegName(Res2->Reg), zero));
	AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
	AppendSeq(code, GenInstr(NULL, "beq", TmpRegName(reg), TmpRegName(Expr->Reg), ending));
	AppendSeq(code, GenInstr(NULL, "li", "$v0", "4", NULL));
	AppendSeq(code, GenInstr(NULL, "la", "$a0", "_s", NULL));
   	AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
	AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(reg), TmpRegName(reg), "1"));
	AppendSeq(code, GenInstr(NULL, "j", label, NULL, NULL));
	AppendSeq(code, GenInstr(zero, NULL, NULL, NULL, NULL));
	//AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(answer), "$zero", "1"));
	AppendSeq(code, GenInstr(ending, NULL, NULL, NULL, NULL));
	
	ReleaseTmpReg(Expr->Reg);
	ReleaseTmpReg(answer);
	ReleaseTmpReg(reg);
	return code;
}

struct InstrSeq * print(struct ExprResList * Res1){
	struct InstrSeq * code;
	struct InstrSeq * retCode = (struct InstrSeq*) malloc(sizeof(struct InstrSeq));
	while (Res1 != NULL){
		code = Res1->Expr->Instrs;
		//printf("printing name%s bool%d val%d\n", Res1->Expr->name, Res1->Expr->b, Res1->Expr->v);
		   int found = 0;
		   int boolVal = 0;
		   struct booleans * tmp = allBools;
		   //printf("print name %s", Res1->Expr->name);
		   while(tmp != NULL){
			if(strcmp(tmp->name, Res1->Expr->name) == 0){
				found = 1;
		                boolVal = tmp->v;
				break;
			}
			tmp = tmp->next;
		   }
		if (found == 1){
			//struct InstrSeq * seq2;
			char * label = GenLabel();
			char * skip = GenLabel();
			
						
  			AppendSeq(code, GenInstr(NULL,"lw",TmpRegName(Res1->Expr->Reg),Res1->Expr->name,NULL));
			AppendSeq(code, GenInstr(NULL, "beq", "$zero", TmpRegName(Res1->Expr->Reg), label));
			
			AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    			AppendSeq(code,GenInstr(NULL,"la","$a0","_TRUE",NULL));
			//seq2 = AppendSeq(Res->Instrs, seq);
			AppendSeq(code, GenInstr(NULL, "j", NULL, NULL, skip));
			AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
			AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    			AppendSeq(code,GenInstr(NULL,"la","$a0","_FALSE",NULL));
			//AppendSeq(code, seq3);
			AppendSeq(code, GenInstr(skip, NULL, NULL, NULL, NULL));
			//free(Res);
			//printf("%s", tmp->name);
			//if(boolVal == 1){
			//	AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    			//	AppendSeq(code,GenInstr(NULL,"la","$a0","_TRUE",NULL));
			//} else {
			//	AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    			//	AppendSeq(code,GenInstr(NULL,"la","$a0","_FALSE",NULL));
			//}
    		} else {	
			AppendSeq(code,GenInstr(NULL,"li","$v0","1",NULL));
    			AppendSeq(code,GenInstr(NULL,"move","$a0",TmpRegName(Res1->Expr->Reg),NULL));
		}
		
		AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

    		AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    		AppendSeq(code,GenInstr(NULL,"la","$a0","_s",NULL));
    		AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
		
		AppendSeq(retCode, code);
		ReleaseTmpReg(Res1->Expr->Reg);
		//free the struct?
		Res1 = Res1->Next;
		
	}

		
    	//AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    	//AppendSeq(code,GenInstr(NULL,"la","$a0","_nl",NULL));
    	//AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

	return retCode;
}

extern struct InstrSeq * Read(struct IdList * list){
	struct InstrSeq * retCode = (struct InstrSeq*) malloc(sizeof(struct InstrSeq));
	while(list != NULL){
		AppendSeq(retCode, GenInstr(NULL, "li","$v0", "5", NULL));
		AppendSeq(retCode, GenInstr(NULL, "syscall", NULL, NULL, NULL));
		AppendSeq(retCode, GenInstr(NULL, "sw", "$v0", list->TheEntry->name, NULL));
		//ReleaseTmpReg(list->Expr->Reg);		
		list = list->Next;
	}	

	return retCode;
}

struct InstrSeq * doPrint(struct ExprRes * Expr) { 

  struct InstrSeq *code;
    
  code = Expr->Instrs;
  
    AppendSeq(code,GenInstr(NULL,"li","$v0","1",NULL));
    AppendSeq(code,GenInstr(NULL,"move","$a0",TmpRegName(Expr->Reg),NULL));
    AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

    AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
    AppendSeq(code,GenInstr(NULL,"la","$a0","_nl",NULL));
   AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

    ReleaseTmpReg(Expr->Reg);
    free(Expr);

  return code;
}

struct InstrSeq * doAssign(char *name, struct ExprRes * Expr) { 
   //printf("before assign charname=%s %d %s\n", name, Expr->v, Expr->name);
   int found = 0;
   int boolVal = 0;
   struct booleans * tmp = allBools;
   while(tmp != NULL){
	if(strcmp(tmp->name, name) == 0){
		found = 1;
     //           printf("found in assign\n");
		break;
	} 
	tmp = tmp->next;
   }

  if (found == 1){
	//printf("assign expr%d tmp%d\n", Expr->v, tmp->v);
	tmp->v = Expr->v;
  }
  struct InstrSeq *code;
  
   if (!findName(table, name)) {
		writeIndicator(getCurrentColumnNum());
		writeMessage("Undeclared variable");
   }

  code = Expr->Instrs;
    
  AppendSeq(code,GenInstr(NULL,"sw",TmpRegName(Expr->Reg), name,NULL));

  ReleaseTmpReg(Expr->Reg);
  free(Expr);
  
  return code;
}

extern struct BExprRes * doBExpr(struct ExprRes * Res1,  struct ExprRes * Res2) {
	struct BExprRes * bRes;
	AppendSeq(Res1->Instrs, Res2->Instrs);
 	bRes = (struct BExprRes *) malloc(sizeof(struct BExprRes));
	bRes->Label = GenLabel();
	AppendSeq(Res1->Instrs, GenInstr(NULL, "bne", TmpRegName(Res1->Reg), TmpRegName(Res2->Reg), bRes->Label));
	bRes->Instrs = Res1->Instrs;
	ReleaseTmpReg(Res1->Reg);
  	ReleaseTmpReg(Res2->Reg);
	free(Res1);
	free(Res2);
	return bRes;
}


extern struct ExprRes * doR (struct ExprRes * Res1,  struct ExprRes * Res2, char* type){
	struct ExprRes * Res;
	int reg = AvailTmpReg();
	AppendSeq(Res1->Instrs, Res2->Instrs);
	Res = (struct ExprRes*) malloc(sizeof(struct ExprRes));
		
	if(strcmp(type, ">") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "sgt", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	} else if(strcmp(type, "<") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "slt", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	} else if(strcmp(type, ">=") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "sge", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	} else if(strcmp(type, "<=") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "sle", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	} else if(strcmp(type, "!=") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "sne", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	} else if(strcmp(type, "==") == 0){
		AppendSeq(Res1->Instrs, GenInstr(NULL, "seq", TmpRegName(reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
	}

	Res->Reg = reg;
	Res->Instrs = Res1->Instrs;
	ReleaseTmpReg(Res1->Reg);
	ReleaseTmpReg(Res2->Reg);
	free(Res1);
	free(Res2);
	return Res;
}
/*
extern struct InstrSeq * doIf(struct BExprRes * bRes, struct InstrSeq * seq) {
	struct InstrSeq * seq2;
	seq2 = AppendSeq(bRes->Instrs, seq);
	AppendSeq(seq2, GenInstr(bRes->Label, NULL, NULL, NULL, NULL));
	free(bRes);
	return seq2;
}
*/
extern struct InstrSeq * doIf(struct ExprRes * Res, struct InstrSeq * seq){
	struct InstrSeq * seq2;
	char * label = GenLabel();
	AppendSeq(Res->Instrs, GenInstr(NULL, "beq", "$zero", TmpRegName(Res->Reg), label));
	seq2 = AppendSeq(Res->Instrs, seq);
	AppendSeq(seq2, GenInstr(label, NULL, NULL, NULL, NULL));
	ReleaseTmpReg(Res->Reg);
	free(Res);
	return seq2;
}

extern struct InstrSeq * doIFELSE(struct ExprRes* Res, struct InstrSeq* seq, struct InstrSeq* seq3){	
	struct InstrSeq * seq2;
	char * label = GenLabel();
	char * skip = GenLabel();

	AppendSeq(Res->Instrs, GenInstr(NULL, "beq", "$zero", TmpRegName(Res->Reg), label));
	seq2 = AppendSeq(Res->Instrs, seq);
	AppendSeq(seq2, GenInstr(NULL, "j", NULL, NULL, skip));
	AppendSeq(seq2, GenInstr(label, NULL, NULL, NULL, NULL));
	AppendSeq(seq2, seq3);
	AppendSeq(seq2, GenInstr(skip, NULL, NULL, NULL, NULL));
	ReleaseTmpReg(Res->Reg);
	free(Res);
	return seq2;
}
/*

extern struct InstrSeq * doIf(struct ExprRes *res1, struct ExprRes *res2, struct InstrSeq * seq) {
	struct InstrSeq *seq2;
	char * label;
	label = GenLabel();
	AppendSeq(res1->Instrs, res2->Instrs);
	AppendSeq(res1->Instrs, GenInstr(NULL, "bne", TmpRegName(res1->Reg), TmpRegName(res2->Reg), label));
	seq2 = AppendSeq(res1->Instrs, seq);
	AppendSeq(seq2, GenInstr(label, NULL, NULL, NULL, NULL));
	ReleaseTmpReg(res1->Reg);
  	ReleaseTmpReg(res2->Reg);
	free(res1);
	free(res2);
	return seq2;
}

*/
void							 
Finish(struct InstrSeq *Code)
{ struct InstrSeq *code;
  //struct SymEntry *entry;
    int hasMore;
  struct Attr * attr;


  code = GenInstr(NULL,".text",NULL,NULL,NULL);
  //AppendSeq(code,GenInstr(NULL,".align","2",NULL,NULL));
  AppendSeq(code,GenInstr(NULL,".globl","main",NULL,NULL));
  AppendSeq(code, GenInstr("main",NULL,NULL,NULL,NULL));
  AppendSeq(code,Code);
  AppendSeq(code, GenInstr(NULL, "li", "$v0", "10", NULL)); 
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(code,GenInstr(NULL,".data",NULL,NULL,NULL));
  AppendSeq(code,GenInstr(NULL,".align","4",NULL,NULL));
  AppendSeq(code,GenInstr("_nl",".asciiz","\"\\n\"",NULL,NULL));
  AppendSeq(code,GenInstr("_s",".asciiz","\" \" ",NULL,NULL));
 
 // AppendSeq(code,GenInstr("_TRUE",".asciiz","\"true\"",NULL,NULL));
 // AppendSeq(code,GenInstr("_FALSE",".asciiz","\"false\"",NULL,NULL));

 hasMore = startIterator(table);
 while (hasMore) {
    struct intArr* t = (struct intArr*)malloc(sizeof(struct intArr));
    t = (struct intArr*)getCurrentAttr(table);
    if(t != NULL){
	char x[100];
	sprintf(x, "%d", t->size);	
	//AppendSeq(code, GenInstr("", "", "", NULL, NULL));
	AppendSeq(code, GenInstr((char*)getCurrentName(table), ".space", x, NULL, NULL));
    } else{
    	AppendSeq(code,GenInstr((char *) getCurrentName(table),".word","0",NULL,NULL));
    }
    hasMore = nextEntry(table);
 }

  AppendSeq(code,GenInstr("_TRUE",".asciiz","\"true\"",NULL,NULL));
  AppendSeq(code,GenInstr("_FALSE",".asciiz","\"false\"",NULL,NULL));
 AppendSeq(code, GenInstr((char *)"_T", ".word", "1", NULL, NULL)); 
 AppendSeq(code, GenInstr((char *)"_F", ".word", "0", NULL, NULL));
  WriteSeq(code);
  
  return;
}




