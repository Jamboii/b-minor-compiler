#include "type.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* create a type */
/*
inputs
- kind: type kind
- subtype: subtype of kind
- param_list: applies to function params
- size: for arrays, size of array declaration
output
- t: type struct
*/
struct type * type_create( type_t kind,
						   struct type *subtype,
						   struct param_list *params,
						   int size
						   )
{
	struct type* t = malloc(sizeof(*t));
	t->kind = kind;
	t->subtype = subtype;
	t->params = params;
	t->size = size;

	return t;
} 

/* type_copy helper function to create a copy of an existing type */
/*
inputs
- t: type struct
outputs
- a copy of the input type struct
*/
struct type * type_copy(struct type *t)
{
	// return nothing if the type struct doesn't exist
	if (!t) return 0;

	// printf("copying type: ");
	// type_print(t);
	// printf("\n");

	return type_create(t->kind, type_copy(t->subtype), param_list_copy(t->params), t->size);
}

/* type_compare - Helper function to compare different type structs and see if they are the same */
/*
inputs
- a: type struct a
- b: type struct b
outputs
- integer: 0 for no equivalence between types, 1 for equivalence
*/
int type_compare(struct type *a, struct type *b)
{
	// if there are two functions we gotta make sure they are the same subtype AND have the same params used
    if      (a->kind == TYPE_FUNCTION && b->kind == TYPE_FUNCTION)
    {
    	// printf("TYPE COMPARE: functions\n");
    	if (type_compare(a->subtype,b->subtype) && param_list_compare(a->params,b->params)) return 1;
    	else 																				return 0;
	}
	// if there are two arrays we gotta make sure they're the same subtype
	else if (a->kind == TYPE_ARRAY && b->kind == TYPE_ARRAY)
	{
		// printf("TYPE COMPARE: arrays\n");
		if (type_compare(a->subtype,b->subtype)) return 1;
		else                                     return 0;
	}
	// otherwise, compare types by their kind, and if they match, return true
    else if (a->kind == b->kind) // return 1;
    {
    	// printf("TYPE COMPARE: kinds of ");
    	// type_print(a);
    	// printf(" and ");
    	// type_print(b);
    	// printf(" match\n");
    	return 1;
    }
	// else return FALSE
	return 0;
}

/* Remove a type from memory */
void type_delete(struct type *t)
{
	// if the type doesn't exist in the first place we return
	if (!t) return;

	// printf("type delete of ");
	// type_print(t);
	// printf("\n");

	// if we got a function then we gotta delete that associated param list
	if (t->kind == TYPE_FUNCTION) 
	{
		// type_print(t);
		// printf(" - type is function, deleting parameters\n");
		param_list_delete(t->params);
	}

	// type_print(t);
	// printf(" - deleting any subtypes\n");
	// delete any existing subtypes
	type_delete(t->subtype);

	// type_print(t);
	// printf(" - freeing type memory\n");
	// free up that memory
	free(t);
}

/* print a type struct */
/*
input
- type: type struct
*/
void type_print(struct type *t)
{
	// return the type if it just does not exist
	if (!t) return;

	// print type based on its kind
	switch (t->kind)
	{
		case TYPE_VOID:
			printf("void");
			break;
		case TYPE_BOOLEAN:
			printf("boolean");
			break;
		case TYPE_CHARACTER:
			printf("char");
			break;
		case TYPE_INTEGER:
			printf("integer");
			break;
		case TYPE_STRING:
			printf("string");
			break;
		case TYPE_AUTO:
			printf("auto");
			break;
		case TYPE_ARRAY:
			printf("array [");
			
			// print the size of the array if it exists
			if (t->size) printf("%d", t->size);

			printf("] ");
			
			type_print(t->subtype);
			break;
		case TYPE_FUNCTION:
		case TYPE_PROTO:
			printf("function ");

			// if there exists a subtype of the function we print it
			if (t->subtype)
				type_print(t->subtype);

			// print any params that come from defining the function
			if (t->params)
			{
				printf(" ( ");
				param_list_print(t->params);
				printf(" )");	
			}
			else // otherwise print associated no arguments
			{
				printf(" ()");
			}

			// printf("\nleaving func decl\n");

			break;
	}

	return;
}