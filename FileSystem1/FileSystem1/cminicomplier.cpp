
#include "cminicomplier.h"

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#ifndef _GLOBALS_H_
#define _GLOBALS_H_



#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif // !TRUE

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 8

typedef enum {
	/*book-keeping tokens*/
	ENDFILE, ERROR_,
	/*reserved words*/
	ELSE, IF, INT, RETURN, VOID_, WHILE,
	/*multicaracter tokens*/
	ID, NUM,
	/*special symbols*/
	PLUS, //  + 
	MINUS, //  -
	MULTIPLY, //  * 
	DIVIDE, //  /
	LESSTHAN, //  <
	LESSOREQUAL, //  <=
	GREATERTHAN, //  >
	GREATEROREQUAL, //  >=
	EEQUAL, //  ==
	UNEQUAL, //  !=
	EQUAL, //  =
	SEMI, //  ;
	COMMA, //  ,
	LPAREN, //  (
	RPAREN, //  )
	LMPAREN, //  [
	RMPAREN, //  ]
	LLPAREN, //  {
	RLPAREN, //  }
	LNOTE, //  /*
	RNOTE  //  */
}TokenType;

extern FILE* source; /* source code text file */
extern FILE* listing;	/* listing output text file */
extern FILE* code;	/* code text file for TM simulator */

extern int lineno;	/* source line number for listing */

/*****************************************************************/
/****************** Syntax tree for parsing **********************/
/*****************************************************************/

typedef enum { StmtK, ExpK, DeclK, ParamK, TypeK }NodeKind;
typedef enum { AssignK, CompK, IfK, IterK, RetK }StmtKind;
typedef enum { OpK, ConstK, IdK, ArrIdK, CallK }ExpKind;
typedef enum { FuncK, VarK, ArrVarK }DeclKind;//函数声明或变量声明
typedef enum { ArrParamK, NonArrParamK }ParamKind;
typedef enum { TypeNameK }TypeKind;

/* ArrayAttr is used for attributes for array variables */
typedef struct arrayAttr {
	TokenType type;
	char* name;
	int size;
}ArrayAttr;


typedef enum { Void, Integer, Boolean, IntegerArray }ExpType;

#define MAXCHILDREN 3

struct ScopeRec;

typedef struct treeNode
{
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	int lineno;
	NodeKind nodeKind;
	union {
		StmtKind stmt; ExpKind exp;
		DeclKind decl; ParamKind param;
		TypeKind type;
	}kind;
	union {
		TokenType op;
		TokenType type;
		int val;
		char* name;
		ArrayAttr arr;
		struct ScopeRec* scope;
	}attr;
	ExpType type;	/* for type checking of exps */
	bool isInFuncCom;  // if isInFuncCom == true stack in
	bool isAInt; // like  a[x] is true , a is false

}TreeNode;


/*****************************************************************/
/******************    Flags for tracing    **********************/
/*****************************************************************/

/* EchoSource = TRUE causes the source porgram to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TracePrase;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif

#pragma once
#ifndef _PARSE_H_
#define _PARSE_H_

/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void);

#endif

#pragma once
#ifndef _SCAN_H_
#define _SCAN_H_


/* MAXTOKENLEN is the maximum size of a token */
#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

/*  function getToken returns the
*	next token in source file
*/
TokenType getToken(void);

#endif

#pragma once
#ifndef _SYMTAB_H_
#define _SYMTAB_H_



/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec
{
	int lineno;
	struct LineListRec* next;
}*LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
{
	char* name = NULL;
	LineList lines = NULL;
	TreeNode* treeNode = NULL;
	int memloc = 0;	/* memory location for variable */
	struct BucketListRec* next = NULL;
}*BucketList;

typedef struct ScopeRec
{
	char* funcName = 0;
	int nestedLevel = 0;
	struct ScopeRec* parent = 0;
	BucketList hashTable[SIZE] = { 0 };	/* the hash table */
}*Scope;

extern Scope globalScope;

/* Procedure st_insert inserts line numbers and
 * memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char* name, int lineno, int loc, TreeNode* treeNode);

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup(char* name);
int st_add_lineno(char* name, int lineno);
BucketList st_bucket(char* name);
int st_lookup_top(char* name);

Scope sc_create(char* funcName);
Scope sc_top(void);
void sc_pop(void);
void sc_push(Scope scope);
int addLocation(void);
int addLocation(int size);

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE* listing);
#endif

#pragma once

#ifndef _UTIL_H_
#define UTIL_H_

/* Procedure printToken prints a token
* and tis lexeme to the listing file
*/
void printToken(TokenType, const char*);

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind);

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind);

/* Function newParamNode creates a new declaration
 * node for syntax tree construction
 */
TreeNode* newDeclNode(DeclKind);

/* Function newParamNode creates a new parameter
 * node for syntax tree construction
 */
TreeNode* newParamNode(ParamKind);

/* Function newTypeNode creates a new type
 * node for syntax tree construction
 */
TreeNode* newTypeNode(TypeKind);

/* Funtion copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char*);

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode*);
#endif



#pragma once
/************************************************/
/*File name:code.h								*/
/*code emitting utilities for the TINY compiler	*/
/*and interface to the TM machine				*/
/************************************************/

#ifndef _CODE_H_
#define _CODE_H_

/*pc = program counter*/
#define pc 7

/*mp = "memory pointer" points
 *to the top of memory(for temp storage)*/
 /*fp = "frame pointer*/
#define fp 6

/*gp = "global pointer" points
 *to bottom of memory for (global) variable storage*/
#define gp 5

 /*accumulator*/
#define ac 0

/*2nd accumulator*/
#define ac1 1

#define MAX_GLOBAL_SIZE 256

/* code emitting utilities */

/*Procedure emitComment prints a comment line
 *with comment c in the code file
 */
void emitComment(const char* c);

/*Procdure emitRO emits a register-only TM instruction
 *op=the opcode
 *r=target register
 *s=1st source register
 *t=2nd source register
 *c=acomment to be printed if TraceCode is TRUE
 */
void emitRO(const char* op, int r, int s, int t, const char* c);

/*Procdure emitRM emits a register-to-memory TM instruction
 *op=the opcode
 *r=target register
 *d=the offset
 *s=the base register
 *c=acomment to be printed if TraceCode is TRUE
 */
void emitRM(const char* op, int r, int d, int s, const char* c);

/*Function emitSkip skips "howMany" code
 *location for later backpach.It also
 *returns the current code position
 */
int emitSkip(int howMany);

/*Procedure emitBackup backs up to
 *loc = a previously skips location
 */
void emitBackup(int loc);

/*Procedure emitRestore restores the current
 *code position to the highest previously
 *unemitted position
 */
void emitRestore(void);

/*Procedure emitRM_Abs convert an absolute reference
 *to a pc-relative reference when emmiting a
 *register-to-memory TM instruction
 *op=the opcode
 *r=target register
 *a=an absolute location in memory
 *c=a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs(const char* op, int r, int a, const char* c);

#endif

#pragma once
/****************************************************/
/*File name :cgen.h									*/
/*The code generator interface to the TINY compiler	*/
/****************************************************/

#ifndef _CGEN_H_
#define _CGEN_H_

/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode* syntaxTree, char* codefile);

#endif 


#pragma once

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode*);

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode*);

#endif



int complier(char* data, char* filename);



/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */


 //----------------------------------------analyze.cpp---------------------------------------//
  /* allocate global variables */
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

/* allocate and set tracing flags */
int EchoSource = TRUE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

static char* funcName;
static int preserveLastScope = FALSE;


/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode* t,
	void (*preProc)(TreeNode*),
	void (*postProc)(TreeNode*))
{
	if (t != NULL)
	{
		preProc(t);
		{
			int i;
			for (i = 0; i < MAXCHILDREN; i++)
				traverse(t->child[i], preProc, postProc);

		}
		postProc(t);
		traverse(t->sibling, preProc, postProc);
	}
}

static void insertIOFunc(void)
{
	TreeNode* func;
	TreeNode* typeSpec;
	TreeNode* param;
	TreeNode* compStmt;

	func = newDeclNode(FuncK);

	typeSpec = newTypeNode(TypeNameK);
	typeSpec->attr.type = INT;
	func->type = Integer;

	compStmt = newStmtNode(CompK);
	compStmt->child[0] = NULL;	// no local var
	compStmt->child[1] = NULL;	// no stmt

	func->lineno = 0;
	char* funcName = new char[10];

	strcpy(funcName, "input");
	func->attr.name = funcName;
	func->child[0] = typeSpec;
	func->child[1] = NULL;	// no param
	func->child[2] = compStmt;

	char* tempName = new char[10];
	strcpy(tempName, "input");

	st_insert(tempName, -1, addLocation(), func);

	func = newDeclNode(FuncK);

	typeSpec = newTypeNode(TypeNameK);
	typeSpec->attr.type = VOID_;
	func->type = Void;

	param = newParamNode(NonArrParamK);
	char* tempName2 = new char[10];
	strcpy(tempName2, "arg");
	func->attr.name = tempName2;
	param->child[0] = newTypeNode(TypeNameK);
	param->child[0]->attr.type = INT;

	compStmt = newStmtNode(CompK);
	compStmt->child[0] = NULL;
	compStmt->child[1] = NULL;

	func->lineno = 0;
	char* tempName3 = new char[10];
	strcpy(tempName3, "output");
	func->attr.name = tempName3;
	func->child[0] = typeSpec;
	func->child[1] = param;
	func->child[2] = compStmt;

	char* tempName1 = new char[10];
	strcpy(tempName1, "output");
	st_insert(tempName1, -1, addLocation(), func);
}

