#include "expr.h"
#include "scope.h"
#include "scratch.h"
#include "label.h"
#include "library.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int type_val;
extern int resolve_val;
extern int yylineno;

extern int reg_table[6];

/* create an expression*/
/*
inputs
- kind: expression type
- L: sitting to the left of the expression
- R: sitting to the right of the expression
output
- e: expr struct
*/
struct expr * expr_create( expr_t kind,
						   struct expr *L,
						   struct expr *R )
{
	struct expr *e = calloc(1, sizeof(*e));
	e->kind  = kind;
	e->left  = L;
	e->right = R;

	return e;
}

/* create a name for an expression */
struct expr * expr_create_name(const char *name)
{
	struct expr *e = expr_create(EXPR_NAME, 0, 0);

	e->name = name;

	return e;
}

/* create an integer literal for an expression */
struct expr * expr_create_integer_literal(int int_val)
{
	struct expr *e = expr_create(EXPR_INT_LITERAL, 0, 0);

	e->literal_value = int_val;

	return e;
}

/* create a boolean for an expression */
struct expr * expr_create_boolean_literal(int bool_val)
{
	struct expr *e = expr_create(EXPR_BOOLEAN_LITERAL, 0, 0);

	e->literal_value = bool_val;

	return e;
}

/* create a char for an expression */
struct expr * expr_create_char_literal(char char_val)
{
	struct expr *e = expr_create(EXPR_CHAR_LITERAL, 0, 0);

	e->literal_value = char_val;

	return e;
}

/* create a string for an expression */
struct expr * expr_create_string_literal(const char *str_val)
{
	struct expr *e = expr_create(EXPR_STRING_LITERAL, 0, 0);

	// printf("Adding string %s\n",str_val);

	e->string_literal = str_val;

	return e;
}

/* expr_resolve - resolve the current expression by looking it up in the scope */
/*
inputs
- e: expr struct
*/
void expr_resolve(struct expr *e)
{
	// resolve nothing if it doesn't exist 
	if (!e) return;

	// check if the kind of expression is an identifier
	if (e->kind == EXPR_NAME)
	{
		// printf("assigning a symbol to %s\n",e->name);
		// try to link a symbol from the existing scope to the variable name
		e->symbol = scope_lookup(e->name);

		// scope_bind(e->name,e->symbol);
		// if we can't actually link any sorta symbol name to our identifier theres a resolution error
		if (!e->symbol)
		{
			printf("resolve error: %s is not defined\n",e->name);
			resolve_val++;
		}

		// printf("COMPLETED assigning a symbol to %s\n",e->name);
	}

	// if we don't have an identifier just look at every other expression linked to this one until we do
	expr_resolve(e->next);  // resolve the "next" expression, which will be some new array element

	expr_resolve(e->left);  // resolve the left expression
	expr_resolve(e->right); // resolve the right expression
	
}

/* exprs_resolve - resolve multiple expressions joined together by some linked list, for function parameter lists or array elements */
/*
inputs
- e: expr struct, has a next attribute associated
*/
void exprs_resolve(struct expr *e)
{
	// resolve nothing if it doesn't exist
	if (!e) return;
	// resolve our current expression
	expr_resolve(e);
	// but also resolve the next expression, doesn't matter if it exists or not
	exprs_resolve(e->next);
}

/* expression type checker */
/*
- computes the type of an expression recursively and return a new type object to represent it
- should check for errors within the expresision
- the result of this typecheck method should be used to compare against expectations in stmt_typecheck and decl_typecheck
*/
struct type * expr_typecheck(struct expr *e)
{
	// if expression doesn't exist we gotta go
	if (!e) return 0;

	// fflush(stdout);

	struct type *l = expr_typecheck(e->left);
	struct type *r = expr_typecheck(e->right);
	struct type *res;

