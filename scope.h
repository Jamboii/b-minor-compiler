#ifndef SCOPE_H
#define SCOPE_H

#include "symbol.h"
#include "hash_table.h"

#include <stdlib.h>
#include <string.h>

struct scope_stack {
	struct hash_table *hash;
	struct scope_stack *next;
	symbol_t symbl;
	int loc;
};

void scope_enter();
void scope_exit();
int  scope_level();

void           scope_bind( const char *name, struct symbol *sym );
struct symbol *scope_lookup( const char *name );
struct symbol *scope_lookup_current( const char *name );

#endif