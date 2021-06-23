#include "decl.h"
#include "scope.h"
#include "label.c"
#include "scratch.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

extern int type_val;
extern int resolve_val;
extern int yylineno;

extern int func_label;

/* create a declaration */
/*
inputs
- name: name of decl
- type: type of decl
- value: if an expression
- code: if a function
- next: the next declaration
output
- d: decl struct
*/
struct decl * decl_create( char *name, 
						   struct type *type, 
						   struct expr *value, 
						   struct stmt *code, 
						   struct decl *next )
{
	struct decl *d = malloc(sizeof(*d));
	d->name  = name;
	d->type  = type;
	d->value = value;
	d->code  = code;
	d->next  = next;

	return d;
}

/* RESOLVE - locate all global, local, and function parameter variables */
/*
inputs
- d: decl struct
*/
void decl_resolve(struct decl *d)
{
	// resolve nothing if it doesn't exist
	if (!d) return;

	// check if the depth of the scope is greater than one, checking whether we delve into functions w/local variables or not
	symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;

	// printf("decl symbol kind: %s\n",k);
	// printf("decl symbol type kind: ");
	// type_print(d->type);
	// printf("\n");
	// printf("decl symbol name: %s\n",d->name);

	// create a symbol for our decl, complete with its known kind, and grabbing its already-known type and name
	d->symbol        = symbol_create(kind,d->type,d->name);
	// set initial ordinal position of local variable/parameters
	d->symbol->which = 0;

	// resolve whatever expressions come with this delcaration
	expr_resolve(d->value);
	// bind the name of the decl to the symbol we created
	scope_bind(d->name,d->symbol);

	/* print resolve */
	if          (kind == SYMBOL_GLOBAL) 
	{
		if      (d->type->kind == TYPE_FUNCTION) printf("%s resolves to global %s (FUNCTION)\n",d->name,d->name);
		else if (d->type->kind == TYPE_PROTO   ) printf("%s resolves to global %s (PROTOTYPE)\n",d->name,d->name);
		else                                     printf("%s resolves to global %s\n",d->name,d->name);
	} 
	else if     (kind == SYMBOL_LOCAL )          printf("%s resolves to local %i\n",d->name,d->symbol->which);

    // we got a function, enter parameter list scope and its statements
	if ((d->type->kind == TYPE_FUNCTION) || (d->type->kind == TYPE_PROTO))
	{
		scope_enter(); 						 // create a new hash table scope for the function

		param_list_resolve(d->type->params); // resolve the parameter list for the function
		stmt_resolve(d->code);				 // resolve any statements found within the function

		scope_exit(); 						 // remove this hash table scope
	}

	// printf("\n");
	// resolve the next connected declaration
	decl_resolve(d->next);
}

