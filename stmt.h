#ifndef STMT_H
#define STMT_H

#include "decl.h"
#include "expr.h"

typedef enum {
	STMT_DECL,    // 0
	STMT_EXPR,	  // 1
	STMT_IF_ELSE, // 2
	STMT_FOR,     // 3
	STMT_PRINT,   // 4
	STMT_RETURN,  // 5
	STMT_BLOCK    // 6
} stmt_t;

struct stmt {
	stmt_t kind;
	struct decl *decl;
	struct expr *init_expr;
	struct expr *expr;
	struct expr *next_expr;
	struct stmt *body;
	struct stmt *else_body;
	struct stmt *next;
};

struct stmt * stmt_create( stmt_t kind, struct decl *decl, struct expr *init_expr, struct expr *expr, struct expr *next_expr, struct stmt *body, struct stmt *else_body, struct stmt *next );

void stmt_resolve( struct stmt *s );

void stmt_typecheck( struct stmt *s );

void stmt_codegen( struct stmt *s, FILE *outfil );

void stmt_print( struct stmt *s, int indent );
void stmt_print_tabs( int indent );


#endif
