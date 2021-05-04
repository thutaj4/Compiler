%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IOMngr.h"
#include "SymTab.h"
#include "Semantics.h"
#include "CodeGen.h"

extern int yylex();	/* The next token function. */
extern char *yytext;   /* The matched token text.  */
extern int yyleng;      /* The token text length.   */
extern int yyparse();
extern int yyerror(char *);
void dumpTable();
char * num;
char* t;

extern SymTab *table;

%}


%union {
  long val;
  char * string;
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct ExprResList * ExprResList;
  struct IdList * IdList;
//  struct BExprRes * BExprRes;
}

%type <string> Id
%type <ExprRes> Factor
%type <ExprRes> Term
%type <ExprRes> Expr
%type <InstrSeq> StmtSeq
%type <InstrSeq> Stmt
%type <ExprRes> BExpr
%type <ExprResList> ExprList
%type <ExprRes> RExpr
%type <ExprRes> Expo
%type <IdList> IDList

%token Ident 		
%token IntLit 	
%token Int
%token Write
%token printlines
%token printspaces
%token IF
%token EQ
%token GE
%token LE
%token NE
%token AND
%token OR	
%token READ
%token ELSE
%token WHILE
%token FOR
%token TRUE
%token FALSE
%token Bool

%%

Prog		:	Declarations StmtSeq							{Finish($2); } ;
Declarations	:	Dec Declarations							{ };
Declarations	:										{ };
Dec		:	Int Id {enterName(table, $2); }';'				{ };
Dec             :       Bool Ident {enterName(table, enterBoolean(yytext));} ';'                { };
Dec             :       Int Id {enterName(table,$2);} '[' IntLit {num = strdup(yytext);} ']' ';' {declareArray(num);};
StmtSeq 	:	Stmt StmtSeq								{$$ = AppendSeq($1, $2); } ;
StmtSeq		:										{$$ = NULL;} ;
//Stmt		:	Write Expr ';'								{$$ = doPrint($2); };
Stmt            :       Write '(' ExprList ')' ';'                                              {$$ = print($3);};
Stmt            :       printlines '(' Expr ')' ';'                                             {$$ = PrintLines($3);};
Stmt            :       printspaces '(' Expr ')' ';'                                            {$$ = PrintSpaces($3);};
Stmt            :       READ '(' IDList ')' ';'                                                 {$$ = Read($3);};
Stmt            :       READ '(' Id '[' BExpr ']' ')' ';'                                       {$$ = arrayRead($3, $5);};
Stmt		:	Id '=' BExpr ';'							{$$ = doAssign($1, $3);};
Stmt            :       Id '['BExpr']' '=' BExpr ';'                                            {$$ = arrayAssign($1, $3, $6);};
Stmt            :       WHILE '(' BExpr ')' '{' StmtSeq '}'                                     {$$ = whileLOOP($3,$6);};
Stmt            :       FOR '(' Stmt  BExpr ';' Stmt ')' '{' StmtSeq '}'                        {$$ = forLOOP($3, $4, $6, $9);};
Stmt            :       IF '(' BExpr ')' '{' StmtSeq '}' ELSE '{' StmtSeq '}'                   {$$ = doIFELSE($3, $6, $10);};
Stmt		:	IF '(' BExpr ')' '{' StmtSeq '}'					{$$ = doIf($3, $6);};               
ExprList        :       BExpr                                                                   {$$ = addToList($1, NULL);};
ExprList        :       BExpr ',' ExprList                                                      {$$ = addToList($1, $3);};
IDList          :       Id                                                                      {$$ = addTOIDList($1, NULL);};
IDList          :       Id ',' IDList                                                           {$$ = addTOIDList($1, $3);};
BExpr           :       BExpr OR RExpr                                                          {$$ = doOR($1, $3);};
BExpr           :       BExpr AND RExpr                                                         {$$ = doAND($1, $3);};
BExpr           :       RExpr                                                                   {$$ = $1;};
RExpr           :       Expr '>' Expr                                                           {$$ = doR($1, $3, (char*)">");};
RExpr           :       Expr '<' Expr                                                           {$$ = doR($1, $3, (char*)"<");};
RExpr           :       Expr LE Expr                                                            {$$ = doR($1, $3, (char*)"<=");};
RExpr           :       Expr GE Expr                                                            {$$ = doR($1, $3, (char*)">=");};
RExpr		:	Expr EQ Expr								{$$ = doR($1, $3, (char*)"==");};
RExpr           :       Expr NE Expr                                                            {$$ = doR($1, $3, (char*)"!=");};
RExpr           :       Expr                                                                    {$$ = $1;};
Expr            :	Expr '-' Term                                                           {$$ = doSub($1, $3);}
Expr		:	Expr '+' Term								{$$ = doAdd($1, $3); } ;
Expr		:	Term									{$$ = $1; } ;
Term            :       Term '/' Factor                                                         {$$ = doDiv($1,$3);}
Term		:	Term '*' Factor								{$$ = doMult($1, $3); } ;
Term            :       Term '%' Factor                                                         {$$ = doMod($1, $3);}
Term		:	Factor									{$$ = $1; } ;
Factor          :       Expo '^' Factor                                                         {$$ = doExpo($1, $3);};
Factor          :       Expo                                                                    {$$ = $1;};
Expo            :       '!' Expo                                                                {$$ = doNOT($2);};
Expo            :       '-' Expo                                                                {$$ = doUSub($2);};
Expo            :       '(' BExpr ')'                                                           {$$ = $2;};
Expo		:	IntLit									{$$ = doIntLit(yytext); };
Expo            :       Id '[' BExpr ']'                                                        {$$ = getArrayVal($1, $3);};
Expo		:	Id									{$$ = doRval($1); };
Expo            :       TRUE                                                                    {$$ = doBool(1);};
Expo            :       FALSE                                                                   {$$ = doBool(0);};
//Expo            :       Id '[' BExpr ']'                                                        {$$ = getArrayVal($1, $3);};
Id		: 	Ident									{$$ = strdup(yytext);}
 
%%

int yyerror(char *s)  {
  writeIndicator(getCurrentColumnNum());
  writeMessage("Illegal Character in YACC");
  return 1;
}
