%token TOKEN_EOF
%token TOKEN_IDENT
%token TOKEN_ERROR
// Literals
%token TOKEN_NUMBER
%token TOKEN_STRING_LITERAL
%token TOKEN_CHAR_LITERAL
// Types
%token TOKEN_INTEGER 
%token TOKEN_ARRAY
%token TOKEN_BOOLEAN 
%token TOKEN_CHAR
%token TOKEN_STRING
// Key Words
%token TOKEN_ELSE 
%token TOKEN_FALSE 
%token TOKEN_FOR 
%token TOKEN_FUNCTION 
%token TOKEN_IF 
%token TOKEN_PRINT 
%token TOKEN_RETURN 
%token TOKEN_TRUE 
%token TOKEN_VOID
%token TOKEN_AUTO
// Bracket Types
%token TOKEN_LEFTPARAND
%token TOKEN_RIGHTPARAND
%token TOKEN_LEFTSQUARE
%token TOKEN_RIGHTSQUARE
%token TOKEN_LEFTCURLY
%token TOKEN_RIGHTCURLY
// Operations
%token TOKEN_INCREMENT
%token TOKEN_DECREMENT 
%token TOKEN_ADD
%token TOKEN_SUBTRACT 
%token TOKEN_EXPONENT 
%token TOKEN_MULTIPLY
%token TOKEN_DIVIDE
%token TOKEN_MODULUS
%token TOKEN_ASSIGNMENT
// Compare and Contrast
%token TOKEN_GT
%token TOKEN_GE
%token TOKEN_LT
%token TOKEN_LE
%token TOKEN_EQ
%token TOKEN_NEQ
// Logic
%token TOKEN_AND
%token TOKEN_OR
%token TOKEN_NOT
// Other
%token TOKEN_SEMICOLON
%token TOKEN_COLON
%token TOKEN_COMMA

%union
{
	struct decl *decl;
	struct stmt *stmt;
	struct expr *expr;
	struct param_list *param_list;
	struct type *type;
	char* ident;
	int number;	
};

%type <decl> prog decls decl decl_assignment decl_function assign no_assign no_assign_arr
%type <stmt> stmt stmts match unmatch stmt_other stmt_eds
%type <expr> expr exprs exprs_more expr_for add_sub mult_div_mod exp comp logic negative postfix group brackets bracket array_elements elements elements_more atomic
%type <type> type type_func array_assign array_arg 
%type <param_list> no_assigns no_assigns_more
%type <ident> ident
%type <number> number

%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	#include "decl.h"
	#include "stmt.h"
	#include "expr.h"
	#include "type.h"
	#include "param_list.h"

	extern int yylex();
	extern char *yytext;
	extern int yylineno;

	int yyerror();

	struct decl* parser_result;
%}

%%

////////////////////////////////
// DECL GRAMMAR
// format
// - decl_create(name, type, value, code, next)
// name: name of declaration
// type: type of declaration
// value: if an expression
// code: if a function
// next: pointer to the next declaration in the program
////////////////////////////////

// program is a set of declarations
prog: decls 		{ parser_result = $1;   }
	| %empty		{ parser_result = NULL; }
	;

// can either have one or many declarations
decls: decl 		{ $$ = $1;                }
	 | decl decls   { $$ = $1, $1->next = $2; }
	 ;

// global variable or global function declarations or function prototypes
decl: decl_assignment { $$ = $1; }
    | decl_function   { $$ = $1; }
    ;

// declaration for global variable
decl_assignment: assign TOKEN_SEMICOLON    { $$ = $1; }
	           | no_assign TOKEN_SEMICOLON { $$ = $1; }

// declaration for function
decl_function: ident TOKEN_COLON TOKEN_FUNCTION type_func TOKEN_LEFTPARAND no_assigns TOKEN_RIGHTPARAND TOKEN_SEMICOLON	{ $$ = decl_create($1, type_create(TYPE_PROTO, $4, $6, 0), 0, 0, 0); }
	         | ident TOKEN_COLON TOKEN_FUNCTION type_func TOKEN_LEFTPARAND no_assigns TOKEN_RIGHTPARAND TOKEN_ASSIGNMENT TOKEN_LEFTCURLY stmts TOKEN_RIGHTCURLY	{ $$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, $10, 0); }
	         | ident TOKEN_COLON TOKEN_FUNCTION type_func TOKEN_LEFTPARAND no_assigns TOKEN_RIGHTPARAND TOKEN_ASSIGNMENT TOKEN_LEFTCURLY TOKEN_RIGHTCURLY { $$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, 0, 0); }
	         ;

