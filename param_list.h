#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include "type.h"
#include "expr.h"
#include "symbol.h"
#include <stdio.h>

struct expr;

struct param_list {
	char *name;
	struct type *type;
	struct symbol *symbol;
	struct param_list *next;
};

struct param_list * param_list_create( char *name, struct type *type, struct param_list *next );

struct param_list * param_list_copy( struct param_list *a);
int param_list_compare( struct param_list *a, struct param_list *b );
int param_list_compare_call( struct param_list *a, struct expr *b );
void param_list_delete( struct param_list *a );

void param_list_resolve( struct param_list *a );

void param_list_typecheck( struct param_list *a );

void param_list_print( struct param_list *a );

#endif