/* DECL TYPECHECK - simply confirms that variable declarations amtch their initializers and otherwise typechecks the body of function delcarations */
/*
inputs
- d: decl struct
*/
void decl_typecheck(struct decl *d)
{
	// if there's no decl to typecheck then get out
	if (!d) return;

	// printf("expr kind: %i\n", d->value->kind);
	// if (d->value->literal_value)  printf("expr literal value: %c\n",d->value->literal_value);
	// if (d->value->string_literal) printf("expr string literal: %s\n",d->value->string_literal);
	// printf("expr type: %i\n", t->kind);

	// printf("typechecking decl of %s\n", d->name); // fflush(stdout);

	if (d->type->kind == TYPE_FUNCTION) // function typechecking
	{
		// printf("function declared: %s ", d->name);
		// type_print(d->type);
		// printf("\n");
		if (d->type->params)
		{
			// compare the function definition with an already-resolved name
			if (!param_list_compare(d->type->params, d->symbol->type->params))
			{
				printf("type error: declaration of %s and its prototype have different types\n", d->name);
				type_val++;
			} 
		}

		// typecheck the return statements of the function and check for any errors
		struct stmt *dcode;
		dcode = d->code;    // set up a pointer for the code inside the function expression
		while (dcode)
		{
			// printf("back at the top\n");
			// search through all statements until we happen upon a return statement
			if (dcode->kind == STMT_RETURN)
			{
				// printf("funny");
				// type_print(d->type);
				// printf("\nand ");
				// type_print(d->type->subtype);
				printf("\r"); // somehow this prevents a seg fault

				struct type *func_return_type = d->type->subtype;
				// if function is an auto, just assign its return type to whatever the return statement says
				if (func_return_type->kind == TYPE_AUTO)
				{
					if (dcode->expr->kind) // return type expression exists
					{
						func_return_type = type_copy(expr_typecheck(dcode->expr));
						d->symbol->type  = type_copy(expr_typecheck(dcode->expr));
					}
					else // set return type to void
					{
						func_return_type = type_create(TYPE_VOID, 0, 0, 0);
						d->symbol->type  = type_create(TYPE_VOID, 0, 0, 0);
					}
					printf("notice: return type of function %s is ",d->symbol->name);
					type_print(d->symbol->type);
					printf("\n");
				}
				// if function is a void, make sure it doesn't actually return anything
				else if (func_return_type->kind == TYPE_VOID)
				{
					if (dcode->expr->kind) // if the return type isn't already void
					{
						printf("type error: void function %s must have void return statement\n", d->name);
						type_val++;
					}
					// set the expr return to void
					dcode->expr->kind = TYPE_VOID;
				}
				// check the function subtype against the type of the variable being returned
				else if (!type_compare(d->type->subtype, expr_typecheck(dcode->expr)))
				{
					printf("type error: type mismatch between function %s and return value\n", d->name);
					type_val++;
				}
			}

			// look at any next statement, otherwise exit
			if (dcode->next) dcode = dcode->next;
			else             dcode = 0;
		}
	}
	else if (d->value) // variable declaration typechecking
	{
		struct type *t;
		t = expr_typecheck(d->value);

		// there's auto declaration going on
		if (d->type->kind == TYPE_AUTO)
		{
			if (d->value) 
			{
				d->type         = type_copy(expr_typecheck(d->value));
				d->symbol->type = type_copy(expr_typecheck(d->value));
				printf("notice: type of %s is ",d->symbol->name);
				type_print(d->symbol->type);
				printf("\n");
			}
			else
			{
				printf("type error: cannot assign type to auto if value is undeclared\n");
				type_val++;
			}
		}
	
		// do some array typechecking
		if (d->type->kind == TYPE_ARRAY)
		{
			struct expr *elem = d->value; // the expression related to an array is its first element

			int elem_count = 0;  		  // to count the numbers and check for size

			// while we have array elements to look at, compare their types to the type of the array
			while (elem)
			{	
				// compare the type of the array to the type of the element
				if (!type_compare(d->type->subtype, expr_typecheck(elem))) // type_compare(array, type)
				{
					printf("type error: array type and item declaration do not match");
					type_val++;
				}

			    // oh hey this is an element
				elem_count++;

				// point to whatever the next element in the array is if it exists
				if (elem->next) elem = elem->next;
				else            elem = 0;
			}

			// if there exists an element count but not the exact amount as the declared size
			if (elem_count && (elem_count != d->type->size))
			{
				printf("type error: array %s declaration has %i element(s) instead of %i\n", d->name, elem_count, d->type->size);
				type_val++;
			}
		}
		// compare any other type declaration with its initialization
		else if (d->type->kind != TYPE_ARRAY && !type_compare(t,d->symbol->type))
		{
			/* display an error */
			printf("type error: declaration of %s and initialization do not match types\n", d->name);
			type_val++;
		}

		// was the declaration of some type void (barring function definitions)
		if (d->type->kind == TYPE_VOID && d->type->kind != TYPE_FUNCTION)
		{
			printf("type error: cannot declare variable with type void\n");
			type_val++;
		}
	}

	
	if (d->code)
	{
		// printf("typecheck of next code statement\n");
		// stmt_print(d->code, 0);
		stmt_typecheck(d->code); // typecheck the following statement code
	}

	// printf("typecheck of next decl\n");
	decl_typecheck(d->next); // typecheck the following declaration
}

