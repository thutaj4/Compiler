/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

/* Semantic Records */
struct IdList {
  struct SymEntry * TheEntry;
  struct IdList * Next;
};

struct ExprRes {
  int Reg;
  int b;
  int v;
  char* name;
  struct InstrSeq * Instrs;
};

struct ExprResList {
	struct ExprRes *Expr;
	struct ExprResList * Next;
};

struct BExprRes {
  char * Label;
  struct InstrSeq * Instrs;
};

struct booleans {
	char * name;
        int v;
	struct booleans * next;
};

struct intArr {
	int size;
	char* print;
};


char * enterBoolean(char* name);
void declareArray(char * space);
extern struct InstrSeq * arrayRead(char* name, struct ExprRes* Res1);
extern struct ExprRes* getArrayVal(char * name, struct ExprRes* Res1);
extern struct InstrSeq * arrayAssign(char * name, struct ExprRes* Res1, struct ExprRes* Res2);
/* Semantics Actions */
extern struct ExprRes *  doIntLit(char * digits);
extern struct ExprRes *  doRval(char * name);
extern struct ExprRes * doBool(int x);
extern struct InstrSeq *  doAssign(char * name,  struct ExprRes * Res1);
extern struct IdList * addTOIDList(char * variable, struct IdList * list);
extern struct ExprRes *  doAdd(struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct ExprRes *  doSub(struct ExprRes * Res1, struct ExprRes * Res2);
extern struct ExprRes *  doUSub(struct ExprRes * Res1);
extern struct ExprRes * doNOT(struct ExprRes * Res1);
extern struct ExprRes *  doMult(struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct ExprRes *  doDiv(struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct ExprRes *  doExpo(struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct InstrSeq * whileLOOP(struct ExprRes * Res, struct InstrSeq * seq);
extern struct InstrSeq * forLOOP(struct InstrSeq * assign, struct ExprRes * Res, struct InstrSeq * check, struct InstrSeq * seq); 
extern struct InstrSeq *  doPrint(struct ExprRes * Expr);
extern struct InstrSeq * print(struct ExprResList * Res1);
extern struct ExprResList * addToList(struct ExprRes * Res1, struct ExprResList* Res2);
extern struct InstrSeq * PrintLines(struct ExprRes * Expr);
extern struct InstrSeq * PrintSpaces(struct ExprRes * Expr);
extern struct InstrSeq * Read(struct IdList * list);
extern struct BExprRes * doBExpr (struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct ExprRes * doR (struct ExprRes * Res1,  struct ExprRes * Res2, char* type);
extern struct ExprRes * doAND (struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct ExprRes * doOR (struct ExprRes * Res1,  struct ExprRes * Res2);
extern struct InstrSeq * doIf(struct ExprRes *Res, struct InstrSeq * seq);
extern struct InstrSeq * doIFELSE(struct ExprRes* Res, struct InstrSeq* seq, struct InstrSeq* seq2);
extern struct ExprRes *  doMod(struct ExprRes * Res1,  struct ExprRes * Res2);

extern void	Finish(struct InstrSeq *Code);