// assignment of global and local variables (other than an array)
assign: ident TOKEN_COLON type TOKEN_ASSIGNMENT expr 				   { $$ = decl_create(strdup($1), $3, $5, 0, 0); }
      | ident TOKEN_COLON array_assign								   { $$ = decl_create(strdup($1), $3,  0, 0, 0); }
	  | ident TOKEN_COLON array_assign TOKEN_ASSIGNMENT array_elements { $$ = decl_create(strdup($1), $3, $5, 0, 0); /*printf("DECL ARRAY ELEMENTS\n");*/ }
	  ;

// variable declaration without assignment
no_assign: ident TOKEN_COLON type 			{ $$ = decl_create($1, $3, 0, 0, 0); }
		 ;

// an unassigned array without a size definition for use only in array arguments
no_assign_arr: ident TOKEN_COLON array_arg  { $$ = decl_create($1, $3, 0, 0, 0); }
             ;

// multiple arguments or var declarations without assignment (like function args)
no_assigns: no_assign 						{ $$ = param_list_create($1->name, $1->type, 0);  }
          | no_assign_arr 					{ $$ = param_list_create($1->name, $1->type, 0);  }
		  | no_assign no_assigns_more 		{ $$ = param_list_create($1->name, $1->type, 0), $$->next = $2; }
		  | no_assign_arr no_assigns_more   { $$ = param_list_create($1->name, $1->type, 0), $$->next = $2; }
		  | %empty 							{ $$ = 0; }
		  ;

// are we using more than one function arg
no_assigns_more: TOKEN_COMMA no_assign no_assigns_more 	   { $$ = param_list_create($2->name, $2->type, 0), $$->next = $3; }
               | TOKEN_COMMA no_assign_arr no_assigns_more { $$ = param_list_create($2->name, $2->type, 0), $$->next = $3; }
			   | TOKEN_COMMA no_assign 					   { $$ = param_list_create($2->name, $2->type, 0);  }
			   | TOKEN_COMMA no_assign_arr  			   { $$ = param_list_create($2->name, $2->type, 0);  }
			   ;

////////////////////////////////
// STMT GRAMMAR
// format
// - stmt_create(kind, decl, init_expr, expr, next_expr, body, else_body, next)
////////////////////////////////

// 1 or more statements
stmts: stmt       { $$ = $1;                }
     | stmt stmts { $$ = $1, $1->next = $2; }
     ;

// statements (stmt) such as if-else, loops, return statements
stmt: match		{ $$ = $1; }
	| unmatch   { $$ = $1; }
	;

// matched statement
match: TOKEN_IF TOKEN_LEFTPARAND expr TOKEN_RIGHTPARAND match TOKEN_ELSE match 											 { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0); }
	 | TOKEN_FOR TOKEN_LEFTPARAND expr_for TOKEN_SEMICOLON expr_for TOKEN_SEMICOLON expr_for TOKEN_RIGHTPARAND match     { $$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0); }
	 | stmt_other 																										 { $$ = $1; }
	 ;

// unmatched statement
unmatch: TOKEN_IF TOKEN_LEFTPARAND expr TOKEN_RIGHTPARAND stmt 															 { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, 0, 0); }
	   | TOKEN_IF TOKEN_LEFTPARAND expr TOKEN_RIGHTPARAND match TOKEN_ELSE unmatch 										 { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0); }
	   | TOKEN_FOR TOKEN_LEFTPARAND expr_for TOKEN_SEMICOLON expr_for TOKEN_SEMICOLON expr_for TOKEN_RIGHTPARAND unmatch { $$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0); }
	   ;

// other statement: return, print, some other statements, expressions, or declarations
stmt_other: TOKEN_RETURN expr TOKEN_SEMICOLON 	{ $$ = stmt_create(STMT_RETURN, 0, 0, $2, 0, 0, 0, 0);                           }
		  | TOKEN_RETURN TOKEN_SEMICOLON		{ $$ = stmt_create(STMT_RETURN, 0, 0, expr_create(TYPE_VOID, 0, 0), 0, 0, 0, 0); }
		  | TOKEN_PRINT exprs TOKEN_SEMICOLON   { $$ = stmt_create(STMT_PRINT,  0, 0, $2, 0, 0, 0, 0);                           }
		  | stmt_eds                            { $$ = $1; }
          ;

// expr or decl or statements
stmt_eds: TOKEN_LEFTCURLY stmts TOKEN_RIGHTCURLY { $$ = stmt_create(STMT_BLOCK, 0, 0, 0, 0, $2, 0, 0);                  }
		| expr TOKEN_SEMICOLON                   { $$ = stmt_create(STMT_EXPR,  0, 0, $1, 0, 0, 0, 0);                  }
		| decl_assignment                        { $$ = stmt_create(STMT_DECL, $1, 0, 0, 0, 0, 0, 0); } // may need to be null vals instead
		;

