#include "param_list.h"
#include "symbol.h"
#include "scope.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int type_val;

/* create a param list struct */
/*
inputs
- name: name of the parameter
- type: type of the param
- next: next item in the param list
output
- p: param_list struct
*/
struct param_list * param_list_create( char *name,
									   struct type *type,
									   struct param_list *next )
{
	struct param_list* p = calloc(1, sizeof(*p));

	// p->type = type_copy(type);
	p->type = type;
	p->name = strdup(name);
	p->next = next;

	return p;
}

struct param_list * param_list_copy(struct param_list *p)
{
	if (!p) return 0;

	struct param_list *p_elem = param_list_create(p->name, p->type, 0);
	p_elem->next = param_list_copy(p->next);

	return p_elem;
}

/* param_list_compare - compare function parameter lists to see if they are the same */
/*
inputs
- a: param_list struct a
- b: param_list struct b
outputs
- integer 0 for no equivalence, 1 for equivalence
*/
int param_list_compare(struct param_list *a, struct param_list *b)
{
	struct param_list *p_a = a; // create a pointer to the first parameter list
	struct param_list *p_b = b; // create a pointer to the second parameter list

	// printf("list a: ");
	// param_list_print(a);
	// printf("\nlist b: ");
	// param_list_print(b);
	// printf("\n");

	// check to see if the parameter lists are the same
	while (p_a && p_b)
	{
		// check the types of each param in the order that they are specified
		if (!type_compare(p_a->type, p_b->type)) return 0;

		// if there an exists a next param for one list but not the other - XOR operation
		// if (!p_a != !p_b) return 0;

		// we know here that there exists a next parameter for each list, so set them
		p_a = p_a->next;
		p_b = p_b->next;
	}
	// check to see if one of the pointers still exists, both should be same length
	if (p_a || p_b) return 0;

	// hey the param lists match thats epic
	// printf("param_list_compare: param lists match\n");
	return 1;
}

/* param_list_compare_call - compare param lists for function calls */
/*
inputs
- p: param_list struct, the function definition parameters
- e: expr struct, the function call parameters in a linked list
outputs
- integer 0 for no equivalence, 1 for equivalence
*/
int param_list_compare_call(struct param_list *p, struct expr *e)
{
	if (!p && !e)
	{
		// printf("hello you've reached the end bye\n");

		return 1;
	}
	else if (!p || !e)
	{
		// printf("just kidding no param list call\n");

		return 0;
	}

	// printf("PARAM LIST COMPARE CALL between ");
	// param_list_print(p);
	// printf(" and ");
	// expr_print(e);
	// printf("\n");

	if (type_compare(p->type, expr_typecheck(e)))
	{
		if (param_list_compare_call(p->next, e->next))
		{
			return 1;
		}
		else return 0;
	}
	else return 0;

	
}


/* param_list_delete - watch out we gotta delete the param list! */
/*
inputs
p: param_list struct
*/
void param_list_delete(struct param_list *p)
{
	// if the param list doesn't exist we get outta there
	if (!p) return;

	// printf("param list delete of ");
	// param_list_print(p);
	// printf("\n");

	// delete the chracter array associated with the name of the param
	if (p->name) 
	{
		// param_list_print(p);
		// printf(" - freeing param memory\n");
		free(p->name);
	}

	// delete the type associated
	// param_list_print(p);
	// printf(" - deleting associated type\n");
	// type_delete(p->type);
	
	// delete the param list associated
	// param_list_print(p);
	// printf(" - deleting any next parameter\n");
	param_list_delete(p->next);

	// free the memory of the parameter :)
	// param_list_print(p);
	// printf(" - freeing param memory\n");
	free(p);
}

/* param_list_resolve - resolve any function parameters found within a function definition */
/*
inputs
- p: param_list struct
*/
void param_list_resolve(struct param_list *p)
{
	// enter new var decl for each param of function
	if (!p) return;


	if (p->type->kind == TYPE_AUTO)
	{
		printf("type error: function parameter %s cannot be of type auto\n",p->name);
		type_val++;
	}

	// create a symbol for the param using this param_list item's type and identifier
	p->symbol = symbol_create(SYMBOL_PARAM,p->type,p->name);

	// bind the identifier to our created symbol
	scope_bind(p->name,p->symbol);

	// print the param resolution
	printf("%s resolves to param %i\n",p->name,p->symbol->which);

	// resolve the next parameter in the param list
	param_list_resolve(p->next);
}

/* print param list struct */
/*
inputs
- p: param_list struct
*/
void param_list_print(struct param_list *p)
{
	// return nothing if param list does not exist
	if (!p) return;

	// print param list name
	printf("%s: ",p->name);
	// print associated type
	type_print(p->type);

	// if there exists another param, add a comma
	if (p->next) printf(", ");
	param_list_print(p->next);
}