static void symbolError(TreeNode* t, const char* message)
{
	fprintf(listing, "Symbol error at line %d: %s\n", t->lineno, message);
	Error = TRUE;
}


/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode* t)
{
	if (t == NULL)return;
	else return;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode* t)
{
	switch (t->nodeKind)
	{
	case StmtK:
		switch (t->kind.stmt)
		{
		case CompK:
			if (preserveLastScope) {
				preserveLastScope = FALSE;
			}
			else {
				if (t->isInFuncCom)
				{
					Scope scope = sc_create(funcName);
					sc_push(scope);
				}

			}
			t->attr.scope = sc_top();
			break;
		default:
			break;

		}
		break;
	case ExpK:
		switch (t->kind.exp)
		{

		case ArrIdK:
			if (st_lookup(t->attr.arr.name) == -1)
				/* not yet in table , error */
			{
				Scope nowScope = sc_top();
				symbolError(t, "undelcared symbol");
			}

			else
				/* already in table, so ignore location
				   add line number of use only */
				st_add_lineno(t->attr.arr.name, t->lineno);
			break;

		case IdK:
		case CallK:
			/* not yet in table, so treat as new definition */
			if (st_lookup(t->attr.name) == -1)
				/* not yet in table , error */
			{
				Scope nowScope = sc_top();
				symbolError(t, "undelcared symbol");
			}

			else
				/* already in table, so ignore location
				   add line number of use only */
				st_add_lineno(t->attr.name, t->lineno);
			break;
		default:
			break;
		}
		break;
	case DeclK:
		switch (t->kind.decl)
		{
		case FuncK:
			funcName = t->attr.name;
			if (st_lookup_top(funcName) >= 0) {
				/* already in table, so it's an error */
				symbolError(t, "function already declared");
				break;
			}


			st_insert(funcName, t->lineno, addLocation(), t);
			sc_push(sc_create(funcName));
			preserveLastScope = TRUE;
			switch (t->child[0]->type)
			{
			case Integer:
				t->type = Integer;
				break;
			case Void:
			default:
				t->type = Void;
				break;
			}
			break;
		case VarK:
		case ArrVarK:
		{
			char* name;

			if (t->child[0]->attr.type == VOID_) {
				symbolError(t, "variable should have non-void type");
				break;
			}
			if (t->kind.decl == VarK) {
				name = t->attr.name;
			}
			else {
				name = t->attr.arr.name;

			}

			if (st_lookup_top(name) < 0)
			{

				if (t->type == Integer)
				{
					st_insert(name, t->lineno, addLocation(), t);
				}
				else if (t->type == IntegerArray)
				{
					st_insert(name, t->lineno, addLocation(t->attr.arr.size), t);
				}

			}
			else
				symbolError(t, "symbol already declared for current scope");
		}
		break;
		default:
			break;

		}
		break;
	case ParamK:
		if (t->child[0]->attr.type == VOID_)
			symbolError(t->child[0], "void type parameter is not allowed");
		else if (t->kind.param == NonArrParamK)
		{
			if (st_lookup(t->attr.name) == -1) {
				st_insert(t->attr.name, t->lineno, addLocation(), t);
				if (t->kind.param == NonArrParamK)
					t->type = Integer;
				else
					symbolError(t, "symbol already declared for current scope");
			}
		}
		else if (t->kind.param == ArrParamK)
		{
			if (st_lookup(t->attr.arr.name) == -1) {
				st_insert(t->attr.arr.name, t->lineno, addLocation(), t);
				/*
				if (t->kind.param == NonArrParamK)
					t->type = Integer;
				else
					symbolError(t, "symbol already declared for current scope");
				*/
			}
		}

		break;
	default:
		break;
	}

}

static void afterInsertNode(TreeNode* t)
{
	switch (t->nodeKind)
	{
	case StmtK:
		switch (t->kind.stmt)
		{
		case CompK:
			if (t->isInFuncCom)
			{
				sc_pop();
			}

			break;
		default:
			break;
		}
	}
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode* syntaxTree)
{
	globalScope = sc_create(NULL);
	sc_push(globalScope);
	insertIOFunc();
	traverse(syntaxTree, insertNode, afterInsertNode);
	sc_pop();
	if (TraceAnalyze)
	{
		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode* t, const char* message)
{
	fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
	Error = TRUE;
}

static void beforeCheck(TreeNode* t)
{
	switch (t->nodeKind)
	{
	case DeclK:
		switch (t->kind.decl)
		{
		case FuncK:
			funcName = t->attr.name;
			break;
		default:
			break;
		}
		break;
	case StmtK:
		switch (t->kind.stmt)
		{
		case CompK:
			sc_push(t->attr.scope);
			break;
		default:
			break;
		}
	default:
		break;
	}
}



/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode* t)
{
	switch (t->nodeKind)
	{
	case StmtK:
		switch (t->kind.stmt)
		{
		case AssignK:
			if (t->child[0]->child[0]->type == IntegerArray)
				/* no value can be assigned to array variable */
				typeError(t->child[0], "assignment to array variable");
			else if (t->child[1]->type == Void)
				/* r-value cannot have void type */
				typeError(t->child[0], "assignment of void value");
			else
				t->type = t->child[0]->type;
			break;
		case CompK:
			sc_pop();
			break;
		case IterK:
			if (t->child[0]->type == Void)
				/* while test should be void function call */
				typeError(t->child[0], "while test has void value");
			break;
		case RetK:
		{
			const TreeNode* funcDecl =
				st_bucket(funcName)->treeNode;
			const ExpType funcType = funcDecl->type;
			const TreeNode* expr = t->child[0];

			if (funcType == Void &&
				(expr != NULL && expr->type != Void)) {
				typeError(t, "expected no return value");
				//ValueReturned = TRUE;
			}
			else if (funcType == Integer &&
				(expr == NULL || expr->type == Void)) {
				typeError(t, "expected return vaule");
			}
		}
		break;
		default:
			break;
		}
		break;
	case ExpK:
		switch (t->kind.exp)
		{

		case OpK:
		{
			ExpType leftType, rightType;
			TokenType op;

			leftType = t->child[0]->type;
			rightType = t->child[1]->type;
			op = t->attr.op;

			if (leftType == Void ||
				rightType == Void)
				typeError(t, "two operands should have non-void type");
			else if (leftType == IntegerArray &&
				rightType == IntegerArray)
				typeError(t, "not both of operands can be array");
			else if (op == MINUS &&
				leftType == Integer &&
				rightType == IntegerArray)
				typeError(t, "invalid operands to binary expression");
			else if ((op == MULTIPLY || op == DIVIDE) &&
				(leftType == IntegerArray ||
					rightType == IntegerArray))
				typeError(t, "invalid operands to binary expression");
			else {
				t->type = Integer;
			}
		}
		break;
		case ConstK:
			t->type = Integer;
			break;
		case IdK:
		case ArrIdK:
		{
			char* symbolName;
			if (t->kind.exp == IdK)
			{
				symbolName = t->attr.name;
			}
			else
			{
				symbolName = t->attr.arr.name;
			}
			const BucketList bucket =
				st_bucket(symbolName);
			TreeNode* symbolDecl = NULL;

			if (bucket == NULL)
				break;
			symbolDecl = bucket->treeNode;

			if (t->type == IntegerArray) {
				if (symbolDecl->type != IntegerArray)
					typeError(t, "expected array symbol");
				else if (t->child[1] != NULL && t->child[1]->type != Integer)
					typeError(t, "index expression should have integer type");
				/*
				else
					t->type = Integer;
				*/
			}
			/*
			else {
				t->type = symbolDecl->type; ////////////////////////////////////////////////////////////maybe change int to arrint
			}
			*/

		}
		break;
		case CallK:
		{
			char* callingFuncName = t->attr.name;

			BucketList funcBucketList = st_bucket(callingFuncName);
			if (funcBucketList == NULL)
				break;
			TreeNode* funcDecl =
				funcBucketList->treeNode;
			TreeNode* arg;
			TreeNode* param;

			if (funcDecl == NULL)
				break;

			arg = t->child[1];
			param = funcDecl->child[1];

			if (funcDecl->kind.decl != FuncK)
			{
				typeError(t, "expected function symbol");
				break;
			}

			while (arg != NULL)
			{
				if (param == NULL)
					/* the number of arguments does not match to
						that of parameters */
					typeError(arg, "the number of parameters is wrong");
				/*else if (arg->type == IntergerArray &&
					param->type!=IntegerArray)
				  typeError(arg,"expected non-array value");
				  else if(arg->type==Integer &&
					param->type==IntegerArray)
				 typeError(arg,"expected array value");*/
				else if (arg->type == Void)
					typeError(arg, "void value cannot be passed as an argument"); \
				else { // no problem! all right!
					arg = arg->sibling;
					param = param->sibling;
					continue;
				}
				/* any problem */
				break;

			}
			if (arg == NULL && param != NULL)
				/* the number of arguments does not match to
					that of parameters */
				typeError(t->child[0], "the number of parameters is wrong");

			//t->type = funcDecl->type;  ////////////maybe change int to arrint
		}
		break;
		default:
			break;
		}
		break;
	default:
		break;
		break;
	}
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode* syntaxTree)
{
	sc_push(globalScope);
	traverse(syntaxTree, beforeCheck, checkNode);
	sc_pop();
}