/* code generation for a declaration wow */
void decl_codegen(struct decl *d, FILE *outfil)
{
	// if no decl exists to generate code for then don't
	if (!d) return;

	// looking at declared scope first, types second
	switch (d->symbol->kind)
	{
		case SYMBOL_LOCAL:
			printf("generate code for local variable %s\n",d->symbol->name);
			switch (d->type->kind)
			{
				case TYPE_VOID: 	 // 0
					printf("codegen error: No such thing as a local type void\n");
					exit(1);
				case TYPE_BOOLEAN:   // 1
				case TYPE_CHARACTER: // 2
				case TYPE_INTEGER:   // 3
					// generate code for this epxression and place the reg value in d->value->reg
					expr_codegen(d->value, outfil);
					// then we need to store this register of our local variable onto the stack
					if (d->value) // check if there's a value we need to store
					{
						printf("\tstr\t%s, [sp, %s]\n", scratch_name(d->value->reg), symbol_codegen(d->symbol));
						fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(d->value->reg), symbol_codegen(d->symbol));
						scratch_free(d->value->reg);
					}
					else // no expression coupled with variable, allocate a register ourselves
					{
						int no_assign_reg = scratch_alloc();
						printf("\tstr\t%s, [sp, %i]\n", scratch_name(no_assign_reg), symbol_codegen(d->symbol));
						// fprintf(outfil, "sup\n");
						fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(no_assign_reg), symbol_codegen(d->symbol));
						scratch_free(no_assign_reg);
					}
					break;
				case TYPE_STRING:    // 4
					// TODO for now free up these registers when done, if it causes problems then change later

					// generate code for this epxression and place the reg value in d->value->reg
					expr_codegen(d->value, outfil);
					// then we need to store this register of our local variable onto the stack
					if (d->value) // check if there's a value we need to store
					{
						fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(d->value->reg), symbol_codegen(d->symbol));
						scratch_free(d->value->reg);
					}
					else // no expression coupled with variable, allocate a register ourselves
					{
						int no_assign_reg = scratch_alloc();
						fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(no_assign_reg), symbol_codegen(d->symbol));
						scratch_free(no_assign_reg);
					}
					break;
				case TYPE_ARRAY:     // 5
					printf("codegen error: No such thing as a local type array\n");
					exit(1);
				case TYPE_FUNCTION:  // 6
					printf("codegen error: No such thing as a local type function\n");
					exit(1);
				case TYPE_PROTO:     // 7
					printf("codegen error: No such thing as a local type function prototype\n");
					exit(1);
				case TYPE_AUTO:      // 8
					printf("codegen error: No such thing as a local type auto\n");
					exit(1);
			}
			break;
		case SYMBOL_GLOBAL:
			printf("generate code for global variable %s\n",d->symbol->name);
			switch (d->type->kind)
			{
				case TYPE_VOID: 	 // 0
					break;
				case TYPE_BOOLEAN:   // 1 // making global bool, char, and int make the same code until further notice
				case TYPE_CHARACTER: // 2
				case TYPE_INTEGER:   // 3
					if (d->value->literal_value)
					{
						// 8 bytes for every allocation
						fprintf(outfil, "\t.global %s\n", d->name);
						fprintf(outfil, "\t.data\n"); // is this still necessary after the first global var?
						fprintf(outfil, "\t.align\t3\n");
						fprintf(outfil, "\t.type\t%s, %%object\n",d->name);
						fprintf(outfil, "\t.size\t%s, 8\n",d->name);

						// print label for the variable name
						fprintf(outfil, "%s:\n",d->name);

						// var value
						fprintf(outfil, "\t.xword\t%d\n",d->value->literal_value);
					}
					else // no initialization
					{
						// 8 bytes for every allocation
						fprintf(outfil, "\t.comm\t%s,8,8\n", d->name);
					}
					break;
				case TYPE_STRING:    // 4
					if (d->value->string_literal)
					{
						// 8 bytes for every allocation
						fprintf(outfil, "\t.global %s\n", d->name);
						// fprintf(outfil, "\t.data\n"); // is this still necessary after the first global var?
						fprintf(outfil, "\t.section\t.rodata\n");
						fprintf(outfil, "\t.align\t3\n");

						// label creation
						int str_label = label_create();
						fprintf(outfil, "%s:\n", label_name(str_label));
						fprintf(outfil, "\t.string\t%s\n", d->value->string_literal);
						fprintf(outfil, "\t.section\t.data.rel.local,\"aw\"\n");
						fprintf(outfil, "\t.align\t3\n");

						fprintf(outfil, "\t.type\t%s, %%object\n",d->name);
						fprintf(outfil, "\t.size\t%s, 8\n",d->name);

						// print label for the variable name
						fprintf(outfil, "%s:\n",d->name);

						// var value
						fprintf(outfil, "\t.xword\t%s\n", label_name(str_label));
					}
					else // no initialization
					{
						// 8 bytes for every allocation
						fprintf(outfil, "\t.comm\t%s,8,8\n", d->name);
					}
					break;
				case TYPE_ARRAY:     // 5
					// failsafe for only having global 1D integer arrays
					if (d->type->subtype->kind != TYPE_INTEGER)
					{
						printf("codegen error: only 1D arrays of integers are supported.\n");
						exit(1);
					}
					// otherwise let's make an array
					struct expr *elem_p = d->value;
					if (elem_p) // array has elements
					{
						// 8 bytes for every allocation
						fprintf(outfil, "\t.global %s\n", d->name);
						fprintf(outfil, "\t.data\n"); // is this still necessary after the first global var?
						fprintf(outfil, "\t.align\t3\n");
						fprintf(outfil, "\t.type\t%s, %%object\n",d->name);
						fprintf(outfil, "\t.size\t%s, %i\n",d->name,d->type->size*8);

						// print label for the variable name
						fprintf(outfil, "%s:\n",d->name);

						// element values
						while (elem_p)
						{
							fprintf(outfil, "\t.xword\t%d\n",elem_p->literal_value);
							elem_p = elem_p->next;
						}
					}
					else // no elements declared
					{
						fprintf(outfil, "\t.comm\t%s,%i,8\n",d->name,d->type->size*8);
					}
					break;
				case TYPE_FUNCTION:  // 6
					// check if there's code associated with the function, otherwise there's no point in generating it
					/*
					- Global func decls
					  - emit label with function's name
					  - follow with func prologue
					    - must take into account the num of params and local vars
					    - make appropriate amount of space on the stack
					  - body of the function
					  - function epilogue
					    - should have unique label so that return statements can easily jump there
					*/

					if (d->code)
					{
						fprintf(outfil, "\t.text\n"); // no clue if this is right to keep or not
						fprintf(outfil, "\t.align\t2\n");
						fprintf(outfil, "\t.global\t%s\n",d->name);
						fprintf(outfil, "\t.type\t%s, %%function\n",d->name);

						// label for the function code
						fprintf(outfil, "%s:\n",d->name);
						
						// set prologue based on existence of function call
						// store the frame pointer (x29) @ sp and link register (x30) @ sp+8
						fprintf(outfil, "\tstp\tx29, x30, [sp, -%i]!\n", STACK_SIZE);
						// make the value of the frame pointer the same as the value of the stack pointer
						fprintf(outfil, "\tmov\tx29, sp\n");
						
						// store arguments immediately
						// these arguments will be loaded into arg registers x0-x7
						struct param_list *param = d->type->params;
						int arg_start = STACK_SIZE - 16;
						int arg_num = 0;
						while (param)
						{
							// push old values of argument registers into the stack, 8 bytes
							// scratch_alloc(); // allocate register for a parameter
							/*
							arg_start -= 8; // 8 bytes per register
							fprintf(outfil, "\tstr\t%s, [sp, %d]\n", scratch_name(arg_num), arg_start);
							arg_num++; // increase the argument counter
							*/

							fprintf(outfil, "\tstr\tx%i, [sp, %s]\n", param->symbol->which-1, symbol_codegen(param->symbol));

							// go to the next parameter
							param = param->next;
						}

						// create a function label so we can branch to the epilogue at any return statement
						func_label = label_create_func();

						// code output for the function content
						stmt_codegen(d->code, outfil);

						// create function epilogue
						fprintf(outfil, "%s:\n", label_name(func_label));

						// load the stack pointer and link register
						fprintf(outfil, "\tldp\tx29, x30, [sp], %d\n", STACK_SIZE);	

						// TODO double check to see that this is the right action to take
						// run a nop for a void function (probably)
						if (d->type->subtype->kind == TYPE_VOID)
						{
							fprintf(outfil, "\tnop\n");
						}

						// return from the function
						fprintf(outfil, "\tret\n");

						// there's also this extra size line that I don't understand much about
						fprintf(outfil, "\t.size\t%s, .-%s\n", d->name, d->name);
					}
					break;
				case TYPE_PROTO:     // 7
					break;
				case TYPE_AUTO:      // 8
					break;
			}
			// fprintf(outfil, "\n");
			break;
	}

	// generate code for the following declaration
	decl_codegen(d->next, outfil);
}