////////////////////////////////
// TYPE GRAMMAR
//
// format
// - type_create(kind, subtype, params, size)
// current issues:
// - make the array_assign semantics into a decl or expression creation of some sort so we can account for the array size
////////////////////////////////

// atomic types
// definitions  (type) representing abstract types like integer, string, and array
type: TOKEN_INTEGER 							{ $$ = type_create(TYPE_INTEGER, 0, 0, 0);   }
	| TOKEN_BOOLEAN								{ $$ = type_create(TYPE_BOOLEAN, 0, 0, 0);   }
	| TOKEN_CHAR 								{ $$ = type_create(TYPE_CHARACTER, 0, 0, 0); }
	| TOKEN_STRING 								{ $$ = type_create(TYPE_STRING, 0, 0, 0);    }
	| TOKEN_AUTO                                { $$ = type_create(TYPE_AUTO, 0, 0, 0);      }
	;

// function return type, can be any of the regular types or nothing (void)
type_func: type 								{ $$ = $1;                          }
         | TOKEN_VOID							{ $$ = type_create(TYPE_VOID, 0, 0, 0);}
         ;

// array usage: "a: array[5] integer = {1,2,3,4,5};"
array_assign: TOKEN_ARRAY TOKEN_LEFTSQUARE number TOKEN_RIGHTSQUARE type         { $$ = type_create(TYPE_ARRAY, $5, 0, $3); }
            | TOKEN_ARRAY TOKEN_LEFTSQUARE number TOKEN_RIGHTSQUARE array_assign { $$ = type_create(TYPE_ARRAY, $5, 0, $3); }
	        ;

// array usage as a function argument: printarray: function void (a: array [] array [] integer, size: integer)
array_arg: TOKEN_ARRAY TOKEN_LEFTSQUARE TOKEN_RIGHTSQUARE type      { $$ = type_create(TYPE_ARRAY, $4, 0, 0); }
         | TOKEN_ARRAY TOKEN_LEFTSQUARE TOKEN_RIGHTSQUARE array_arg { $$ = type_create(TYPE_ARRAY, $4, 0, 0); }
         ;

// array element declarations
array_elements: TOKEN_LEFTCURLY elements TOKEN_RIGHTCURLY      { $$ = $2; /*printf("ARRAY ELEMENTS\n");*/ }
			  | TOKEN_LEFTCURLY elements_more TOKEN_RIGHTCURLY { $$ = $2; /*printf("ARRAY ELEMENTS MORE\n");*/ }
			  ;

// array element literals, 1D
elements: atomic 					  { $$ = $1;                 /*printf("ARRAY ELEMENT ATOMIC\n");*/ }
		| atomic TOKEN_COMMA elements { $$ = $1, $1->next = $3; /*printf("ARRAY ELEMENT ATOMIC COMMA\n");*/ }
		;

// array element literals, 2D+
elements_more: TOKEN_LEFTCURLY elements TOKEN_RIGHTCURLY                           { $$ = $2;                 /*printf("2D ARRAY ELEMENT\n");*/      }
 			 | TOKEN_LEFTCURLY elements TOKEN_RIGHTCURLY TOKEN_COMMA elements_more { $$ = $2, $2->right = $5; /*printf("2D ARRAY ELEMENT MORE\n");*/ }
 			 ;

////////////////////////////////
// EXPR GRAMMAR
//
// format
// - expr_create(kind, left, right)
// current issues:
// - duplication of different tokens followed by some atomic doesn't sound good
////////////////////////////////

// for loop expression ( could be something like for (i=0;i<5;i++) or (;;) )
expr_for: expr   { $$ = $1; }
        | %empty { $$ = 0;  }
        ;

// multiple expressions for either print statements or function calls, or nothing at all
exprs: exprs_more { $$ = $1; }
     | %empty     { $$ = 0;  }
     ;

// multiple expressions for either print statements or function calls
exprs_more: expr   						{ $$ = $1;                }
	      | expr TOKEN_COMMA exprs_more { $$ = $1, $1->next = $3; }
	      ;
 
// assignment expression
expr: expr TOKEN_ASSIGNMENT add_sub	{ $$ = expr_create(EXPR_ASSIGN, $1, $3); }
    | add_sub                   	{ $$ = $1; }
    ;

// operations: addition, subtraction
add_sub: add_sub TOKEN_ADD mult_div_mod    		{ $$ = expr_create(EXPR_ADD, $1, $3); }
       | add_sub TOKEN_SUBTRACT mult_div_mod	{ $$ = expr_create(EXPR_SUB, $1, $3); }
       | mult_div_mod                           { $$ = $1; }
       ;

