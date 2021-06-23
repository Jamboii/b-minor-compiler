#ifndef EXPR_H
#define EXPR_H

#include "symbol.h"

#include <stdio.h>

typedef enum {
	/* expr exprs exprs_more expr_for ops comp logic negative postfix group brackets bracket array_elements elements elements_more atomic */
	EXPR_ASSIGN,			// 0
	EXPR_ADD,				// 1
	EXPR_SUB,				// 2
	EXPR_MUL,				// 3
	EXPR_DIV,				// 4
	EXPR_MOD,				// 5
	EXPR_EXP,				// 6
	EXPR_LE,				// 7
	EXPR_LT,				// 8
	EXPR_GE,				// 9
	EXPR_GT,				// 10
	EXPR_EQ,				// 11
	EXPR_NEQ,				// 12
	EXPR_AND,				// 13
	EXPR_OR,				// 14
	EXPR_NOT,				// 15
	EXPR_NEG,				// 16
	EXPR_INCR,				// 17
	EXPR_DECR,				// 18
	EXPR_GROUP,				// 19
	EXPR_ARRELEM,			// 20
	EXPR_INT_LITERAL,		// 21
	EXPR_BOOLEAN_LITERAL, 	// 22
	EXPR_CHAR_LITERAL,		// 23
	EXPR_STRING_LITERAL,	// 24
	EXPR_NAME,				// 25
	EXPR_FUNCCALL			// 26
} expr_t;

struct expr {
	/* used by all kinds of exprs */
	expr_t kind;
	struct expr *left;
	struct expr *right;

	/* used by various leaf exprs */
	const char *name;
	int literal_value;
	const char * string_literal;
	struct symbol *symbol;

	struct expr* next;

	int reg;
};

struct expr * expr_create( expr_t kind, struct expr *left, struct expr *right );

struct expr * expr_create_name( const char *n );
struct expr * expr_create_integer_literal( int c );
struct expr * expr_create_boolean_literal( int c );
struct expr * expr_create_char_literal( char c );
struct expr * expr_create_string_literal( const char *str );

void expr_resolve( struct expr *e );
void exprs_resolve( struct expr *e );

struct type * expr_typecheck( struct expr *e );

void expr_codegen( struct expr *e, FILE *outfil );

void expr_print( struct expr *e );
void exprs_print( struct expr *e );



#endif