//----------------------------------------analyze.cpp---------------------------------------//


//----------------------------------------cgen.cpp---------------------------------------//
/****************************************************/
/* File: cgen.cpp                                   */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/****************************************************/


static char buffer[1000];
#define ofpFO 0
#define retFO -1
#define initFO -2

static int globalOffset = 0;
static int frameoffset = initFO;

/* numOfParams is the number of parameters in current frame */
static int numOfParams = 0;

/* isInFunc is the flag that shows if current node
 * is in a function block. This flag is used when
 * calculating localOffset of a function declaration.
 */
static int isInFunc = FALSE;

/* mainFuncLoc is the location of main() function */
static int mainFuncLoc = 0;

/* prototype for internal recursive code generator */
static void cGen(TreeNode* tree);

/* Function getBlockOffset returns the offset for
 * temp variables in the block where list is */
static int getBlockOffset(TreeNode* list) {
	int offset = 0;
	if (list == NULL) {
		//offset=0;
	}
	else if (list->nodeKind == DeclK) {
		/*local variable declaration*/
		TreeNode* node = list;
		while (node != NULL) {
			switch (node->kind.decl) {
			case VarK:
				++offset;
				break;
			case ArrVarK:
				offset += node->attr.arr.size;
				break;
			default:
				break;
			}
			node = node->sibling;
		}
	}
	else if (list->nodeKind == ParamK) {
		/* parameter declaration */
		TreeNode* node = list;
		while (node != NULL) {
			++offset;
			node = node->sibling;
		}
	}
	return offset;
}