	// printf("expr typecheck ");
	// expr_print(e);
	// printf("\n");
	// if (e->name)          printf("expr name: %s\n",e->name);
	// if (e->literal_value)
	// {
	// 	if (e->kind == EXPR_CHAR_LITERAL) printf("expr literal value: %c\n",e->literal_value);
	// 	else                              printf("expr literal value: %i\n",e->literal_value);
	// }
	// if (e->string_literal)printf("expr string literal: %s\n",e->string_literal);

	switch (e->kind)
	{
		case EXPR_ASSIGN:			// 0
			// if the left expression exists
			if (l)
			{
				if (l->kind == TYPE_AUTO) // type auto must be reassigned to a new type
				{
					l = type_copy(r); // just copy the type of the RHS of the assignment
					printf("notice: type of %s is ",e->left->name);
					type_print(r);
					printf("\n");
				}
				else if (!type_compare(l,r)) // if the types of the exprs on the left and the right side don't match at all
				{
					printf("type error: you can't assign a variable %s with a value of a different type silly\n", e->left->name);
					type_val++;
				}
				else if (l->kind == TYPE_ARRAY && r->kind == TYPE_ARRAY) // do we have arrays on the left and right sides
				{
					if (!type_compare(l->subtype,r->subtype))
					{
						printf("type error: cannot assign two arrays %s and %s of different subtypes\n", e->left->name, e->right->name);
					}
				}

				res = type_copy(l);
			}
			break;
		case EXPR_ADD:				// 1
		case EXPR_SUB:				// 2
		case EXPR_MUL:				// 3
		case EXPR_DIV:				// 4
		case EXPR_MOD:				// 5
		case EXPR_EXP:				// 6
			// check to see if either side of the epxression is a non-integer
			if (l->kind != TYPE_INTEGER || r->kind != TYPE_INTEGER)
			{
				// printf("type error: cannot perform a binary operation on a non-integer type\n");
				printf("type error: cannot perform a binary operation between ");
				type_print(r);

				printf(" (");
				expr_print(e->right);

				printf(") and");
				type_print(l);

				printf(" (");
				expr_print(e->left);

				printf(")\n");

				type_val++;
			}
			// create an integer type for this expression
			res = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_LE:				// 7
		case EXPR_LT:				// 8
		case EXPR_GE:				// 9
		case EXPR_GT:				// 10
			// check to see if either operation of the comparison is a non-integer
			if (l->kind != TYPE_INTEGER || r->kind != TYPE_INTEGER)
			{
				printf("type error: cannot perform a logical operation on non-integer type ");
				// print an error message for an invalid left hand side
				if (l->kind != TYPE_INTEGER)
				{
					type_print(l);

					printf(" (");
					expr_print(e->left);

					printf(")\n");
					type_val++;
				}
				// print an error message for an invalid right hand side
				if (r->kind != TYPE_INTEGER)
				{
					type_print(r);

					printf(" (");
					expr_print(e->right);

					printf(")\n");
					type_val++;
				}
				// type_val++;
			}
			// create a boolean type for this expression
			res = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;
		case EXPR_EQ:				// 11
		case EXPR_NEQ:				// 12
			// the types on both sides of this comparison need to be the same, check to see if this is true
			if (!type_compare(l,r))
			{
				printf("type error: ");
				type_print(l);
				expr_print(e->left);

				printf(" and ");
				type_print(r);
				expr_print(e->right);

				printf("are of different types\n");

				type_val++;
			}

			if (l->kind == TYPE_FUNCTION ||
				l->kind == TYPE_ARRAY    ||
				l->kind == TYPE_VOID )
			{
				printf("type error: cannot compare equality for ");
				type_print(l);

				printf("\n");

				type_val++;				
			}

			// create a boolean type for this expression
			res = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;			
		case EXPR_AND:				// 13
		case EXPR_OR:				// 14
			// may only be applied to boolean values nice
			if (l->kind != TYPE_BOOLEAN || r->kind != TYPE_BOOLEAN)
			{
			 	printf("type error: cannot perform logical operation on non-boolean ");
				// print an error message for an invalid left hand side
				if (l->kind != TYPE_BOOLEAN)
				{
					type_print(l);

					printf("(");
					expr_print(e->left);

					printf(")");
					type_val++;
				}
				// print an error message for an invalid right hand side
				if (r->kind != TYPE_BOOLEAN)
				{
					if (l->kind != TYPE_BOOLEAN) printf(" and ");
					type_print(r);

					printf("(");
					expr_print(e->right);

					printf(") ");
					type_val++;
				}
				printf("\n");
			}
			// create a boolean type for this expression
			res = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;		
		case EXPR_NOT:				// 15
			// may only be applied to boolean values nice
			if (r->kind != TYPE_BOOLEAN)
			{
				printf("type error: cannot negate a non-boolean ");
				type_print(r);
				printf("(");
				expr_print(e->right);

				printf(")\n");
				type_val++;
			}
			// create a boolean type for this expression
			res = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;	
		case EXPR_NEG:				// 16
			if (r->kind != TYPE_INTEGER)
			{
				printf("type error: cannot negate non-integer ");
				type_print(r);

				printf(" (");
				expr_print(e->right);

				printf(")\n");

				type_val++;
			}
			res = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_INCR:				// 17
		case EXPR_DECR:				// 18
			if (l->kind != TYPE_INTEGER)
			{
				printf("type error: cannot perform a postfix operation on non-integer ");
				type_print(l);

				printf(" (");
				expr_print(e->left);

				printf(")\n");

				type_val++;
			}
			res = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_GROUP:			// 19
			// printf("GROUP ");
			// type_print(expr_typecheck(e->right));
			// printf("\n");
			// copy the type
			res = type_copy(expr_typecheck(e->right));
			break;
		case EXPR_ARRELEM:			// 20
			// are we looking at an array's elements here
			if (l->kind == TYPE_ARRAY)
			{
				// are we trying to get into a non-integer access
				if (r->kind != TYPE_INTEGER)
				{
					printf("type error: cannot access %s array with non-integer \n", e->left->name);
					type_val++;
				}
				// if the array multi-dimensional
				if (e->right->right)
				{
					struct expr *r_expr = e->right;

					while (r_expr)
					{
						// constantly look at each consecutive array access
						if (expr_typecheck(r_expr)->kind != TYPE_INTEGER)
						{
							printf("type error: cannot access %s array with non-integer \n", e->left->name);
							type_val++;
						}

						if (r_expr->right) r_expr = r_expr->right;
						else               r_expr = 0;
					}
				}
				// add multiple array accesses
				res = type_copy(l->subtype);
			}
			else // hey we got an error, this ain't an array
			{
				printf("type error: cannot index string %s with non-integer\n", e->left->name);
				res = type_copy(l);
				type_val++;
			}
			break;
		case EXPR_INT_LITERAL:		// 21
			res = type_create(TYPE_INTEGER, 0, 0, 0); 	// create a new integer type
			break;
		case EXPR_BOOLEAN_LITERAL: 	// 22
			res = type_create(TYPE_BOOLEAN, 0, 0, 0); 	// create a new boolean type
			break;
		case EXPR_CHAR_LITERAL:		// 23
			res = type_create(TYPE_CHARACTER, 0, 0, 0); // create a new character type
			break;
		case EXPR_STRING_LITERAL:	// 24
			res = type_create(TYPE_STRING, 0, 0, 0);	// create a new string type
			break;
		case EXPR_NAME:				// 25
			// printf("copying identifier %s\n",e->name);
			// printf("kind %i\n",e->symbol->type->kind);
			// if (e->symbol)
			// {
			// 	printf("theres a symbol here\n");
			// }
			// else
			// {
			// 	printf("there's no symbol here\n");
			// }
			res = type_copy(e->symbol->type); 			// copy the symbol type
			break;
		case EXPR_FUNCCALL:			// 26
			// printf("Function call has been MADE\n");
			if (l->kind == TYPE_FUNCTION)
			{

				struct param_list *p = e->left->symbol->type->params; // pointer to param list of actual function decl
				struct expr *er      = e->right; 					  // pointer to the first param of a linked list for the function call

				// make sure the types of the vars used in the function call are the same as the types of the variables in the declaration
				if (!param_list_compare_call(p, er))
				{
					printf("type error: parameters not matching in function call of %s\n", e->left->name);
					type_val++;
				}
				res = type_copy(l->subtype); // this is the return type of the function call
			}
			else // you're making a call to a non-function, stop that
			{
				printf("type error: cannot call non-function %s\n", e->left->name);
				res = type_copy(l); // create a type copy to return anyway

				type_val++;
			}
			break;
	}

