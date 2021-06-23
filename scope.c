#include "scope.h"

extern int resolve_val;
extern int type_val;

struct scope_stack *head = 0;

/* cause new hash table to be pushed on the top of the stack, representing a new scope */
void scope_enter()
{
	// allocate memory for new scope stack
	struct scope_stack *temp = calloc(1, sizeof(*temp));

	// create new hash table and push it to the top of the stack, much like adding an elem to a linked list would be like
	temp->hash = hash_table_create(0, 0);
	temp->next = head;

	// if the scope level exceeeds a depth of 1, we have variables on a local scale, otherwise they're global
	if (scope_level() >= 1) temp->symbl = SYMBOL_LOCAL;
	else                    temp->symbl = SYMBOL_GLOBAL;

	// set the new head to the top of the stack
	head = temp;

	return;
}

/* the topmost hash table is removed */
void scope_exit()
{
	// if no head exists return nothing
	if (!head) return;

	// declare a scope stack head
	struct scope_stack *temp = head;
	// get rid of the hash table associated with the head
	hash_table_delete(temp->hash);

	// set the new head as the next scope in the stack
	head = head->next;
	// free up the memory of the previous head
	free(temp);

	return;
}

/* returns number of hash tables in current stack (tells if we are in global scope or not) */
int scope_level()
{
	// if no head exists return nothing
	if (!head) return 0;

	// level starts out as 1 (base level)
	int level = 1;
	// declare a scope stack head
	struct scope_stack *temp = head;

	// while theres another scope in the stack to move to
	while (temp->next)
	{
		level++; 		 	// increase level amount
		temp = temp->next;  // keep moving down the stack
	}

	return level; 			// return our level
}

/* adds an entry to the topmost hash table of the stack, mapping name to the symbol structure sym */
void scope_bind(const char *name, struct symbol *sym)
{
	head->loc++;            // increment the location value of the head
	sym->which = head->loc; // update ordinal position of local var/param with scope head location

	// insert new symbol into the hash table
	// if key already exists in the hash table (name), return 0
	// if symbol is successfully added into the table, return 1
	int result = hash_table_insert(head->hash,strdup(name),(void*) sym);

	if (!result) // symbol was unable to be inserted, therefore (maybe) generate an error message
	{
		// encountered a function implementation
		if (sym->type->kind == TYPE_FUNCTION)
		{
			// the only way that a func implementation should pass right now is if there's a func prototype that preceeds it
			struct symbol *proto_check;
			proto_check = scope_lookup_current(name);

			if (proto_check->type->kind == TYPE_PROTO) // did we pass the proto check
			{
				// check to see if both functions are of the same return type
				struct type *proto_type = proto_check->type;
				struct type *func_type  = sym->type;
				if (!type_compare(proto_type->subtype,func_type->subtype))
				{
					printf("type error: prototype function (");
					type_print(proto_type->subtype);
					printf(") and function declaration (");
					type_print(func_type->subtype);
					printf(") have different return types\n");
					type_val++;
				}

				// let's also check to see if they have the same parameter list types
				if (proto_type->params && func_type->params)
				{
					if (!param_list_compare(proto_type->params,func_type->params))
					{
						printf("type error: %s prototype parameter list does not match function parameter list\n", name);
						type_val++;
					}
				}
				else if (proto_type->params || func_type->params)
				{
					printf("type error: function prototype and declaration parameter lists do not match\n");
					type_val++;
				}

				// update the type of our proto and leave
				proto_type->kind = TYPE_FUNCTION;
				return;
			}
		}

		// there's no other possible way we wouldn't have an error, so we generate
		printf("resolve error: redeclaring symbol %s within same scope\n", name);
		resolve_val++;
	}

	return;
}

/* searches the stack of hash tables from top to bottom, looks for first entry that matches "name" exactly. Return null if no match found */
struct symbol *scope_lookup(const char *name)
{
	struct symbol *res = NULL;
	struct scope_stack *temp = head;

	while (temp)
	{
		res = (struct symbol*) hash_table_lookup(temp->hash, name);

		if (res)             
		{
			// printf("res name: %s\n", res->name);
			// printf("res type %i\n", res->type->kind);
			return res;
		}
		else if (temp->next) temp = temp->next;
		else                 return 0;
	}

	return 0;
}

/* works like scope lookup, except that it only searches the topmost table. Used to determine whether a symbol has already been defined in the current scope */
struct symbol *scope_lookup_current(const char *name)
{
	return (struct symbol*) hash_table_lookup(head->hash, name);
}