/* Procedure genExp generates code at an expression node */
static void genExp(TreeNode* tree, int lhs)
{
	int loc;
	int varOffset, baseReg;
	int numOfArgs;
	TreeNode* p1, * p2;
	switch (tree->kind.exp) {

	case OpK:
		//printToken(tree->attr.op, "\0");
		if (TraceCode) emitComment("-> Op");

		p1 = tree->child[0];
		p2 = tree->child[1];

		/* gen code for ac = left arg */
		cGen(p1);
		/* gen code to push left operand */
		emitRM("ST", ac, frameoffset--, fp, "op: push left");

		/* gen code for ac = right operand */
		cGen(p2);
		/* now load left operand */
		emitRM("LD", ac1, ++frameoffset, fp, "op: load left");

		switch (tree->attr.op) {

		case PLUS:
			emitRO("ADD", ac, ac1, ac, "op +");
			break;
		case MINUS:
			emitRO("SUB", ac, ac1, ac, "op -");
			break;
		case MULTIPLY:
			emitRO("MUL", ac, ac1, ac, "op *");
			break;
		case DIVIDE:
			emitRO("DIV", ac, ac1, ac, "op /");
			break;
		case LESSTHAN:
			emitRO("SUB", ac, ac1, ac, "op <");
			emitRM("JLT", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case LESSOREQUAL:
			emitRO("SUB", ac, ac1, ac, "op <=");
			emitRM("JLE", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case GREATERTHAN:
			emitRO("SUB", ac, ac1, ac, "op >");
			emitRM("JGT", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case GREATEROREQUAL:
			emitRO("SUB", ac, ac1, ac, "op >=");
			emitRM("JGE", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case EEQUAL:
			emitRO("SUB", ac, ac1, ac, "op ==");
			emitRM("JEQ", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case UNEQUAL:
			emitRO("SUB", ac, ac1, ac, "op !=");
			emitRM("JNE", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		default:
			emitComment("BUG: Unknown operator ");

			//fprintf(code, "%5s", tree->attr.name);
			break;
		} /* case op */

		if (TraceCode)  emitComment("<- Op");

		break; /* OpK */

	case ConstK:
		if (TraceCode) emitComment("-> Const");
		/* gen code to load integer constant using LDC */
		emitRM("LDC", ac, tree->attr.val, 0, "load const");
		if (TraceCode)  emitComment("<- Const");
		break; /* ConstK */

	case IdK:
	case ArrIdK:
		char* IdName;
		if (tree->kind.exp == IdK)
		{
			IdName = tree->attr.name;
		}
		else
		{
			IdName = tree->attr.arr.name;
		}
		if (TraceCode) {
			sprintf(buffer, "-> Id (%s)", IdName);
			emitComment(buffer);
		}

		loc = st_lookup_top(IdName);
		if (loc >= 0)
			varOffset = initFO - loc;
		else
		{
			varOffset = -(st_lookup(IdName));
		}


		/* generate code to load varOffset */
		emitRM("LDC", ac, varOffset, 0, "id: load varOffset");

		if (tree->kind.exp == ArrIdK) {
			/* kind of node is for array id */

			if (loc >= 0 && loc < numOfParams) {

				/* generate code to push address */
				emitRO("ADD", ac, fp, ac, "id: load the memory address of base address of array to ac");
				emitRO("LD", ac, 0, ac, "id: load the base address of array to ac");
			}
			else {
				/* global or local variable */

				/* generate code for address */
				if (loc >= 0)
				{
					/* symbol found in current frame */
					emitRO("ADD", ac, fp, ac, "id: calculate the address");
				}
				else
				{
					/* symbol found in global scope */
					emitRO("ADD", ac, gp, ac, "id: calculate the address");

				}
			}

			/* generate code to push localOffset */
			emitRM("ST", ac, frameoffset--, fp, "id: push base address");

			/* generate code for index expression */
			p1 = tree->child[1];
			if (p1 != NULL)
			{
				cGen(p1);
				/* gen code to get correct varOffset */
				emitRM("LD", ac1, ++frameoffset, fp, "id: pop base address");
				emitRO("SUB", ac, ac1, ac, "id: calculate element address with index");
			}
			else
			{
				emitRM("LD", ac1, ++frameoffset, fp, "id: pop base address");
			}

		}
		else {
			/* kind of node is for non-array id */

			/* generate code for address */
			if (loc >= 0)
			{
				/* symbol found in current frame */
				emitRO("ADD", ac, fp, ac, "id: calculate the address found in current frame");
			}
			else
			{
				/* symbol found in global scope */
				emitRO("ADD", ac, gp, ac, "id: calculate the address found in global scope");

			}
		}

		if (lhs) {
			emitRM("LDA", ac, 0, ac, "load id address");
		}
		else {
			emitRM("LD", ac, 0, ac, "load id value");
		}

		if (TraceCode)  emitComment("<- Id");

		break; /* IdK, ArrIdK */

	case CallK:
		if (TraceCode) emitComment("-> Call");

		/* init */
		numOfArgs = 0;

		p1 = tree->child[1];

		/* for each argument */
		while (p1 != NULL) {                                    //*****************************************
			/* generate code for argument expression */         //*****************************************
			if (p1->type == IntegerArray)                       //*****************************************
				genExp(p1, TRUE);                               //*****************************************
			else                                                //*****************************************
				genExp(p1, FALSE);

			/* generate code to push argument value */
			emitRM("ST", ac, frameoffset + initFO - (numOfArgs++), fp,
				"call: push argument");

			p1 = p1->sibling;
		}

		if (strcmp(tree->attr.name, "input") == 0) {
			/* generate code for input() function */
			emitRO("IN", ac, 0, 0, "read integer value");
		}
		else if (strcmp(tree->attr.name, "output") == 0) {
			/* generate code for output(arg) function */
			/* generate code for value to write */
			emitRM("LD", ac, frameoffset + initFO, fp, "load arg to ac");
			/* now output it */
			emitRO("OUT", ac, 0, 0, "write ac");
		}
		else {
			/* generate code to store current mp */
			emitRM("ST", fp, frameoffset + ofpFO, fp, "call: store current mp");
			/* generate code to push new frame */
			emitRM("LDA", fp, frameoffset, fp, "call: push new frame");
			/* generate code to save return in ac */
			emitRM("LDA", ac, 1, pc, "call: save return in ac");

			/* generate code to relative-jump to function entry */
			loc = -(st_lookup(tree->attr.name));
			emitRM("LD", pc, loc, gp, "call: relative jump to function entry");

			/* generate code to pop current frame */
			emitRM("LD", fp, ofpFO, fp, "call: pop current frame");
		}

		if (TraceCode)  emitComment("<- Call");

		break; /* CallK */

	default:
		break;
	}
} /* genExp */

/* Procedure genStmt generates code at a statement node */
static void genStmt(TreeNode* tree) {
	TreeNode* p1, * p2, * p3;
	int savedLoc1, savedLoc2, currentLoc;
	int Loc;
	int offset;
	switch (tree->kind.stmt) {

	case AssignK:
		if (TraceCode) emitComment("-> assign");

		p1 = tree->child[0];
		p2 = tree->child[1];

		// generate code for ac = address of lhs
		genExp(p1, TRUE);
		// generate code to push lhs
		emitRM("ST", ac, frameoffset--, fp, "assign: push left (address)");

		// generate code for ac = rhs
		cGen(p2);
		// now load lhs
		emitRM("LD", ac1, ++frameoffset, fp, "assign: load left (address)");

		emitRM("ST", ac, 0, ac1, "assign: store value");

		if (TraceCode) emitComment("<- assign");
		break; // assign_k

	case CompK:
		if (TraceCode)emitComment("->compound");

		p1 = tree->child[0];
		p2 = tree->child[1];

		/* update localOffset with the offset derived from declarations */
		offset = getBlockOffset(p1);
		frameoffset -= offset;

		/* push scope */
		if (tree->isInFuncCom)
		{
			sc_push(tree->attr.scope);
		}


		/* generate code for body */
		cGen(p2);

		/* pop scope */
		if (tree->isInFuncCom)
		{
			sc_pop();
		}


		/* restore localOffset */
		frameoffset -= offset;

		if (TraceCode)emitComment("<-compound");

		break;

	case IfK:
		if (TraceCode)emitComment("->if");

		p1 = tree->child[0];
		p2 = tree->child[1];
		p3 = tree->child[2];

		/* generate code for test expression */
		cGen(p1);

		savedLoc1 = emitSkip(1);
		emitComment("if:jump to else belongs here");

		/* recurse on then part */
		cGen(p2);

		savedLoc2 = emitSkip(1);
		emitComment("if:jump to end belongs here");

		currentLoc = emitSkip(0);
		emitBackup(savedLoc1);
		emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
		emitRestore();

		/* recurse on else part */
		cGen(p3);
		currentLoc = emitSkip(0);
		emitBackup(savedLoc2);
		emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
		emitRestore();
		if (TraceCode)  emitComment("<- if");

		break; /* select_k */

	case IterK:
		if (TraceCode) emitComment("-> iter.");

		p1 = tree->child[0];
		p2 = tree->child[1];

		savedLoc1 = emitSkip(0);
		emitComment("while: jump after body comes back here");

		/* generate code for test expression */
		cGen(p1);

		savedLoc2 = emitSkip(1);
		emitComment("while: jump to end belongs here");

		/* generate code for body */
		cGen(p2);
		emitRM_Abs("LDA", pc, savedLoc1, "while: jmp back to test");
		/* backpatch */
		currentLoc = emitSkip(0);
		emitBackup(savedLoc2);
		emitRM_Abs("JEQ", ac, currentLoc, "while: jmp to end");
		emitRestore();

		if (TraceCode)  emitComment("<- iter.");

		break; /* iter_k */

	case RetK:
		if (TraceCode) emitComment("-> return");

		p1 = tree->child[0];

		/* generate code for expression */
		cGen(p1);
		emitRM("LD", pc, retFO, fp, "return: to caller");

		if (TraceCode) emitComment("<- return");

		break; /* return_k */
	default:
		break;
	}
}/* genStmt */

/* Procedure genDecl generates code at a declaration node */
static void genDecl(TreeNode* tree)
{
	TreeNode* p1, * p2;
	int loadFuncLoc, jmpLoc, funcBodyLoc, nextDeclLoc;
	int loc;
	int size;

	switch (tree->kind.decl) {
	case FuncK:
		if (TraceCode) {
			sprintf(buffer, "-> Function (%s)", tree->attr.name);
			emitComment(buffer);
		}

		p1 = tree->child[1];
		p2 = tree->child[2];

		isInFunc = TRUE;

		/* generate code to store the location of func. entry */
		loc = -(st_lookup(tree->attr.name));
		loadFuncLoc = emitSkip(1);
		emitRM("ST", ac1, loc, gp, "func: store the location of func. entry");
		/* decrease global offset by 1 */
		--globalOffset;

		/* generate code to unconditionally jump to next declaration */
		jmpLoc = emitSkip(1);
		emitComment("func: unconditional jump to next declaration belongs here");

		/* skip code generation to allow jump to here
		   when the function was called */
		funcBodyLoc = emitSkip(0);
		emitComment("func: function body starts here");

		/* backpatch */
		emitBackup(loadFuncLoc);
		emitRM("LDC", ac1, funcBodyLoc, 0, "func: load function location");
		emitRestore();

		/* generate code to store return address */
		emitRM("ST", ac, retFO, fp, "func: store return address");

		/* calculate localOffset and numOfParams */
		frameoffset = initFO;
		numOfParams = 0;
		cGen(p1);

		/* generate code for function body */
		if (strcmp(tree->attr.name, "main") == 0)
			mainFuncLoc = funcBodyLoc;

		cGen(p2);

		/* generate code to load pc with return address */
		emitRM("LD", pc, retFO, fp, "func: load pc with return address");

		/* backpatch */
		nextDeclLoc = emitSkip(0);
		emitBackup(jmpLoc);
		emitRM_Abs("LDA", pc, nextDeclLoc, "func: unconditional jump to next declaration");
		emitRestore();

		isInFunc = FALSE;

		if (TraceCode) {
			sprintf(buffer, "-> Function (%s)", tree->attr.name);
			sprintf(buffer, " frameoffset (%d) ", frameoffset);
			emitComment(buffer);
		}

		break;

	case VarK:
	case ArrVarK:
		if (TraceCode) emitComment("-> var. decl.");

		if (tree->kind.decl == ArrVarK)
			size = tree->attr.arr.size;
		else
			size = 1;

		if (isInFunc == TRUE)
			frameoffset -= size;
		else
			globalOffset -= size;

		if (TraceCode) emitComment("<- var. decl.");
		break;

	default:
		break;
	}
} /* genDecl */

/* Procedure genParam generates code at a declaration node */
static void genParam(TreeNode* tree)
{
	switch (tree->kind.stmt) {

	case ArrParamK:
		if (TraceCode) emitComment("-> param");
		emitComment(tree->attr.arr.name);

		--frameoffset;
		++numOfParams;

		if (TraceCode) emitComment("<- param");
		break;
	case NonArrParamK:
		if (TraceCode) emitComment("-> param");
		emitComment(tree->attr.name);

		--frameoffset;
		++numOfParams;

		if (TraceCode) emitComment("<- param");

		break;

	default:
		break;
	}
} /* genParam */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen(TreeNode* tree)
{
	if (tree != NULL)
	{
		switch (tree->nodeKind) {
		case StmtK:
			genStmt(tree);
			break;
		case ExpK:
			genExp(tree, FALSE);
			break;
		case DeclK:
			genDecl(tree);
			break;
		case ParamK:
			genParam(tree);
			break;
		default:
			break;
		}
		cGen(tree->sibling);
	}
}

void genMainCall() {
	emitRM("LDC", ac, globalOffset, 0, "init: load globalOffset");
	emitRO("ADD", fp, fp, ac, "init: initialize mp with globalOffset");

	if (TraceCode) emitComment("-> Call");

	/* generate code to store current mp */
	emitRM("ST", fp, ofpFO, fp, "call: store current mp");
	/* generate code to push new frame */
	emitRM("LDA", fp, 0, fp, "call: push new frame");
	/* generate code to save return in ac */
	emitRM("LDA", ac, 1, pc, "call: save return in ac");

	/* generate code for unconditional jump to function entry */
	emitRM("LDC", pc, mainFuncLoc, 0, "call: unconditional jump to main() entry");

	/* generate code to pop current frame */
	emitRM("LD", fp, ofpFO, fp, "call: pop current frame");

	if (TraceCode) emitComment("<- Call");
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode* syntaxTree, char* codefile)
{
	char* s = (char*)malloc(strlen(codefile) + 7);
	strcpy(s, "File: ");
	strcat(s, codefile);
	emitComment("TINY Compilation to TM Code");
	emitComment(s);
	/* generate standard prelude */
	emitComment("Standard prelude:");
	emitRM("LD", gp, 0, ac, "load gp with maxaddress");
	emitRM("LDA", fp, -MAX_GLOBAL_SIZE, gp, "copy gp to mp");
	emitRM("ST", ac, 0, ac, "clear location 0");
	emitComment("End of standard prelude.");
	/* push global scope */
	sc_push(globalScope);
	/* generate code for TINY program */
	cGen(syntaxTree);
	/* pop global scope */
	sc_pop();
	/* call main() function */
	genMainCall();
	/* finish */
	emitComment("End of execution.");
	emitRO("HALT", 0, 0, 0, "");
}

//-----------------------------------------cgen.cpp------------------------------------------

//-----------------------------------------code.cpp------------------------------------------
/************************************************/
/*File name code.cpp							*/
/*TM code emitting utilities implementation		*/
/*for the TINY compiler							*/
/************************************************/


/*TM location number for current instruction emission*/
static int emitLoc = 0;

/*Highest TM location emitted so far
 *For use in conjunction with emitSkip,
 *emitBackup,and emitRestore
 */
static int highEmitLoc = 0;

/*Procedure emitComment prints a comment line
 *with comment c in the code file
 */
void emitComment(const char* c) {
	if (TraceCode)fprintf(code, "*%s\n", c);
}

/*Procdure emitRO emits a register-only TM instruction
 *op=the opcode
 *r=target register
 *s=1st source register
 *t=2nd source register
 *c=acomment to be printed if TraceCode is TRUE
 */
void emitRO(const char* op, int r, int s, int t, const char* c) {
	fprintf(code, "%3d:   %5s   %d,%d,%d", emitLoc++, op, r, s, t);
	if (TraceCode)fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)highEmitLoc = emitLoc;
}

/*Procdure emitRM emits a register-to-memory TM instruction
 *op=the opcode
 *r=target register
 *d=the offset
 *s=the base register
 *c=acomment to be printed if TraceCode is TRUE
 */
void emitRM(const char* op, int r, int d, int s, const char* c) {
	fprintf(code, "%3d:  %5s  %d,%d(%d) ", emitLoc++, op, r, d, s);
	if (TraceCode) fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc;
}

/*Function emitSkip skips "howMany" code
 *location for later backpach.It also
 *returns the current code position
 */
int emitSkip(int howMany) {
	int i = emitLoc;
	emitLoc += howMany;
	if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc;
	return i;
}

/*Procedure emitBackup backs up to
 *loc = a previously skips location
 */
void emitBackup(int loc) {
	if (loc > highEmitLoc)emitComment("BUG in emitBackup");
	emitLoc = loc;
}

/*Procedure emitRestore restores the current
 *code position to the highest previously
 *unemitted position
 */
void emitRestore(void) {
	emitLoc = highEmitLoc;
}

/*Procedure emitRM_Abs convert an absolute reference
 *to a pc-relative reference when emmiting a
 *register-to-memory TM instruction
 *op=the opcode
 *r=target register
 *a=an absolute location in memory
 *c=a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs(const char* op, int r, int a, const char* c) {
	fprintf(code, "%3d:   %5s %d,%d(%d)", emitLoc, op, r, a - (emitLoc + 1), pc);
	++emitLoc;
	if (TraceCode)fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)highEmitLoc = emitLoc;
}

//-----------------------------------------code.cpp------------------------------------------


//-----------------------------------------parse.cpp------------------------------------------

/*
DeclNode:
	FuncK:
		attr.name = ID
		type = Void | Integer
		child[0] = TypeNode(TypeNameK) : type = Void | Integer
		child[1] = ParamNode | NULL
		child[2] = StmtNode(CompK) | NULL
		sibling = DeclNode | NULL
	VarK:
		attr.name = ID
		type = Void | Integer
		child[0] = TypeNode(TypeNameK) : type = Void | Integer
		sibling = DeclNode | NULL
	ArrVarK:
		attr.arr.name = ID
		attr.arr.size = NUM
		type = Void | Integer
		child[0] = TypeNode(TypeNameK) : type = Void | Integer
		sibling = DeclNode | NULL

ParamNode:
	ArrParamK:
		attr.arr.name = ID
		type = IntegerArray
		child[0] = TypeNode(TypeNameK) : type = IntegerArray
		sibling = ParamNode | NULL
	NonArrParamK:
		attr.name = ID
		type = Integer
		child[0] = TypeNode(TypeNameK) : type = Integer
		sibling = ParamNode | NULL

StmtNode:
	AssignK:
		child[0] = ExpNode(IdK)
		child[1] = ExpNode
		sibling = StmtNode | NULL
	CompK:
		child[0] = DeclNode( VarK | ArrVarK ) | NULL
		child[1] = StmtNode | NULL
		sibling = NULL
	IfK:
		child[0] = ExpNode(OpK)
		child[1] = StmtNode
		child[2] = StmtNode | NULL
		sibling = StmtNode | NULL
	IterK:
		child[0] = ExpNode(OpK)
		child[1] = StmtNode
		sibling = StmtNode | NULL
	RetK:
		child[0] = ExpNode | NULL

ExpNode:
	OpK:
		attr.op = + - * / < <= >= > == !=
		child[0] = ExpNode
		child[1] = ExpNode
		sibling = NULL
	ConstK:
		attr.val = NUM
		type = Integer
		child[0] = TypeNode(TypeNameK) : type = Integer
		sibling = NULL
	IdK:
		attr.name = ID
		type = Integer
		child[0] = TypeNode(TypeNameK) : type = Integer
		sibling = NULL
	ArrIdK:
		attr.arr.name = ID
		type = IntegerArray
		child[0] = TypeNode(TypeNameK) : type = IntegerArray
		child[1] = ExpNode
		sibling = NULL
	CallK:
		attr.name = ID
		type = Integer | Void
		child[0] = TypeNode(TypeNameK) : type = Integer | Void
		child[1] = ExpNode | NULL
		sibling = NULL

TypeNode:
	TypeNameK:
		type = ExpType

*/

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* declaration_list(void); //声明列表
static TreeNode* declaration(void); //声明
static TreeNode* var_declaration(void); //整型变量声明
static TreeNode* arrvar_declaration(void); //整型数组变量声明
static TreeNode* fun_declaration(void); //函数声明
static TreeNode* params(void); //函数内参数
static TreeNode* param_list(void); //参数列表
static TreeNode* param(void); //参数
static TreeNode* compound_stmt(void); //复合语句
static TreeNode* local_declarations(void); //局部变量声明
static TreeNode* statement_list(void); //语句列表
static TreeNode* statement(void); //语句
static TreeNode* assignment_stmt(void); //赋值语句
static TreeNode* selection_stmt(void); //if语句
static TreeNode* iteration_stmt(void); //while语句
static TreeNode* return_stmt(void); //return语句
static TreeNode* expression(void); //表达式
static TreeNode* additive_expression(void); //算数表达式
static TreeNode* term(void);
static TreeNode* factor(void);
static TreeNode* args(void); //函数参数
static TreeNode* arg_list(void); //函数参数列表

static void syntaxError(const char* message)
{
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;
}

/* 匹配当前token，若匹配，获取下一个token，若不匹配，输出错误信息 */
static void match(TokenType expected)
{
	if (token == expected) token = getToken();
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "        ");
	}
}

/* 2. declaration_list -> declaration_list declaration | declaration */
TreeNode* declaration_list(void) /* declaration_list 声明列表 */
{
	TreeNode* t = declaration();
	TreeNode* p = t;
	while (token != ENDFILE)
	{
		TreeNode* q;
		q = declaration();
		if (q != NULL) {
			if (t == NULL)
				t = p = q;
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

/* 3. declaration -> var_declaration | arrvar_declaration | fun_declaration */
TreeNode* declaration(void)
{
	TreeNode* t = NULL;
	TreeNode* p = t;
	if (token == INT || token == VOID_) {
		ExpType expType;
		if (token == INT) {
			expType = Integer;
		}
		else if (token == VOID_) {
			expType = Void;
		}
		match(token);
		if (token == ID) {
			char* idname = copyString(tokenString);
			match(ID);
			if (token == SEMI) { // declaration -> var_declaration
				t = var_declaration();
				t->attr.name = idname;
				t->type = Integer;
				TreeNode* q = newTypeNode(TypeNameK);
				q->type = Integer;
				t->child[0] = q;
			}
			else if (token == LMPAREN) { // declaration -> arrvar_declaration
				t = arrvar_declaration();
				t->attr.arr.name = idname;
				t->type = IntegerArray;
				TreeNode* q = newTypeNode(TypeNameK);
				q->type = IntegerArray;
				t->child[0] = q;
			}
			else if (token == LPAREN) { // declaration -> fun_declaration
				t = fun_declaration();
				t->attr.name = idname;
				t->type = expType;
				TreeNode* q = newTypeNode(TypeNameK);
				q->type = expType;
				t->child[0] = q;
			}
			else {
				syntaxError("unexpected token -> ");
				printToken(token, tokenString);
				fprintf(listing, "        ");
			}
		}
		else {
			syntaxError("unexpected token -> ");
			printToken(token, tokenString);
			token = getToken();
		}
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	return t;
}

/* 4. var_declaration -> type_specifier ID ; */
TreeNode* var_declaration(void) {
	TreeNode* t = newDeclNode(VarK);
	match(SEMI);
	return t;
}

/* 5. arrvar_declaration -> type_specifier ID [ NUM ] ; */
TreeNode* arrvar_declaration(void) {
	TreeNode* t = newDeclNode(ArrVarK);
	match(LMPAREN);
	if (token == NUM) {
		t->attr.arr.size = atoi(tokenString);
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	match(NUM);
	match(RMPAREN);
	match(SEMI);
	return t;
}

/* 6. fun_declaration -> type_specifier ID ( params ) compound_stmt */
TreeNode* fun_declaration(void) {
	TreeNode* t = newDeclNode(FuncK);
	match(LPAREN);
	t->child[1] = params();
	match(RPAREN);
	t->child[2] = compound_stmt();
	t->child[2]->isInFuncCom = true;
	return t;
}

/* 7. type_specifier -> int | void */

/* 8. params -> param_list | void | empty */
TreeNode* params(void) {
	TreeNode* t = NULL;
	if (token == VOID_) {
		match(VOID_);
	}
	else if (token == INT) {
		t = param_list();
	}
	return t;
}

/* 9. param_list -> param_list , param | param */
TreeNode* param_list(void) {
	TreeNode* t = param();
	TreeNode* p = t;
	while (token == COMMA)
	{
		match(COMMA);
		TreeNode* q;
		q = param();
		if (q != NULL) {
			if (t == NULL)
				t = p = q;
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

/* 10. param -> int ID | int ID [ ] */
TreeNode* param(void) {
	TreeNode* t = NULL;
	match(INT);
	if (token == ID) {
		char* idname = copyString(tokenString);
		match(ID); //param的最后一个字符是ID
		if (token == LMPAREN) {
			match(LMPAREN);
			match(RMPAREN); //param的最后一个字符是']'
			t = newParamNode(ArrParamK);
			t->attr.arr.name = idname;
			t->type = IntegerArray;
			TreeNode* p = newTypeNode(TypeNameK);
			p->type = IntegerArray;
			t->child[0] = p;
		}
		else {
			t = newParamNode(NonArrParamK);
			t->attr.name = idname;
			t->type = Integer;
			TreeNode* p = newTypeNode(TypeNameK);
			p->type = Integer;
			t->child[0] = p;
		}
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}

	return t;
}

/* 11. compound_stmt -> { local_declarations statement_list } */
TreeNode* compound_stmt(void) {
	TreeNode* t = newStmtNode(CompK);
	match(LLPAREN);
	t->child[0] = local_declarations(); //复合语句节点的第一个孩子为局部声明节点
	t->child[1] = statement_list(); //复合语句节点的第二个孩子为语句列表
	match(RLPAREN);
	return t;
}

/* 12. local_declarations -> local_declarations var_declaration | empty */
TreeNode* local_declarations(void) {
	TreeNode* t = NULL; //可能为空
	TreeNode* p = t;
	TreeNode* r = NULL;
	while (token == INT)
	{
		match(INT);
		if (token == ID) {
			char* idname = copyString(tokenString);
			match(ID);
			TreeNode* q = NULL;
			if (token == SEMI) {
				q = var_declaration();
				q->attr.name = idname;
				q->type = Integer;
				r = newTypeNode(TypeNameK);
				r->type = Integer;
				q->child[0] = r;
			}
			else if (token == LMPAREN) {
				q = arrvar_declaration();
				q->attr.arr.name = idname;
				q->type = IntegerArray;
				r = newTypeNode(TypeNameK);
				r->type = IntegerArray;
				q->child[0] = r;
			}
			else {
				syntaxError("unexpected token -> ");
				printToken(token, tokenString);
				token = getToken();
			}
			if (q != NULL) {
				if (t == NULL)
					t = p = q;
				else {
					p->sibling = q;
					p = q;
				}
			}
		}
		else {
			syntaxError("unexpected token -> ");
			printToken(token, tokenString);
			token = getToken();
		}

	}
	return t;
}

/* 13. statement_list -> statement_list statement | empty */
TreeNode* statement_list(void) {
	TreeNode* t = NULL; //可能为空
	TreeNode* p = t;
	while (token == ID || token == LLPAREN || token == IF ||
		token == WHILE || token == RETURN || token == SEMI ||
		token == LPAREN || token == NUM)
	{
		TreeNode* q;
		q = statement();
		if (q != NULL) {
			if (t == NULL)
				t = p = q;
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

/* 14. statement -> assignment_stmt
				   | compound_stmt
				   | selection_stmt
				   | iteration_stmt
				   | return_stmt
*/
TreeNode* statement(void) {
	TreeNode* t = NULL;
	if (token == ID) {
		t = assignment_stmt();
	}
	else if (token == LLPAREN) {
		t = compound_stmt();
	}
	else if (token == IF) {
		t = selection_stmt();
	}
	else if (token == WHILE) {
		t = iteration_stmt();
	}
	else if (token == RETURN) {
		t = return_stmt();
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	return t;
}

/* 15. assignment_stmt -> ID = additive_expression ;
						| ID [ additive_expression ] = additive_expression ;
						| ID ( args ) ;
*/
TreeNode* assignment_stmt(void) {
	TreeNode* t = NULL;
	char* idname = copyString(tokenString);
	match(ID);
	/* assignment_stmt -> ID = additive_expression ;
						| ID [ additive_expression ] = additive_expression ; */
	if (token == EQUAL || token == LMPAREN) {
		t = newStmtNode(AssignK);
		if (t != NULL) {
			//assignment_stmt -> ID[additive_expression] = additive_expression;
			if (token == LMPAREN) {
				TreeNode* q = newExpNode(ArrIdK);
				if (q != NULL) {
					q->attr.arr.name = idname;
					q->type = Integer;
					t->child[0] = q;
					TreeNode* r = newTypeNode(TypeNameK);
					r->type = Integer;
					q->child[0] = r;
					match(LMPAREN);
					q->child[1] = additive_expression();
					match(RMPAREN);
				}

			}
			// assignment_stmt -> ID = additive_expression ;
			else {
				TreeNode* p = newExpNode(IdK);
				if (p != NULL) {
					p->attr.name = idname;
					p->type = Integer;
					t->child[0] = p;
					TreeNode* r = newTypeNode(TypeNameK);
					r->type = Integer;
					p->child[0] = r;
				}

			}
			match(EQUAL);
			t->child[1] = additive_expression();
			match(SEMI);
		}
	}
	/* assignment_stmt -> ID ( args ) ; */
	else if (token == LPAREN) {
		t = newExpNode(CallK);
		t->attr.name = idname;
		if (t != NULL) {
			match(LPAREN);
			TreeNode* r = newTypeNode(TypeNameK);
			r->type = Void;
			t->child[0] = r;
			t->child[1] = args();
			t->type = Void;
			match(RPAREN);
			match(SEMI);
		}
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	return t;
}

/* 16. selection_stmt -> if ( expression ) statement
						| if ( expression ) statement else statement
*/
TreeNode* selection_stmt(void) {
	TreeNode* t = newStmtNode(IfK);
	match(IF);
	match(LPAREN);
	if (t != NULL) t->child[0] = expression();
	match(RPAREN);
	if (t != NULL) t->child[1] = statement();
	if (token == ELSE) {
		match(ELSE);
		if (t != NULL) t->child[2] = statement();
	}
	return t;
}

/* 17. iteration_stmt -> while ( expression ) statement */
TreeNode* iteration_stmt(void) {
	TreeNode* t = newStmtNode(IterK);
	match(WHILE);
	match(LPAREN);
	if (t != NULL) t->child[0] = expression();
	match(RPAREN);
	if (t != NULL) t->child[1] = statement();
	return t;
}

/* 18. return_stmt -> return ; | return additive_expression ; */

TreeNode* return_stmt(void) {
	TreeNode* t = newStmtNode(RetK);
	if (t != NULL) {
		match(RETURN);
		if (token != SEMI) {
			t->child[0] = additive_expression();
			match(SEMI);
		}
		else {
			match(SEMI);
		}
	}
	return t;
}

/* 19. expression -> additive_expression relop additive_expression */
TreeNode* expression(void) {
	TreeNode* t = additive_expression();
	if (token == LESSOREQUAL || token == LESSTHAN ||
		token == GREATERTHAN || token == GREATEROREQUAL ||
		token == EEQUAL || token == UNEQUAL) {
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t != NULL) {
			t->child[1] = additive_expression();
		}
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	return t;
}

/* 20. relop -> <= | < | > | >= | == | != */


/* 21. additive_expression -> additive_expression addop term | term */
TreeNode* additive_expression(void) {
	TreeNode* t = term();
	while (token == PLUS || token == MINUS)
	{
		TreeNode* q = newExpNode(OpK);
		if (q != NULL) {
			q->attr.op = token;
			match(token);
			q->child[0] = t;
		}
		if (q != NULL) q->child[1] = term();
		t = q;
	}
	return t;
}

/* 22. addop -> + | - */


/* 23. term -> term mulop factor | factor */
TreeNode* term(void) {
	TreeNode* t = factor();
	while (token == MULTIPLY || token == DIVIDE)
	{
		TreeNode* q = newExpNode(OpK);
		if (q != NULL) {
			q->attr.op = token;
			match(token);
			q->child[0] = t;
		}
		if (q != NULL) q->child[1] = factor();
		t = q;
	}
	return t;
}

/* 24. mulop -> * | / */


/* 25. factor -> ( additive_expression ) | var | call | NUM */
TreeNode* factor(void) {
	TreeNode* t = NULL;
	if (token == LPAREN) { // factor -> ( additive_expression )
		match(LPAREN);
		t = additive_expression();
		match(RPAREN);
	}
	else if (token == ID) { // factor -> var | call
		char* idname = copyString(tokenString);
		match(ID);
		if (token == LMPAREN) { // factor -> ID [ additive_expression ] | ID [ ]
			match(LMPAREN);
			t = newExpNode(ArrIdK);
			t->attr.arr.name = idname;
			if (token == RMPAREN) { // factor -> ID [ ]
				t->type = IntegerArray;
				TreeNode* w = newTypeNode(TypeNameK);
				w->type = IntegerArray;
				t->child[0] = w;
			}
			else { // factor -> ID [ additive_expression ]
				t->type = Integer;
				TreeNode* w = newTypeNode(TypeNameK);
				w->type = Integer;
				t->child[0] = w;
				t->child[1] = additive_expression();
			}
			match(RMPAREN);
		}
		else if (token == LPAREN) { // factor -> call
			match(LPAREN);
			t = newExpNode(CallK);
			t->attr.name = idname;
			t->type = Integer;
			TreeNode* w = newTypeNode(TypeNameK);
			w->type = Integer;
			t->child[0] = w;
			t->child[1] = args();
			match(RPAREN);
		}
		else { // factor -> ID
			t = newExpNode(IdK);
			t->attr.name = idname;
			t->type = Integer;
			TreeNode* w = newTypeNode(TypeNameK);
			w->type = Integer;
			t->child[0] = w;
		}

	}
	else if (token == NUM) { // factor -> NUM
		t = newExpNode(ConstK);
		t->attr.val = atoi(copyString(tokenString));
		t->type = Integer;
		TreeNode* w = newTypeNode(TypeNameK);
		w->type = Integer;
		t->child[0] = w;
		match(NUM);
	}
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
	}
	return t;
}

/* 26. var -> ID | ID [ additive_expression ] | ID [ ] */


/* 27. call -> ID ( args ) */


/* 28. args -> arg_list | empty */
TreeNode* args(void) {
	TreeNode* t = NULL;
	if (token == RPAREN)
		return t;
	else {
		t = arg_list();
		return t;
	}
}

/* 29. arg_list -> arg_list , additive_expression | additive_expression */
TreeNode* arg_list(void) {
	TreeNode* t = additive_expression();
	TreeNode* p = t;
	while (token == COMMA)
	{
		TreeNode* q;
		match(COMMA);
		q = additive_expression();
		if (q != NULL) {
			if (t == NULL)
				t = p = q;
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}


/******************************************************/
/*       the primary function of the parser           */
/******************************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void)
{
	TreeNode* t;
	token = getToken();
	t = declaration_list(); /* 1. program -> declaration_list */
	if (token != ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}

//-----------------------------------------parse.cpp------------------------------------------

//-----------------------------------------scan.cpp------------------------------------------

//DFA scanner states
typedef enum {
	START, INNUM, INID, INCOMMENT1, INCOMMENT2, DONE
}StateType;
char tokenString[MAXTOKENLEN + 1];
#define BUFLEN 256
static char lineBuf[BUFLEN];//hold the current line
static int linepos = 0;//current psition in LineBuf
static int bufsize = 0;//current size of buffer string
static char getNextChar(void) {
	if (!(linepos < bufsize)) {
		lineno++;
		if (fgets(lineBuf, BUFLEN - 1, source)) {
			bufsize = strlen(lineBuf);
			linepos = 0;
			if (EchoSource) {
				if (lineBuf[bufsize - 1] != '\n') {
					bufsize++;
					lineBuf[bufsize - 1] = '\n';
					lineBuf[bufsize] = 0;
				}
				fprintf(listing, "%4d:%s", lineno, lineBuf);
			}
			return lineBuf[linepos++];
		}
		else return EOF;
	}
	else return lineBuf[linepos++];
}
static void ungetNextChar(void) {
	linepos--;
}
static struct {
	char str[7];
	TokenType tok;
}reservedWords[MAXRESERVED] = { {"else",ELSE},{"if",IF},{"int",INT},{"return",RETURN},{"void",VOID_},{"while",WHILE} };
static TokenType reservedLookUp(char* s) {
	int i;
	for (i = 0; i < MAXRESERVED; i++)
		if (!strcmp(s, reservedWords[i].str))
			return reservedWords[i].tok;
	return ID;
}
static char c = ' ', c0 = ' ';
TokenType getToken(void) {
	int tokenStringIndex = 0;
	TokenType currentToken;
	StateType state = START;
	int save;
	while (state != DONE) {
		if (c == c0)
			c0 = getNextChar();
		c = c0;
		save = TRUE;
		switch (state) {
		case START:
			if (isdigit(c))
				state = INNUM;
			else if (isalpha(c) || c == '_')
				state = INID;
			else if ((c == ' ') || (c == '\t') || (c == '\n'))
				save = FALSE;
			else {
				state = DONE;
				switch (c) {
				case EOF:
					save = FALSE;
					currentToken = ENDFILE;
					break;
				case'+':
					currentToken = PLUS;
					break;
				case'-':
					currentToken = MINUS;
					break;
				case'*':
					c0 = getNextChar();
					if (c0 == '/') {
						currentToken = RNOTE;
						c = c0;
					}
					else currentToken = MULTIPLY;
					break;
				case'/':
					c0 = getNextChar();
					if (c0 == '*') {
						currentToken = LNOTE;
						save = FALSE;
						state = INCOMMENT1;
						c = c0;
					}
					else if (c0 == '/') {
						currentToken = LNOTE;
						save = FALSE;
						state = INCOMMENT2;
						c = c0;
					}
					else currentToken = DIVIDE;
					break;
				case'<':
					c0 = getNextChar();
					if (c0 == '=') {
						currentToken = LESSOREQUAL;
						c = c0;
					}
					else currentToken = LESSTHAN;
					break;
				case'>':
					c0 = getNextChar();
					if (c0 == '=') {
						currentToken = GREATEROREQUAL;
						c = c0;
					}
					else currentToken = GREATERTHAN;
					break;
				case'=':
					c0 = getNextChar();
					if (c0 == '=') {
						currentToken = EEQUAL;
						c = c0;
					}
					else currentToken = EQUAL;
					break;
				case'!':
					c0 = getNextChar();
					if (c0 == '=') {
						currentToken = UNEQUAL;
					}
					else currentToken = ERROR_;
				case';':
					currentToken = SEMI;
					break;
				case',':
					currentToken = COMMA;
					break;
				case'(':
					currentToken = LPAREN;
					break;
				case')':
					currentToken = RPAREN;
					break;
				case'[':
					currentToken = LMPAREN;
					break;
				case']':
					currentToken = RMPAREN;
					break;
				case'{':
					currentToken = LLPAREN;
					break;
				case'}':
					currentToken = RLPAREN;
					break;


				default:
					currentToken = ERROR_;
					break;

				}
			}
			break;
		case INCOMMENT1:
			save = FALSE;
			c0 = getNextChar();
			if (c == '*' && c0 == '/') {
				state = START;
				c = c0;
			}
			break;
		case INCOMMENT2:
			save = FALSE;
			if (c == '\n') {
				state = START;
				c = c0;
			}
			break;
		case INNUM:
			if (!isdigit(c)) {
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = NUM;
			}
			break;
		case INID:
			if (!isalpha(c) && c != '_' && !isdigit(c)) {
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = ID;
			}
			break;
		case DONE:
		default:
			fprintf(listing, "Scanner Bug: state=%d\n", state);
			state = DONE;
			currentToken = ERROR_;
			break;

		}
		if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			tokenString[tokenStringIndex++] = c;
		if (state == DONE) {
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID)
				currentToken = reservedLookUp(tokenString);
		}
	}
	if (TraceScan) {
		fprintf(listing, "\t%d:", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
}

//-----------------------------------------scan.cpp------------------------------------------

//-----------------------------------------symtab.cpp------------------------------------------


/* SHFIT is the power of two used as multiplier
	in hash function */
#define SHFIT 4

#define MAX_SCOPE 1000

	/* the hash function */
static int hash(const char* key)
{
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{
		temp = ((temp << SHFIT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

static Scope scopes[MAX_SCOPE];
static int nScope = 0;
static Scope scopeStack[MAX_SCOPE];
static int nScopeStack = 0;
static int location_[MAX_SCOPE];
Scope globalScope;

Scope sc_top(void)
{
	return scopeStack[nScopeStack - 1];
}

void sc_pop(void)
{
	--nScopeStack;
}

int addLocation(void)
{
	return location_[nScopeStack - 1]++;
}

int addLocation(int size)
{
	location_[nScopeStack - 1] += size;
	return location_[nScopeStack - 1] - size;
}

void sc_push(Scope scope)
{
	scopeStack[nScopeStack] = scope;
	location_[nScopeStack++] = 0;
}

Scope sc_create(char* funcName)
{
	Scope newScope;

	newScope = (Scope)malloc(sizeof(struct ScopeRec));
	newScope->funcName = funcName;
	newScope->nestedLevel = nScopeStack;
	newScope->parent = sc_top();

	for (int i = 0; i < SIZE; i++)
	{
		newScope->hashTable[i] = NULL;
	}

	scopes[nScope++] = newScope;
	return newScope;
}


// find var in stack from this stack to old stack
BucketList st_bucket(char* name)
{
	int h = ::hash(name);
	Scope sc = sc_top();
	while (sc) {
		BucketList l = sc->hashTable[h];
		while ((l != NULL) && (strcmp(name, l->name) != 0))
			l = l->next;
		if (l != NULL)return l;
		sc = sc->parent;
	}
	return NULL;
}

/* Procedure st_insert inserts line numbers and
 * memroy location into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char* name, int lineno, int loc, TreeNode* treeNode)
{
	int h = ::hash(name);
	Scope top = sc_top();
	BucketList l = top->hashTable[h];
	while ((l != NULL) && (strcmp(name, l->name) != 0))
		l = l->next;
	if (l == NULL)
	{
		l = (BucketList)malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList)malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;
		l->lines->next = NULL;
		l->next = top->hashTable[h];
		l->treeNode = treeNode;
		top->hashTable[h] = l;
	}
	else /* found in table, so just add line number */
	{
		//Error
	}
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 * search all scopes
 */
int st_lookup(char* name)
{
	BucketList l = st_bucket(name);
	if (l != NULL)return l->memloc;
	return -1;
}

// search top scope
int st_lookup_top(char* name)
{
	int h = ::hash(name);
	Scope sc = sc_top();
	while (sc) {
		BucketList l = sc->hashTable[h];
		while ((l != NULL) && (strcmp(name, l->name) != 0))
		{
			l = l->next;
		}
		if (l != NULL)return l->memloc;
		break;
	}
	return -1;
}

int st_add_lineno(char* name, int lineno)
{
	BucketList l = st_bucket(name);
	LineList ll = l->lines;
	while (ll->next != NULL)ll = ll->next;
	ll->next = (LineList)malloc(sizeof(struct LineListRec));
	ll->next->lineno = lineno;
	ll->next->next = NULL;
	return lineno;
}

void printSymTabRows(BucketList* hashTable, FILE* listing)
{
	int j;
	for (j = 0; j < SIZE; j++)
	{
		if (hashTable[j] != NULL)
		{
			BucketList l = hashTable[j];
			TreeNode* node = l->treeNode;

			while (l != NULL)
			{
				LineList t = l->lines;

				fprintf(listing, "%-14s ", l->name);

				switch (node->nodeKind) {
				case DeclK:
					switch (node->kind.decl) {
					case FuncK:
						fprintf(listing, "Function \t");
						break;
					case VarK:
						fprintf(listing, "Variable \t");
						break;
					case ArrVarK:
						fprintf(listing, "Array Var.\t");
						break;
					default:
						break;
					}
					break;
				case ParamK:
					switch (node->kind.param) {
					case NonArrParamK:
						fprintf(listing, "Variable \t");
						break;
					case ArrParamK:
						fprintf(listing, "Array Var.\t");
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}

				switch (node->type) {
				case Void:
					fprintf(listing, "Void    \t");
					break;
				case Integer:
					fprintf(listing, "Integer \t");
					break;
				case Boolean:
					fprintf(listing, "Boolean \t");
					break;
				case IntegerArray:
					fprintf(listing, "Integer \t");
				default:
					break;
				}

				while (t != NULL)
				{
					fprintf(listing, "%4d ", t->lineno);
					t = t->next;
				}

				//fprintf(listing, "memloc %d  ", l->memloc);

				fprintf(listing, "\n");
				l = l->next;
			}
		}
	}
}


/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE* listing)
{
	int i;
	for (i = 0; i < nScope; i++)
	{
		Scope scope = scopes[i];
		BucketList* hashTable = scope->hashTable;

		if (i == 0)
		{
			fprintf(listing, "<global scope>");
		}
		else {
			fprintf(listing, "function name: %s ", scope->funcName);
		}

		fprintf(
			listing,
			"(nested level: %d)\n",
			scope->nestedLevel);

		fprintf(listing, "Symbol Name    Sym.Type         Data  Type        Line Numbers\n");
		fprintf(listing, "-----------    ---------        ----------        -------------------------------\n");

		printSymTabRows(hashTable, listing);

		fputc('\n', listing);

	}
}/* printSymTab */


//-----------------------------------------symtab.cpp------------------------------------------


//-----------------------------------------util.cpp------------------------------------------


 /* Procedure printToken prints a token
  * and its lexeme to the listing file
  */

  /* Procedure printToken prints a token
   * and its lexeme to the listing file
   */
void printToken(TokenType token, const char* tokenString)
{
	switch (token)//ELSE, IF, INT, RETURN, VOID_, WHILE,
	{
	case ELSE:
	case IF:
	case INT:
	case RETURN:
	case VOID_:
	case WHILE:
		fprintf(listing, "reserved word : %s\n", tokenString);
		break;
	case PLUS:fprintf(listing, "+\n"); break;
	case MINUS:fprintf(listing, "-\n"); break;
	case MULTIPLY:fprintf(listing, "*\n"); break;
	case DIVIDE:fprintf(listing, "/\n"); break;
	case LESSTHAN:fprintf(listing, "<\n"); break;
	case LESSOREQUAL:fprintf(listing, "<=\n"); break;
	case GREATERTHAN:fprintf(listing, ">\n"); break;
	case GREATEROREQUAL:fprintf(listing, ">=\n"); break;
	case EEQUAL:fprintf(listing, "==\n"); break;
	case UNEQUAL:fprintf(listing, "!=\n"); break;
	case EQUAL:fprintf(listing, "=\n"); break;
	case SEMI:fprintf(listing, ";\n"); break;
	case COMMA:fprintf(listing, ",\n"); break;
	case LPAREN:fprintf(listing, "(\n"); break;
	case RPAREN:fprintf(listing, ")\n"); break;
	case LMPAREN:fprintf(listing, "[\n"); break;
	case RMPAREN:fprintf(listing, "]\n"); break;
	case LLPAREN:fprintf(listing, "{\n"); break;
	case RLPAREN:fprintf(listing, "}\n"); break;
	case ENDFILE:fprintf(listing, "EOF\n"); break;
	case NUM:
		fprintf(listing, "NUM, val= %s\n", tokenString);
		break;
	case ID:
		fprintf(listing, "ID, name = %s\n", tokenString);
		break;
	case ERROR_:
		fprintf(listing, "ERROR: %s\n", tokenString);
		break;
	default:
		fprintf(listing, "Unknown token: %d\n", token);
	}
}


/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodeKind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
		t->isInFuncCom = false;
	}
	return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++)t->child[i] = NULL;
		t->sibling = NULL;
		t->nodeKind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Void;
		t->isInFuncCom = false;
	}
	return t;
}

/* Function newParamNode creates a new declaration
 * node for syntax tree construction
 */
TreeNode* newDeclNode(DeclKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++)t->child[i] = NULL;
		t->sibling = NULL;
		t->nodeKind = DeclK;
		t->kind.decl = kind;
		t->lineno = lineno;
		t->isInFuncCom = false;
	}
	return t;
}

/* Function newParamNode creates a new parameter
 * node for syntax tree construction
 */
TreeNode* newParamNode(ParamKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++)t->child[i] = NULL;
		t->sibling = NULL;
		t->nodeKind = ParamK;
		t->kind.param = kind;
		t->lineno = lineno;
		t->isInFuncCom = false;
	}
	return t;
}

/* Function newTypeNode creates a new type
 * node for syntax tree constrcution
 */
TreeNode* newTypeNode(TypeKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++)t->child[i] = NULL;
		t->sibling = NULL;
		t->nodeKind = TypeK;
		t->kind.type = kind;
		t->lineno = lineno;
		t->isInFuncCom = false;
	}
	return t;
}

/* Function copuString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s)
{
	int n;
	char* t;
	if (s == NULL)return NULL;
	n = strlen(s) + 1;
	t = (char*)malloc(n);
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else strcpy(t, s);
	return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* maros to increase/decrease indentation */
#define INDENT indentno+=2;
#define UNINDENT indentno -= 2;

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
	int i;
	for (i = 0; i < indentno; i++)
		fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree)
{
	int i;
	INDENT;
	while (tree != NULL) {
		printSpaces();
		if (tree->nodeKind == StmtK)
		{
			switch (tree->kind.stmt) {
			case IfK:
				fprintf(listing, "If\n");
				break;
			case IterK:
				fprintf(listing, "While\n");
				break;
			case AssignK:
				fprintf(listing, "Assign\n");
				break;
			case CompK:
				fprintf(listing, "Compound Statement\n");
				break;
			case RetK:
				fprintf(listing, "Return\n");
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == ExpK)
		{
			switch (tree->kind.exp) {
			case OpK:
				fprintf(listing, "Op: ");
				printToken(tree->attr.op, "\0");
				break;
			case ConstK:
				fprintf(listing, "Const: %d\n", tree->attr.val);
				break;
			case IdK:
				fprintf(listing, "Id: %s\n", tree->attr.name);
				break;
			case ArrIdK:
				fprintf(listing, "ArrId: %s\n", tree->attr.arr.name);
				break;
			case CallK:
				fprintf(listing, "Call(followings are args): %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == DeclK) {
			switch (tree->kind.decl) {
			case VarK:
				fprintf(listing, "Var Decl: %s\n", tree->attr.name);
				break;
			case ArrVarK:
				fprintf(listing, "Var Decl(following const:array length): %s %d\n",
					tree->attr.arr.name, tree->attr.arr.size);
				break;
			case FuncK:
				fprintf(listing, "Funcion Decl: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown DeclNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == ParamK) {
			switch (tree->kind.param) {
			case ArrParamK:
				fprintf(listing, "Array Parameter: %s\n", tree->attr.arr.name);
				break;
			case NonArrParamK:
				fprintf(listing, "Parameter: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown ParamNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == TypeK) {
			switch (tree->type) {
			case Void:
				fprintf(listing, "Type: void\n");
				break;
			case Integer:
				fprintf(listing, "Type: int\n");
				break;
			case IntegerArray:
				fprintf(listing, "Type: int array\n");
				break;
			default:
				fprintf(listing, "Unknown TypeNode kind\n");
				break;
			}
		}
		else fprintf(listing, "Unknown node kind\n");
		for (i = 0; i < MAXCHILDREN; i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}

//-----------------------------------------util.cpp------------------------------------------



char* complier(char * data,int datasize,char *filename)
{
	TreeNode* syntaxTree;

	char* buf =(char*) malloc(sizeof(char) * datasize+1);
	memcpy(buf, data, datasize+1);
	buf[datasize] = 0;
	source = NULL;
	source = fopen("tmp.txt", "w+");
	fputs(buf, source);
	fclose(source);

	source = fopen("tmp.txt", "r");


	listing = stdout;


	fprintf(listing, "\nTINY COMPILATION: %s\n", filename);
#if NO_PARSE
	while (getToken() != ENDFILE);
#else
	syntaxTree = parse();
	if (TraceParse) {
		fprintf(listing, "\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	if (!Error)
	{
		if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
		buildSymtab(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
	}
	

#if !NO_CODE
	char* codefile=NULL;
	if (!Error)
	{
		
		int fnlen = strcspn(filename, ".");

		codefile = (char*)calloc(fnlen + 4, sizeof(char));
		strncpy(codefile, filename, fnlen);
		strcat(codefile, ".tm");
		code = fopen(codefile, "w");
		//code = stdout;
		if (code == NULL)
		{
			fprintf(listing, "Unable to open %s\n", codefile);
			exit(1);
		}
		codeGen(syntaxTree, codefile);
		fprintf(listing, "Code Gen Finished\n");
		fclose(code);
	}
#endif
#endif
#endif
	fclose(source);


	//读取全部数据到新的File
	code = fopen(codefile, "r");
	if (code == NULL)
	{
		return NULL;
	}
	fseek(code, 0, SEEK_END);
	int length = ftell(code);
	char* codeData = (char*)malloc((length + 1) * sizeof(char));
	rewind(code);
	length = fread(codeData, 1, length, code);
	codeData[length] = '\0';
	fclose(code);
	return codeData;
	

}