	type_delete(l);
	type_delete(r);

	return res;
}

/*
- recursively calls itself for its left and right children
- each child will generate code such that the result will be left in the reg num noted in the reg field
- current node generates code using those registers
- current node frees the registers it no longer needs
extra stuff
- not all symbols are simple global vars
- when a symbol forms part of an instruction, symbol_codegen needs to return the string that gives the specific address for that symbol
*/
/* expression code generation */
void expr_codegen(struct expr *e, FILE *outfil)
{
	// if there's no expression to generate code for then leave
	if (!e) return;

	// generate code based on type of expression
	switch (e->kind)
	{
		// Interior node: generate children, then add them
		case EXPR_ASSIGN:			// 0
			printf("assignment time\n");
			// generate code for both sides of the assignment
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);
			// store the resulting register (left) onto the stack
			// fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(e->left->reg), symbol_codegen(e->right->symbol));
			if (e->left->kind == EXPR_ARRELEM)
			{
				int idx = e->left->right->literal_value;
				printf("array elem: index %i\n",idx*8);
				// load the array again cause I'm lazy
				fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->left->reg), e->left->left->name);
				fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), e->left->left->name);
				fprintf(outfil, "\tstr\t%s, [%s, %i]\n", scratch_name(e->right->reg), scratch_name(e->left->reg), idx*8);
			}
			else if (e->left->symbol->kind == SYMBOL_LOCAL)
			{
				printf("not an array element\n");
				// fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(e->left->reg), symbol_codegen(e->right->symbol));
				fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(e->right->reg), symbol_codegen(e->left->symbol));
			}
			else // TODO check to see if global variable editing even works
			{
				printf("global variable storage: %s\n",e->left->name);
				fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->left->reg), symbol_codegen(e->left->symbol));
				fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), symbol_codegen(e->left->symbol));
				fprintf(outfil, "\tstr\t%s, [%s]\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
			}
			e->reg = e->left->reg;
			break;
		case EXPR_ADD:				// 1
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);
			fprintf(outfil, "\tadd\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_SUB:				// 2
			expr_codegen(e->left, outfil);
			/*
			int leftreg_ptr = e->left->reg;
			if (e->right->kind == EXPR_FUNCCALL)
			{
				e->left->reg = 19;
			}
			*/
			expr_codegen(e->right, outfil);
			fprintf(outfil, "\tsub\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			/*
			if (e->right->kind == EXPR_FUNCCALL)
			{
				fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(leftreg_ptr), scratch_name(e->left->reg));
				e->left->reg = leftreg_ptr;
			}
			*/
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_MUL:				// 3
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);
			fprintf(outfil, "\tmul\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_DIV:				// 4
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);
			fprintf(outfil, "\tsdiv\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_MOD:				// 5
			expr_codegen(e->left, outfil);  // left side of modulo operator (a)
			expr_codegen(e->right, outfil); // right side of modulo operator (n)

			// perform modulus operation - a mod n = a - [n * int(a/n)]
			int div_reg = scratch_alloc();
			fprintf(outfil, "\tsdiv\t%s, %s, %s\n", scratch_name(div_reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			fprintf(outfil, "\tmul\t%s, %s, %s\n", scratch_name(e->right->reg), scratch_name(div_reg), scratch_name(e->right->reg));
			fprintf(outfil, "\tsub\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));
			// free left and right registers
			e->reg = e->left->reg;
			scratch_free(div_reg);
			scratch_free(e->right->reg);
			break;
		case EXPR_EXP:				// 6
			expr_codegen(e->left, outfil);  // base
			expr_codegen(e->right, outfil); // exponent

			// let's just hope these registers are in x0 and x1 lol
			int base_reg = 0;
			int expo_reg = 1;
			if (e->left->reg != base_reg)
			{
				fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(base_reg), scratch_name(e->left->reg));
			}
			if (e->right->reg != expo_reg)
			{
				fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(expo_reg), scratch_name(e->right->reg));
			}
			// branch to our integer power function
			fprintf(outfil, "\tbl\tinteger_power\n");

			e->reg = e->left->reg;
			scratch_free(e->left->reg);
			scratch_free(e->right->reg);
			break;
		case EXPR_LE:				// 7
		case EXPR_LT:				// 8
		case EXPR_GE:				// 9
		case EXPR_GT:				// 10
		case EXPR_EQ:				// 11
		case EXPR_NEQ:				// 12
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);

			// compare both sides of the expression
			fprintf(outfil, "\tcmp\t%s, %s\n", scratch_name(e->left->reg), scratch_name(e->right->reg));

			// conditional set a register based on one of the conditions
			switch (e->kind)
			{
				case EXPR_LE:				// 7
					fprintf(outfil, "\tcset\t%s, le\n", scratch_name(e->left->reg));
					break;
				case EXPR_LT:				// 8
					fprintf(outfil, "\tcset\t%s, lt\n", scratch_name(e->left->reg));
					break;
				case EXPR_GE:				// 9
					fprintf(outfil, "\tcset\t%s, ge\n", scratch_name(e->left->reg));
					break;
				case EXPR_GT:				// 10
					fprintf(outfil, "\tcset\t%s, gt\n", scratch_name(e->left->reg));
					break;
				case EXPR_EQ:				// 11
					fprintf(outfil, "\tcset\t%s, eq\n", scratch_name(e->left->reg));
					break;
				case EXPR_NEQ:				// 12 
					fprintf(outfil, "\tcset\t%s, ne\n", scratch_name(e->left->reg));
					break;
			}

			// free unnecessary register and assign expression register
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_AND:				// 13
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);

			// create an AND function
			fprintf(outfil, "\tand\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));

			// update expression register, maybe keep the unused register idk
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_OR:				// 14
			expr_codegen(e->left, outfil);
			expr_codegen(e->right, outfil);

			// create an AND function
			fprintf(outfil, "\torr\t%s, %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), scratch_name(e->right->reg));

			// update expression register, maybe keep the unused register idk
			e->reg = e->left->reg;
			scratch_free(e->right->reg);
			break;
		case EXPR_NOT:				// 15
			expr_codegen(e->right, outfil);

			// do an equals comparison to 0
			fprintf(outfil, "\tcmp\t%s, 0\n", scratch_name(e->right->reg));
			fprintf(outfil, "\tcset\t%s, eq\n", scratch_name(e->right->reg));

			// set new register of the expression
			e->reg = e->right->reg;
			break;
		case EXPR_NEG:				// 16
			expr_codegen(e->right, outfil);

			// use the negative instruction on the register
			fprintf(outfil, "\tneg\t%s, %s\n", scratch_name(e->right->reg), scratch_name(e->right->reg));

			// set new register of the expression
			e->reg = e->right->reg;
			break;
		case EXPR_INCR:				// 17
		case EXPR_DECR: 			// 18
			expr_codegen(e->left, outfil);

			int temp_reg = scratch_alloc();

			if (e->kind == EXPR_INCR)
				fprintf(outfil, "\tadd\t%s, %s, 1\n", scratch_name(temp_reg), scratch_name(e->left->reg));
			else if (e->kind == EXPR_DECR)
				fprintf(outfil, "\tsub\t%s, %s, 1\n", scratch_name(temp_reg), scratch_name(e->left->reg));
			else
			{
				printf("codegen error: how are you here in the incr/decr section.\n");
				exit(1);
			}
			e->reg = e->left->reg;

			// gotta also store this register into memory then free it
			// check whether we're saving an array element, local, or global variable
			if (e->left->kind == EXPR_ARRELEM)
			{
				int idx = e->left->right->literal_value;
				// load the array again cause I'm lazy
				fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->left->reg), e->left->left->name);
				fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), e->left->left->name);
				fprintf(outfil, "\tstr\t%s, [%s, %i]\n", scratch_name(temp_reg), scratch_name(e->left->reg), idx*8);
			}
			else if (e->left->symbol->kind == SYMBOL_LOCAL)
			{
				printf("not an array element\n");
				fprintf(outfil, "\tstr\t%s, [sp, %s]\n", scratch_name(temp_reg), symbol_codegen(e->left->symbol));
			}
			else
			{
				printf("global variable storage: %s\n",e->left->name);
				fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->left->reg), symbol_codegen(e->left->symbol));
				fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->left->reg), scratch_name(e->left->reg), symbol_codegen(e->left->symbol));
				
				fprintf(outfil, "\tstr\t%s, [%s]\n", scratch_name(temp_reg), scratch_name(e->left->reg));
				// immediately load the value into x0 in case we wanna use it
				fprintf(outfil, "\tldr\t%s, [%s]\n", scratch_name(e->left->reg), scratch_name(e->left->reg));
			}

			// free the registers afterward
			scratch_free(temp_reg);
			scratch_free(e->left->reg);
			
			break;
		case EXPR_GROUP:			// 19
			// expression grouping/precedence just doesn't work at all
			expr_codegen(e->right,outfil);
			e->reg = e->right->reg;
			break;
		case EXPR_ARRELEM:			// 20
			// TODO somehow implement the assignment of values to arrays which have not been declared yet
			// TODO also assignment of new values to already declared arrays
			printf("ARRAY ELEM INDEXING\n");
			if (e->left) printf("name: %s\n",e->left->name);
			if (e->right) printf("idx: %i\n",e->right->literal_value);
			e->reg = scratch_alloc();
			// instructions to deal with loading the array up
			fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->reg), e->left->name);
			fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->reg), scratch_name(e->reg), e->left->name);
			// now we gotta load up the exact element based on where it's located on the stack
			// expr_codegen(e->right, outfil);
			
			e->right->reg = scratch_alloc();
			int idx = e->right->literal_value;
			if (idx == 0)
				fprintf(outfil, "\tldr\t%s, [%s]\n",scratch_name(e->right->reg),scratch_name(e->reg));
			else
				fprintf(outfil, "\tldr\t%s, [%s, %i]\n",scratch_name(e->right->reg),scratch_name(e->reg),idx*8);
			
			// to be annoying and give probably the laziest and probably specific fix ever, just move the right reg into the arrays reg
			fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(e->reg), scratch_name(e->right->reg));

			// scratch_free(e->right->reg);
			scratch_free(e->right->reg);
			// e->reg = e->right->reg;
			break;
		// Leaf node: allocate register and load value
		case EXPR_INT_LITERAL:		// 21
		case EXPR_BOOLEAN_LITERAL: 	// 22
		case EXPR_CHAR_LITERAL:		// 23
			e->reg = scratch_alloc();
			fprintf(outfil, "\tmov\t%s, %d\n", scratch_name(e->reg), e->literal_value);
			break;
		case EXPR_STRING_LITERAL:	// 24
			e->reg = scratch_alloc();
			// string label stuff here
			int str_label = label_create();

			// start up the data section once more
			fprintf(outfil, "\t.data\n");

			fprintf(outfil, "\t.section\t.rodata\n");
			// allocate 2^3 = 8 bytes
			fprintf(outfil, "\t.align\t3\n");
			// string label
			fprintf(outfil, "%s:\n", label_name(str_label));
			// string value
			fprintf(outfil, "\t.string\t%s\n", e->string_literal);

			// go back to the text section
			fprintf(outfil, "\t.text\n");

			// now we need instructions to deal with this string
			fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->reg), label_name(str_label));
			fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->reg), scratch_name(e->reg), label_name(str_label));
			break;
		case EXPR_NAME:				// 25
			e->reg = scratch_alloc();
			if (e->symbol->kind != SYMBOL_GLOBAL)
			{
				printf("loading local variable/parameter %s\n",e->name);
				// code to reference any parameter or local variable for anything
				fprintf(outfil, "\tldr\t%s, [sp, %s]\n", scratch_name(e->reg), symbol_codegen(e->symbol));
				// TODO maybe remove this line cause it might not be necessary (i think this is only for global vars)
				// fprintf(outfil, "\tmov\t%s, %s\n", scratch_name(e->reg), symbol_codegen(e->symbol));
			}
			else
			{
				// code to reference a global variable within a function for anything
				printf("loading global variable %s\n",e->name);
				fprintf(outfil, "\tadrp\t%s, %s\n", scratch_name(e->reg), symbol_codegen(e->symbol));
				fprintf(outfil, "\tadd\t%s, %s, :lo12:%s\n", scratch_name(e->reg), scratch_name(e->reg), symbol_codegen(e->symbol));
				// TODO figure out if commenting this breaks things
				// TODO this breaks only when we gotta store something to the global variable itself aka any assignment
				// is there a way to find out if this name comes from an assignment?
				fprintf(outfil, "\tldr\t%s, [%s]\n", scratch_name(e->reg), scratch_name(e->reg));
			}
			break;
		case EXPR_FUNCCALL:			// 26
		{
			struct expr *er = e->right; // parameters
			struct expr *er_copy = e->right; // parameters copy
			struct expr *el = e->left;  // function identifier
			
			// int i = 0;

			// printf("funccall start\n");

			// TODO somehow move other IR values into other registers before calling this
			// let's try to at least
			/*
			int reg_move = 19;
			for (int i=0;i<6;i++)
			{
				if (reg_table[i])
				{

				}
			}
			*/

			// generate code for loading in those parameters
			while (er)
			{
				// TODO fix this up, we gotta load the parameters into argument registers then branch to the function
				// If i hardcode the parameters, have this loop a "mov" command 

				// generate code for parameter
				expr_codegen(er, outfil);

				// free register if possible
				// scratch_free(er->reg);

				// move to next pointer
				er = er->next;
			}

			// scratch_print();

			// printf("now we free\n");

			// free up the registers we just used
			while (er_copy)
			{
				scratch_free(er_copy->reg);

				er_copy = er_copy->next;
			}

			// scratch_print();

			// printf("argument end\n");

			// branch to the function
			fprintf(outfil, "\tbl\t%s\n",el->name);

			/*
			// TODO needs local variable storage for any return parameters, probably also needs a lot of fixes
			if (expr_typecheck(el) != TYPE_VOID)
			{
				printf("returning for function %s\n", el->name);

				e->reg = scratch_alloc();
				fprintf(outfil, "\tstr\t%s, [sp, %i]\n", scratch_name(e->reg), STACK_SIZE-(el->symbol->which*8));
			}
			*/

			// printf("done with return type\n");

			/*
			int funccall_result = scratch_alloc();
			e->reg = funccall_result;
			*/

			break;
		}
	}

	/*
	// I'm just gonna try something
	if (e->right)
	{
		if (e->right->kind == EXPR_FUNCCALL)
		{
			// save registers

			// new register time
			e->reg = 19;

		}
	}
	*/

}