// operations: multiply, divide, modulus
mult_div_mod: mult_div_mod TOKEN_MULTIPLY exp	{ $$ = expr_create(EXPR_MUL, $1, $3); }
   | mult_div_mod TOKEN_DIVIDE exp				{ $$ = expr_create(EXPR_DIV, $1, $3); }
   | mult_div_mod TOKEN_MODULUS exp				{ $$ = expr_create(EXPR_MOD, $1, $3); }
   | exp 										{ $$ = $1; }
   ;

// operations: exponent
exp: exp TOKEN_EXPONENT comp   	{ $$ = expr_create(EXPR_EXP, $1, $3); }
   | comp 						{ $$ = $1; }
   ;

// comparison operators: >, >=, <, <=, ==, !=
comp: comp TOKEN_GE logic 		{ $$ = expr_create(EXPR_GE,  $1, $3); }
    | comp TOKEN_GT logic 		{ $$ = expr_create(EXPR_GT,  $1, $3); }
    | comp TOKEN_LE logic 		{ $$ = expr_create(EXPR_LE,  $1, $3); }
    | comp TOKEN_LT logic 		{ $$ = expr_create(EXPR_LT,  $1, $3); }
    | comp TOKEN_EQ logic 		{ $$ = expr_create(EXPR_EQ,  $1, $3); }
    | comp TOKEN_NEQ logic 		{ $$ = expr_create(EXPR_NEQ, $1, $3); }
    | logic 					{ $$ = $1; }
    ; 

// logic operators: &&, || 
logic: logic TOKEN_AND negative 	{ $$ = expr_create(EXPR_AND, $1, $3); }
     | logic TOKEN_OR  negative 	{ $$ = expr_create(EXPR_OR,  $1, $3); }
     | negative 					{ $$ = $1; }

// unary negation, logical not: ! -
negative: TOKEN_NOT postfix			{ $$ = expr_create(EXPR_NOT, 0, $2); }
        | TOKEN_SUBTRACT postfix    { $$ = expr_create(EXPR_NEG, 0, $2); }
        | postfix  					{ $$ = $1; }
        ;

// postfix increment, decrement: ++ --
postfix: postfix TOKEN_INCREMENT    { $$ = expr_create(EXPR_INCR, $1, 0); }
	   | postfix TOKEN_DECREMENT    { $$ = expr_create(EXPR_DECR, $1, 0); }
	   | group  					{ $$ = $1; }
	   ;

// grouping: (), [], function call f()
group: TOKEN_LEFTPARAND expr TOKEN_RIGHTPARAND 			{ $$ = expr_create(EXPR_GROUP, 0, $2);  					 }
     | ident TOKEN_LEFTPARAND exprs TOKEN_RIGHTPARAND   { $$ = expr_create(EXPR_FUNCCALL, expr_create_name($1), $3); /*printf("FUNC CALL\n");*/ }
     | ident brackets                                   { $$ = expr_create(EXPR_ARRELEM, expr_create_name($1), $2);  /*printf("ARRAY INDEX\n");*/ }
     | atomic 											{ $$ = $1; }
     ;

// one set of brackets or multiple in a row
brackets: bracket 										{ $$ = $1; }
	    | bracket brackets  							{ $$ = $1, $1->next = $2; /*printf("MULTIDIM\n");*/ }
	    ;

// bracket set with some expression in between
bracket: TOKEN_LEFTSQUARE expr TOKEN_RIGHTSQUARE  		{ $$ = $2; /*printf("BRACKET\n");*/ }
       ;

////////////////////////////////
// LITERALS AND ATOMICS
////////////////////////////////

// what could be to the right of an equals sign or function type
atomic: number 					{ $$ = expr_create_integer_literal($1); 		   }
      | ident  					{ $$ = expr_create_name($1);                       }
      | TOKEN_STRING_LITERAL	{ $$ = expr_create_string_literal(strdup(yytext)); /*printf("STRING LITERAL %s\n",strdup(yytext));*/ }
      | TOKEN_CHAR_LITERAL		{ $$ = expr_create_char_literal(yytext[1]);        /*printf("CHARACTER LITERAL %s\n",strdup(yytext));*/ }
      | TOKEN_TRUE 				{ $$ = expr_create_boolean_literal(1);  		   }
      | TOKEN_FALSE				{ $$ = expr_create_boolean_literal(0);     		   }
      ;

// integer literal for either a definition or array size?
number: TOKEN_NUMBER	{ $$ = atoi(yytext); /*printf("NUMBER %d\n", atoi(yytext));*/ }
	  ;

// identifier
ident: TOKEN_IDENT		{ $$ = strdup(yytext);   /*printf("IDENTIFIER %s\n", strdup(yytext));*/ }
     ;

%%

int yyerror(char const *s)
{
	printf("parse error: failed on line %d, %s\n",yylineno,s);
	exit(1);
} 