#include "stmt.h"
#include "scope.h"
#include "scratch.h"
#include "label.h"
#include "library.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int type_val;
extern int isvoid;
extern int yylineno;

extern int func_label;

/* create a statement */
/*
inputs
- decl: decl struct
- init_expr: left expr
- expr: expr struct
- next_expr: right expr
- body: stmt struct
- else_body: else stmt
- next: next statement
output
- s: stmt struct
*/
struct stmt * stmt_create(
	stmt_t kind,
	struct decl *decl, struct expr *init_expr, 
	struct expr *expr, struct expr *next_expr, 
	struct stmt *body, struct stmt *else_body,
	struct stmt *next )
{
	struct stmt *s = calloc(1, sizeof(*s));
	s->kind      = kind;
	s->decl      = decl;
	s->init_expr = init_expr;
	s->expr      = expr;
	s->next_expr = next_expr;
	s->body      = body;
	s->else_body = else_body;
	s->next      = next;

	return s;
}

/* stmt_resolve - resolve a statement's expressions by scope lookup */
/*
inputs
- s: statement struct
*/
void stmt_resolve(struct stmt *s)
{
	// resolve nothing if a statement doesn't exist in the first place
	if (!s) return;

	// printf("new statement: ");
	// stmt_print(s, 0);

	switch (s->kind) 						// look at the kind of statement being dealt with here
	{
		case STMT_DECL:
			decl_resolve(s->decl); 			// resolve the declaration within our statement
			break;
		case STMT_EXPR:
			expr_resolve(s->expr); 			// resolve the expression within our statement
			break;
		case STMT_IF_ELSE:
			scope_enter();                  // create a new hash table scope for the if/else statement

			expr_resolve(s->expr); 			// resolve the expression used in the if statement
			stmt_resolve(s->body); 			// resolve the body of the if statement (block)
			if (s->else_body)  				// resolve the body of the else statement IF it exists (block) ... MIGHT NOT ACTUALLY NEED THIS IF STATEMENT HERE
				stmt_resolve(s->else_body);

			scope_exit(); 					// remove this hash table scope
			break;
		case STMT_FOR:
			scope_enter(); 					// create a new hash table scope for the for statement

			expr_resolve(s->init_expr); 	// resolve the first for loop expr
			expr_resolve(s->expr); 			// resolve the second for loop expr
			expr_resolve(s->next_expr);  	// resolve the third for loop expr
			stmt_resolve(s->body); 			// resolve the body of the for loop statemenet (block)

			scope_exit();  					// remove this hash table scope
			break;
		case STMT_PRINT:
			exprs_resolve(s->expr); 		// resolve the print statement expressions
			/*
			while (s)
			{
				expr_resolve(s->expr);
				s = s->next;
			}
			*/
			break;
		case STMT_RETURN:
			expr_resolve(s->expr);			// resolve the return expression
			break;
		case STMT_BLOCK:
			scope_enter(); 					// create a new hash table scope for the block

			stmt_resolve(s->body); 			// let's resolve all the statements found in this block

			scope_exit();					// remove this hash table scope
			break;
	}
	// find the next linked statement and resolve it
	stmt_resolve(s->next); 					
}

/* STMT TYPECHECK */
/*
  - stmts must be typechecked by evaluating each of their components and then verifying that types match where needed
  - after the type is examined, it is no longer needed and may be deleted
  - for example, if-else statements require that the control expression have boolean type
*/
/*
inputs
- s: stmt struct
*/
void stmt_typecheck(struct stmt *s)
{
	if (!s) return;

	struct type *t;

	// create extra type pointers for extra types within the statement
	struct type *ta;
	struct type *tb;
	struct type *tc;

	// typecheck depnding on the kind of statement
	switch (s->kind)
	{
		case STMT_DECL:
			decl_typecheck(s->decl);
		case STMT_EXPR:
			t = expr_typecheck(s->expr);
			type_delete(t);
			break;
		case STMT_IF_ELSE:
			t = expr_typecheck(s->expr);
			if (t->kind != TYPE_BOOLEAN)
			{
				/* display an error */
				printf("type error: if statement condition has to be of type boolean\n");
				type_val++;
			}
			type_delete(t);
			stmt_typecheck(s->body);
			stmt_typecheck(s->else_body);
			break;
		case STMT_FOR:
			ta = expr_typecheck(s->init_expr);
			if (s->expr)
			{
				tb = expr_typecheck(s->expr);
				if (tb->kind != TYPE_BOOLEAN)
				{
					printf("type error: second expression in for loop has to be of type boolean\n");
					type_val++;
				}
			}
			tc = expr_typecheck(s->next_expr);
			stmt_typecheck(s->body);
			if (ta) type_delete(ta);
			// if (tb) type_delete(tb);
			if (tc) type_delete(tc);
			break;
		case STMT_PRINT:
			t = expr_typecheck(s->expr);
			if (t->kind == TYPE_FUNCTION ||
				t->kind == TYPE_ARRAY    ||
				t->kind == TYPE_VOID )
			{
				printf("type error: cannot print ");
				expr_print(s->expr);
				printf("\n");
				type_val++;
			}
			type_delete(t);
			if (s->expr->next)
			{
				t = expr_typecheck(s->expr->next);
			}
		case STMT_RETURN:
			// if the expression being returned isn't a void type, proceed as normal
			if (s->expr->kind != TYPE_VOID)
			{
				// printf("typecheck expr associated\n");
				// expr_print(s->expr);
				// printf("\n");
				// type_print(expr_typecheck(s->expr));
				// printf("\n");

				t = expr_typecheck(s->expr);
				type_delete(t);
			}
			else // otherwise, do some error handling
			{
				printf("type error: cannot return expression of type void\n");
				type_val++;
			}
		case STMT_BLOCK:
			stmt_typecheck(s->body);
			break;
	}
	// typecheck next statement
	stmt_typecheck(s->next);
}

