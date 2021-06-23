#include "symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Func_Count = 0;

struct symbol * symbol_create(symbol_t kind, struct type *type, char *name)
{
	struct symbol *s = malloc(sizeof(*s));

	s->kind = kind;
	s->type = type;
	s->name = strdup(name);

	// increment function counter by 1 for the sake of the code generator
	if (s->kind == SYMBOL_GLOBAL && s->type->kind == TYPE_FUNCTION) Func_Count++;

	return s;
}

const char * symbol_codegen(struct symbol *s)
{
	/*
	- mapping from the symbols in a program to the assembly lang code representing those symbols. Gotta generate symbol addresses
	  - const char * symbol_codegen( struct symbol *s );

	- symbol_codegen : returns a string which is a fragment of an instruction, representing the address computation needed for a given symbol
	  - examines the scope of the symbol
	  - global : name in assembly language is the same as in the source language ex. count: integer --> returns count
	  - local/param : return an address computation that yields the pos of that local var/param on the stack (based on that "which" parameter of symbol struct)
	    - returns a string describing the precise stack address of local vars and params, knowing only its position in the stack frame
	*/

	char *sym_str = malloc(sizeof(char) * 10);

	// examine the scope of this symbol
	switch (s->kind)
	{
		// local/param : return an address computation that yields the pos of that local var/param on the stack (based on that "which" parameter of symbol struct)
		case SYMBOL_LOCAL: // starts from the top of the stack
		{
			/*
			printf("\tprocessing local variable %s\n",s->name);
			int stack_pos = STACK_SIZE - (s->which * 8);
			printf("\tstack position for %s (%i): %i\n",s->name,s->which,stack_pos);
			if (stack_pos < 0)
			{
				printf("codegen error: No more stack allocation space (LOCAL %s, byte %i)\n",s->name,stack_pos);
				exit(1);
			}
			sprintf(sym_str, "%i", stack_pos);
			return sym_str;
			*/
		}
		case SYMBOL_PARAM: // starts from the bottom of the stack
		{
			printf("\tprocessing local variable/parameter %s\n",s->name);
			int stack_pos = STACK_SIZE - (s->which * 8);
			printf("\tstack position for %s (%i): %i\n",s->name,s->which,stack_pos);
			if (stack_pos < 0)
			{
				printf("codegen error: No more stack allocation space (LOCAL %s, byte %i)\n",s->name,stack_pos);
				exit(1);
			}
			sprintf(sym_str, "%i", stack_pos);
			return sym_str;
		}
		// global : name in assembly language is the same as in the source language ex. count: integer --> returns count
		case SYMBOL_GLOBAL:
			return strdup(s->name);
	}
}