#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

#define STACK_SIZE (128)

typedef enum {
	SYMBOL_LOCAL,
	SYMBOL_PARAM,
	SYMBOL_GLOBAL
} symbol_t;

struct symbol {
	symbol_t kind;
	struct type *type;
	char *name;
	int which; 
};

struct symbol * symbol_create( symbol_t kind, struct type *type, char *name );

struct symbol * symbol_copy( struct symbol *in );

const char * symbol_codegen( struct symbol *s ); 

#endif