/* print an expression */
void expr_print(struct expr *e)
{
	// return if the expression just does not exist
	if (!e) return;

	// print based on the kind of expression
	switch (e->kind)
	{
		case EXPR_ASSIGN:
			expr_print(e->left);
			printf("=");
			expr_print(e->right);
			break;
		case EXPR_ADD:
			// printf("ADDLEFT KIND:%d ",e->left->kind);
			expr_print(e->left);
			printf("+");
			// printf("ADDRIGHT KIND:%d ",e->right->kind);
			expr_print(e->right);
			break;
		case EXPR_SUB:
			// printf("SUBLEFT KIND:%d ",e->left->kind);
			expr_print(e->left);
			printf("-");
			// printf("SUBRIGHT KIND:%d ",e->right->kind);
			expr_print(e->right);
			break;
		case EXPR_MUL:
			expr_print(e->left);
			printf("*");
			expr_print(e->right);
			break;
		case EXPR_DIV:
			expr_print(e->left);
			printf("/");
			expr_print(e->right);
			break;
		case EXPR_MOD:
			expr_print(e->left);
			printf("%%");
			expr_print(e->right);
			break;
		case EXPR_EXP:
			expr_print(e->left);
			printf("^");
			expr_print(e->right);
			break;
		case EXPR_LE:
			expr_print(e->left);
			printf("<=");
			expr_print(e->right);
			break;
		case EXPR_LT:
			expr_print(e->left);
			printf("<");
			expr_print(e->right);
			break;
		case EXPR_GE:
			expr_print(e->left);
			printf(">=");
			expr_print(e->right);
			break;
		case EXPR_GT:
			expr_print(e->left);
			printf(">");
			expr_print(e->right);
			break;
		case EXPR_EQ:
			expr_print(e->left);
			printf("==");
			expr_print(e->right);
			break;
		case EXPR_NEQ:
			expr_print(e->left);
			printf("!=");
			expr_print(e->right);
			break;
		case EXPR_AND:
			expr_print(e->left);
			printf("&&");
			expr_print(e->right);
			break;
		case EXPR_OR:
			expr_print(e->left);
			printf("||");
			expr_print(e->right);
			break;
		case EXPR_NOT:
			printf("!");
			expr_print(e->right);
			break;
		case EXPR_NEG:
			printf("-");
			expr_print(e->right);
			break;
		case EXPR_INCR:
			expr_print(e->left);
			printf("++");
			break;
		case EXPR_DECR:
			expr_print(e->left);
			printf("--");
			break;
		case EXPR_GROUP:
			printf("(");
			expr_print(e->right);
			printf(")");
			break;
		case EXPR_ARRELEM:
			expr_print(e->left);  // expr_create_name(ident) --> expr_create(EXPR_NAME,0,0) --> name 
			printf("[");
			expr_print(e->right); // bracket --> [expr] --> [i]      bracket --> expr->right = brackets --> bracket --> expr
			printf("]");

			e = e->right;

			if (e->next)
			{
				while (e->next)
				{
					printf("[");
					expr_print(e->next);
					printf("]");
					e = e->next;
				}
			}

			break;
		case EXPR_INT_LITERAL:
			printf("%d",e->literal_value);
			break;
		case EXPR_BOOLEAN_LITERAL:
			if (e->literal_value) printf("true");
			else                  printf("false");
			break;
		case EXPR_CHAR_LITERAL:
			printf("'%c'",e->literal_value);
			break;
		case EXPR_STRING_LITERAL:
			printf("%s",  e->string_literal);
			break;
		case EXPR_NAME:
			printf("%s",  e->name);
			break;
		case EXPR_FUNCCALL:
			expr_print(e->left);
			printf("(");
			exprs_print(e->right);
			printf(")");
			break;
	}

	return;
}

/* print multiple expressions, for print, funccalls, arrayelems */
void exprs_print(struct expr *e)
{
	/* print nothing if it doesn't exist */
	if (!e) return;

	expr_print(e);

	if (e->next)
	{
		printf(", ");
	}

	exprs_print(e->next);
}