/* stmt code generation */
void stmt_codegen(struct stmt *s, FILE *outfil)
{
	// if no statement exists then leave
	if (!s) return;

	// generate code based on type of statement
	switch (s->kind)
	{
		case STMT_DECL:    	// 0
			decl_codegen(s->decl, outfil);
			break;
		case STMT_EXPR:	  	// 1
			expr_codegen(s->expr, outfil);
			scratch_free(s->expr->reg);
			break;
		case STMT_IF_ELSE: 	// 2
			// TODO somehow handle multiple returns
			if (s->else_body)
			{
				// first of all, how about we create some labels
				int lbl_else = label_create();
				int lbl_done = label_create();
				// the expr here will contain the expression(s) involved for the if statement
				expr_codegen(s->expr, outfil);
				
				// gotta check if the negative of the if statement is true aka check if the statement is FALSE
				fprintf(outfil, "\tcmp\t%s, 0\n", scratch_name(s->expr->reg));
				// free up that register right after
				scratch_free(s->expr->reg);

				// the if expr is false if it equals 0, therefore we need 0 = 0, therefore branch to the else statement if EQUAL
				fprintf(outfil, "\tbeq\t%s\n", label_name(lbl_else));
				
				// generate the code for when the if expr is true
				stmt_codegen(s->body, outfil);

				// unconditionally branch to the done label when finished
				fprintf(outfil, "\tb\t%s\n", label_name(lbl_done));

				// time to deal with our else expression
				fprintf(outfil, "%s:\n", label_name(lbl_else));
				// generate the code for the else expression
				stmt_codegen(s->else_body, outfil);
				// print our done label and let the rest of our statements follow
				fprintf(outfil, "%s:\n", label_name(lbl_done));
			}
			else // no else expression, just print what we got for the if statement
			{
				int lbl_done = label_create();

				expr_codegen(s->expr, outfil);

				// gotta check if the negative of the if statement s true aka check if the statement is fALSE
				fprintf(outfil, "\tcmp\t%s, 0\n", scratch_name(s->expr->reg));
				// free up that register right after
				scratch_free(s->expr->reg);

				// the if expr is false if it equals 0, therefore we need 0 = 0, therefore branch to the else statement if EQUAL
				// branch if false, don't branch if true
				fprintf(outfil, "\tbeq\t%s\n", label_name(lbl_done));
				
				// generate the code for when the if expr is true
				stmt_codegen(s->body, outfil);

				// unconditionally branch to the done label when finished
				fprintf(outfil, "\tb\t%s\n", label_name(lbl_done));

				// print our done label and let the rest of our statements follow
				fprintf(outfil, "%s:\n", label_name(lbl_done));
			}
			break;
		case STMT_FOR:     	// 3
			;;
			int lbl_top = label_create();
			int lbl_don = label_create();

			// evaluate for init, standard, and next exprs
			// initial for expression
			if (s->init_expr)
			{
				expr_codegen(s->init_expr, outfil);
				scratch_free(s->init_expr->reg);
			}
			fprintf(outfil, "%s:\n", label_name(lbl_top));
			// middle for expression
			if (s->expr)
			{
				expr_codegen(s->expr, outfil);
				// check to see if we can leave now
				fprintf(outfil, "\tcmp\t%s, 0\n", scratch_name(s->expr->reg));
				// free up that register right after
				scratch_free(s->expr->reg);

				fprintf(outfil, "\tbeq\t%s\n", label_name(lbl_don));
			}
			// generate statements for the body of the loop
			stmt_codegen(s->body, outfil);
			// ending for expression
			if (s->next_expr)
			{
				expr_codegen(s->next_expr, outfil);
			}
			// it's time to leave
			fprintf(outfil, "\tb\t%s\n", label_name(lbl_top));
			fprintf(outfil, "%s:\n", label_name(lbl_don));
			break;
		case STMT_PRINT:   	// 4
			// struct expr *s_expr = s->expr;
			while (s->expr)
			{
				// fprintf(outfil, "");

				expr_codegen(s->expr, outfil);
				// fprintf(outfil, "\tMOV %s, ")

				// TODO fix array element printing, might be printing the address of the array instead of the element?

				// TODO stress test printing and see if it's gonna have to forcibly send registers other than x0 to print, which would be bad

				struct type *e_type = expr_typecheck(s->expr);
				switch (e_type->kind)
				{
					case TYPE_INTEGER:
						fprintf(outfil, "\tbl\tprint_integer\n");
						break;
					case TYPE_BOOLEAN:
						fprintf(outfil, "\tbl\tprint_boolean\n");
						break;
					case TYPE_STRING:
						fprintf(outfil, "\tbl\tprint_string\n");
						break;
					case TYPE_CHARACTER:
						fprintf(outfil, "\tbl\tprint_character\n");
						break;
				}

				scratch_free(s->expr->reg);
				if (s->expr->next) s->expr = s->expr->next;
				else               break;
			}
			break;
		case STMT_RETURN:  	// 5
			expr_codegen(s->expr, outfil);
			// print some arm assembly
			// fprintf(outfil,"\tmov %s, %s")
			scratch_free(s->expr->reg);

			// branch to our function epilogue
			fprintf(outfil,"\tbl\t%s\n",label_name(func_label));
		case STMT_BLOCK:    // 6
			stmt_codegen(s->body, outfil);
			break;
	}

	stmt_codegen(s->next, outfil);
}