/* print a declaration */ 
/*
inputs:
- d: decl struct
- indent: amount of tabs/indent spaces to place before printing the declaration
*/
void decl_print(struct decl *d, int indent)
{
	// return nothing if delcaration does not exist
	if (!d) return;

	// start printing some declarations
	// identifier
	printf("%s: ",d->name);

	// type of identifier
	type_print(d->type);
	if (d->type->kind == TYPE_ARRAY) // if the type of identifier is an array, we gotta print the elements it was declared with
	{
		struct expr* ptr = d->value; // look at the value of the decl
		if (ptr) 					 // if a value exists, which would be its array elements
		{
			// start to print the array elements
			printf(" = {");

			// if the array is 2D+
			if (ptr->right)
			{
				// print different {} sets of elements
				while (ptr)
				{
					printf("{");
					exprs_print(ptr);
					printf("}");
					if (ptr->right) printf(", ");

					ptr = ptr->right;
				}
			}
			else // if the array is 1D
			{
				exprs_print(ptr);
			}

			printf("}");
		}
	}
	else if (d->value) // otherwise print the value associated with the declaration
	{
		printf(" = ");
		expr_print(d->value);
	}

	if (d->type->kind == TYPE_FUNCTION) // if the type of an identifier is a function
	{
		if (d->code) // if there's code associated with the function that we gotta print
		{
			printf(" = \n{\n");

			stmt_print(d->code,indent+1);
			printf("}");
		}
		else // otherwise it's just a prototype function
		{
			printf(";");
		}
	}
	else // otherwise we end off the declaration with a semicolon
	{
		printf(";");
	}

	printf("\n");

	// print the next declaration
	decl_print(d->next,indent);
}