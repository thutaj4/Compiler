# Compiler
Implements a slightly modified C compiler and is able to produce MIPS assembly language. 

Uses LEX and YACC for defining regular expressions, symbol table for storing variables, and an IO manager for errors/messages. 

Able to do
  - integer expressions
  - boolean expressions
  - integer arrays (not dynamically allocated)
  - control structures
    - if/else statements
    - while loops
    - for loops (must include semicolon after incrementing variable)
  - all variables must be declared at the top of the file before use
  
  See example runs/output. 
  
  Must have LEX and YACC installed to run.
  
  To run the code:
    yacc -d ExprEval.y
    lex lex1.l
    gcc -o {name} lex.yy.c y.tab.c SymTab.c Semantics.c CodeGen.c IOMngr.c main.c