/* print a statement */
/*
inputs
- s: stmt struct
- indent: amount of spaces/tabs to indent before placing the stmt
*/
void stmt_print(struct stmt *s, int indent)
{
	// return nothing if statement does not exist
	if (!s) return;

	// printf("stmt kind: %i\n", s->kind);

	// print based on the kind of statement
	switch (s->kind)
	{
		case STMT_DECL:
			stmt_print_tabs(indent); 	// indent before placing decl

			decl_print(s->decl,indent); // place decl
			break;
		case STMT_EXPR:
			stmt_print_tabs(indent);	// indent before placing expr

			expr_print(s->expr); 		// place expr
			printf(";\n");
			break;
		case STMT_IF_ELSE:
			stmt_print_tabs(indent); 	// indent before placing if else

			printf("if("); 				

			expr_print(s->expr);		// print if stmt
			printf(") ");

			if (s->body->kind != STMT_BLOCK) // if there does not exist a block for the body, generate one
			{
				printf("{\n");

				stmt_print(s->body, indent+1);
				stmt_print_tabs(indent);
				printf("}");
			}
			else // there is a block for the body, print it
			{
				stmt_print(s->body, indent);
			}

			// if we're looking at an else statement
			if (s->else_body)
			{
				if (s->else_body->kind != STMT_BLOCK) // there does not exist a block for the else body, generate one
				{
					printf(" else ");
					printf("{\n");

					stmt_print(s->else_body, indent+1);
					stmt_print_tabs(indent);
					printf("}\n");
				}
				else // there is a block for the else body, print it
				{
					printf(" else ");

					stmt_print(s->else_body,indent);
					stmt_print_tabs(indent);
					printf("\n");
				}
			}
			else // no else statement, get to a new line
			{
				printf("\n");
			}
			break;
		case STMT_FOR:
			stmt_print_tabs(indent); 		// indent before placing for expr

			printf("for(");

			expr_print(s->init_expr); 		// first for expr
			printf(";");

			expr_print(s->expr); 			// second for expr
			printf(";");

			expr_print(s->next_expr); 		// third for expr
			printf(") ");

			if (s->body->kind != STMT_BLOCK) // there does not exist a block for the for body, generate one
			{
				printf("\n");

				stmt_print_tabs(indent);
				printf("{\n");

				stmt_print(s->body,indent+1);
				stmt_print_tabs(indent);
				printf("}\n");
			}
			else // there exists a block for the for body, print it
			{
				stmt_print(s->body,indent);
				printf("\n");
			}
			break;
		case STMT_PRINT:
			stmt_print_tabs(indent); 		// indent before placing print stmt
			printf("print ");

			exprs_print(s->expr); 			// print stmt
			printf(";\n");
			break;
		case STMT_RETURN:
			stmt_print_tabs(indent); 		// indent before placing return stmt
			printf("return ");

			expr_print(s->expr);			// print return stmt
			printf(";\n");
			break;
		case STMT_BLOCK:
			expr_print(s->expr); 			// print the expr before the block

			printf("{\n");

			stmt_print(s->body,indent+1); 	// print the content of the block
			stmt_print_tabs(indent); 		// indent before placing the last brackets
			printf("}");
			break;
	}
	// print next statement
	stmt_print(s->next, indent);
}

/* print space before placing stmt, expr, or decl */
void stmt_print_tabs(int indent)
{
	for (int i=0;i<indent;i++) printf("    ");
}