#include "token.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

#include "decl.h"
#include "stmt.h"
#include "expr.h"
#include "type.h"
#include "param_list.h"
#include "scope.h"

extern FILE *yyin;
extern int yylex();
extern char *yytext;
extern int yyparse();
extern int yylineno;
extern struct decl* parser_result;

int type_val    = 0;
int resolve_val = 0;

/* Function that converts token number into string */
/*
inputs:
- t: token number
outputs:
- string pertaining to the conversion of the token into a string literal
*/
char* token2text(int t)
{
    switch (t)
    {
        case TOKEN_EOF:             return "EOF";
        case TOKEN_IDENT:           return "IDENTIFIER";
        case TOKEN_NUMBER:          return "INTEGER_LITERAL";
        case TOKEN_STRING_LITERAL:  return "STRING_LITERAL";
        case TOKEN_CHAR_LITERAL:    return "CHARACTER_LITERAL";
        case TOKEN_ERROR:           return "ERROR";
        case TOKEN_ARRAY:           return "ARRAY";
        case TOKEN_BOOLEAN:         return "BOOLEAN";
        case TOKEN_CHAR:            return "CHAR";
        case TOKEN_IF:              return "IF";
        case TOKEN_ELSE:            return "ELSE";
        case TOKEN_TRUE:            return "TRUE";
        case TOKEN_FALSE:           return "FALSE";
        case TOKEN_FOR:             return "FOR";
        case TOKEN_FUNCTION:        return "FUNCTION";
        case TOKEN_INTEGER:         return "INTEGER";
        case TOKEN_PRINT:           return "PRINT";
        case TOKEN_RETURN:          return "RETURN";
        case TOKEN_STRING:          return "STRING";
        case TOKEN_VOID:            return "VOID";
        case TOKEN_AUTO:            return "AUTO";
        case TOKEN_LEFTPARAND:      return "LEFTPARAND";
        case TOKEN_RIGHTPARAND:     return "RIGHTPARAND";
        case TOKEN_LEFTSQUARE:      return "LEFTSQUARE";
        case TOKEN_RIGHTSQUARE:     return "RIGHTSQUARE";
        case TOKEN_LEFTCURLY:       return "LEFTCURLY";
        case TOKEN_RIGHTCURLY:      return "RIGHTCURLY";
        case TOKEN_INCREMENT:       return "INCREMENT";
        case TOKEN_DECREMENT:       return "DECREMENT";
        case TOKEN_ADD:             return "PLUS";
        case TOKEN_SUBTRACT:        return "MINUS";
        case TOKEN_NOT:             return "NOT";
        case TOKEN_EXPONENT:        return "EXPONENT";
        case TOKEN_MULTIPLY:        return "MULTIPLY";
        case TOKEN_DIVIDE:          return "DIVIDE";
        case TOKEN_MODULUS:         return "MODULUS";
        case TOKEN_GT:              return "GT";
        case TOKEN_GE:              return "GE";
        case TOKEN_LT:              return "LT";
        case TOKEN_LE:              return "LE";
        case TOKEN_AND:             return "AND";
        case TOKEN_OR:              return "OR";
        case TOKEN_ASSIGNMENT:      return "ASSIGNMENT";
        case TOKEN_EQ:              return "EQ";
        case TOKEN_NEQ:             return "NEQ";
        case TOKEN_SEMICOLON:       return "SEMICOLON";
        case TOKEN_COLON:           return "COLON";
        case TOKEN_COMMA:           return "COMMA";
        default:                    return 0;
    }
}

/* Scanner method for token identification */
/* 
inputs:
- fil: main bminor file which will be scanned
outputs: 
- N/A
*/
void scan(FILE *fil)
{
    printf("Scanning...\n");

    // printf("scan path: %s\n",scan_path);
    // \'(\\.|[^\\])\'
    char buffer[256]; // holds the contents of our stripped/modified string or character literal
    int buf_idx = 0;  // holds the latest index of the string/character literal we're assigning characters to
    int null_cnt = 0; // carries the amount of null characters \0 in a string or character literal

    // Loop through all possible tokens
    while(1) {
        int t = yylex();
        // token_t t = yylex();
        // End of file token
        if (!t || t == TOKEN_EOF) break;
        // Error token
        if (t == TOKEN_ERROR) 
        {
            fprintf(stderr, "scan error: %s is an invalid token\n", yytext);
            exit(1);   
        }

        // Identifier or integer literal token
        if (t == TOKEN_IDENT || t == TOKEN_NUMBER)
        {
            // Print token name along with value
            printf("%s %s\n",token2text(t),yytext);
        }
        else if (t == TOKEN_STRING_LITERAL || t == TOKEN_CHAR_LITERAL)
        {
            // String literal or character literal

            // Reset function variables
            memset(buffer, 0, 256);
            buf_idx = 0;
            null_cnt = 0;

            // Get rid of the quotations
            for (int i=0;i<strlen(yytext);i++)
            {
                // Handle valid escape characters
                if ((yytext[i+1] == '\\') && (i+2 < strlen(yytext)))
                {
                    // Go through and replace each valid escape character with a corresponding actual escape character
                    switch (yytext[i+2])
                    {
                        case 'n': { buffer[buf_idx] = '\n';             ++i; break; } // newline escape character
                        case '0': { buffer[buf_idx] = '\0'; ++null_cnt; ++i; break; } // null escape character, increase null count
                        default:  { buffer[buf_idx] = yytext[i+2];      ++i; break; } // any other character that follows a '\\'
                    }
                }
                else
                {
                    // otherwise add normal character to char/string literal
                    buffer[buf_idx] = yytext[i+1];
                }
                // increase buffer index to next empty character
                buf_idx++;
            }
            // Assign null character to the end
            buffer[buf_idx-2] = '\0'; ++null_cnt;

            // Everything checks out, print token name and value
            printf("%s %s\n",token2text(t),buffer);
        }
        else
        {
            // Otherwise print identified token
            printf("%s\n",token2text(t));   
        }
    }
    // We've exited the while loop, scan success
    printf("Scan successful.\n");
}

/* Parser method for grammar evaluation */
/*
inputs:
- fil: main BMinor file which will be scanned and then parsed
outputs:
- N/A
*/
void parse(FILE *fil)
{
    printf("Parsing...\n");

    // Parse grammar
    int parse_val;
    parse_val = yyparse();
        
    // Parse success
    if (parse_val == 0)
    {
        printf("parse successful\n");
    }

    return;
}

/* Pretty printer method for pretty printing an AST */
/*
inputs:
- fil: main BMinor file which will be scanned and then parsed and then printed
outputs:
- N/A
*/
void prettyprint(FILE *fil)
{
    printf("Printing...\n");

    // Parse grammar
    int parse_val;
    parse_val = yyparse();

    // Parse success
    if (parse_val == 0)
    {
        decl_print(parser_result, 0);
    }

    return;
}

/* Resolve method for type checking */
/*
inputs:
- fil: main BMinor file which will be scanned and then parsed
outputs:
- N/A
*/
void resolve(FILE *fil)
{
    printf("Resolving...\n");

    int parse_val;
    parse_val = yyparse();

    if (parse_val == 0)
    {
        // create a new hash table and push to the top of the stack, basically create a new scope
        scope_enter();
        // call a resolve decl on the root node of the AST
        decl_resolve(parser_result);
        // if we have a resolve error(s), print how many we got
        if (resolve_val >= 1)
        {
            fprintf(stderr, "resolve error: %i resolve error(s)\n", resolve_val);
            exit(1);
        }
    }

    return;
}

/* Type checking method for type checking :) */
/*
inputs:
- fil: main BMinor file which will be scanned and then parsed
outputs:
- N/A
*/
void typecheck(FILE *fil)
{
    printf("Type checking...\n");

    parse(fil);

    // create a new hash table and push to the top of the stack, basically create a new scope
    scope_enter();
    // call a resolve decl on the root node of the AST
    decl_resolve(parser_result);
    // if we have a resolve error(s), print how many we got
    if (resolve_val >= 1)
    {
        fprintf(stderr, "resolve error: %i resolve error(s)\n", resolve_val);
        exit(1);
    }

    // do typechecking if we got no resolve errors
    decl_typecheck(parser_result);
    if (type_val) // if there exist type errors accumulated, print them
    {
        fprintf(stderr, "type error: %i type error(s)\n", type_val);
        exit(1);
    }
    else // otherwise the typecheck was successful
    {
        printf("typecheck successful.\n");
    }

    return;
}


/*
CODE GENERATION
WEEK 1
- generate code for simple variable delcarations (e.g. x:integer = 5;)
- generate code for returning a single constant value from main (e.g. func1: function integer() = { return 1; })
- work up bit by bit to handle integer arithmetic
WEEK 2
- implement control statements
- implement function calls
- implement literally everything else what is this work split
- student test cases

bminor -codegen src.bminor src.s

WHAT TO DO
- if we pass scanning,parsing,resolving,typechecking, print valid assembly program to src.s, exit 0
- runtime library library.c needed to operate correctly.
  - note that print statements should result in one or more calls to the C functions print_string, print_integer, etc., depending on the type of expression
  - may add additional runtime calls as we see fit (oh we definitely are)
- simplifying assumptions
  - due to the large num of AArch-64 registers , we can use a simple non-spilling reg allocator, fail with an "out of registers" error on REALLY complicated nested exprs
  - to keep conventions simple, calls to funcs with more than 6 args may fail with a too many arguments error
  - only 1D arrays of integers at global scope must be implemented. Arrays of other types or arrays as params or local vars may fail
    - ALLOWED ARRAYS: global 1D integer arrays
    - NOT ALLOWED ARRAYS: arrays of any other type, local arrays, global arrays, 2D+ arrays, any combination of those

THE FUNCTIONS - codegen and scratch
- decl_codegen : generate code for a delcaration
- and you know the drill
- supporting functions
  - int scratch_alloc();
  - void scratch_free( int r );
  - const char * scratch_name( int r );

decl_codegen --> {stmt_codegen, scratch_free}
stmt_codegen --> {expr_codegen, label_create, label_print, scratch_free}
expr_codegen --> {symbol_codegen, scratch_alloc, scratch_free}

- take scratch values and place them into a table of which register they are, their name, and whether they're in use
- scratch_alloc : finds an unused register in the table, marks it as in use, and returns the register number "r"
  - if it cannot find a free register, just emit an error message and halt
- scratch_free  : marks an indicated register as available, frees it up in the table
- scratch_name  : returns the name of a register given its number "r"
- running out of scratch registers is possible but unlikely (these would be our local var regs x9-x15)

MORE FUNCTIONS - label
- these will generate a large num of unique anonymous labels that indicate:
 - targets of jumps
 - targets of conditional branches
  - int label_create();
  - const char * label_name( int label );

- label_create : increments a global counter and returns the current value
- label_name   : returns that label in a string form, ex. label 15 = ".L15"

- mapping from the symbols in a program to the assembly lang code representing those symbols. Gotta generate symbol addresses
  - const char * symbol_codegen( struct symbol *s );

- symbol_codegen : returns a string which is a fragment of an instruction, representing the address computation needed for a given symbol
  - examines the scope of the symbol
  - global : name in assembly language is the same as in the source language ex. count: integer --> returns count
  - local/param : return an address computation that yields the pos of that local var/param on the stack (based on that "which" parameter of symbol struct)
    - returns a string describing the precise stack address of local vars and params, knowing only its position in the stack frame

EXPR GENERATION
- perform post-order traversal of the AST (left right root), emit one or more instructions per node
- keep track of the registers in which an IR is stored
---ADD A REG FIELD TO THE AST NODE STRUCTURE
  - will hold the number of a register returned by scratch_alloc
- visit each node and emit an instruction and place into the reg field the number of the register containing that value
- when the node is no longer needed, use scratch_free to release it

implementation
- recursively calls itself for its left and right children
- each child will generate code such that the result will be left in the reg num noted in the reg field
- current node generates code using those registers
- current node frees the registers it no longer needs
extra stuff
- not all symbols are simple global vars
- when a symbol forms part of an instruction, symbol_codegen needs to return the string that gives the specific address for that symbol

- some nodes may require multiple instructions, in order to handle peculiarities

- how do we invoke a function?
  - idk

STMT GENERATION
- builds on larger code structures that rely on expressions
- creates code for all control flow statements
- every time a expr_codegen is called a scratch register should be freed
- return statement must eval an expr, move it into designated register, jump to func epilogue

- But wait, what do we do for the PRINT statement, the most important one everrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
  - i: integer = 10;
  - b: boolean = true;
  - s: string = "                                                              ffffffffffffffffffffffffffffffffffffffffff :)\n";
  - print i, b, s; = print_integer(i); print_boolean(b); print_string(s);
  - generate the code for each expression to be printed
  - determine the type of expression with "expr_typecheck"
  - emit the corresponding function call

- not gonna do control statements yet

DECL GENERATION
- traverse each declaration of code or data and emit its basic structure
- 3 types: global var decls, local var decls, global func decls
- Global data decls
  - emit a label along with a suitable directive that reserves necessary space
  - emit an initializer if needed
- examples
  - i: integer = 10;
  - s: string = "hello";
  - b: array [4] boolean = { true, false, true, false };
    - output directives
      .data
      i: .quad 10
      s: .string "hello"
      b: .quad 1, 0, 1, 0
  - can only be initialized by a constant value and not general expression
  - IMPORTANT: if programmer accidentally put code into initializer, typechecker should discover this and raise an error begore code gen begins

-  Local variable decls
  - ONLY WHEN decl_codegen is called by a stmt_codegen inside a func decl
  - assume space for the local var is already established by func prologue
    - we don't want to have to manipulate the stack, shouldn't be necessary 
  - IF local var has initializing expression, (e.g. x:integer = y * 10;), gen code for the expression, store in local var, then free the reg

- Global func decls
  - emit label with function's name
  - follow with func prologue
    - must take into account the num of params and local vars
    - make appropriate amount of space on the stack
  - body of the function
  - function epilogue
    - should have unique label so that return statements can easily jump there

HINTS
- write a bunch of very simple programs that do very simple stuff
- write just enough of the codegen to cover those cases
- ex. write a test that declares one var (and nothing else)
- ex. write a test that returns a single constant value from main
- do some integer arithmetic tests too lol 
*/

/*
ISSUES

label_name     : label str probably isn't right
symbol_codegen : symbol str probably isn't right
symbols: param and local "which" vals need to be grouped together rather than separate 
decl_codegen   : function parameters probably need to be created by symbol_codegen?
decl_codegen   : do functions actually need a .text to start?
*/

/* Code generation of BMinor into ASSEMBLY */
/*
inputs:
- fil: main BMinor file which will be scanned and then parsed
- outfil: 
outputs:
- N/A
*/
void codegen(FILE *fil, FILE *outfil)
{
    printf("Code generating...\n");

    /*
    int parse_val;
    parse_val == yyparse();

    // parse
    parse(fil);

    // resolve
    scope_enter();
    decl_resolve(parser_result);

    if (resolve_val >= 1)
    {
        fprintf(stderr, "resolve error: %i resolve error(s)\n", resolve_val);
        exit(1);
    }

    // typecheck
    decl_typecheck(parser_result);
    if (type_val >= 1)
    {
        fprintf(stderr, "type error: %i type error(s)\n", type_val);
        exit(1);
    }
    */
    typecheck(fil);

    // codegen
    // boiler plate prologue
    fprintf(outfil, "\t.arch armv8-a\n");
    fprintf(outfil, "\t.text\n");

    // codegen the actual bminor code
    decl_codegen(parser_result, outfil);

    // boiler plate epilogue
    fprintf(outfil, "\t.text\n"); // unsure if this is okay to have?
    fprintf(outfil, "\t.ident\t\"GCC: (Debian 8.3.0-6) 8.3.0\"\n"); // kernel type and version
    fprintf(outfil, "\t.section\t.note.GNU-stack,\"\",@progbits\n");

    // try to close and output the file
    int out_ret = fclose(outfil);
    if (out_ret)
    {
        fprintf(stderr, "file error: file not outputted\n");
        exit(1);
    }

    return;
}

/* Main function for BMinor compiler */
/*
inputs: 
- argc: the number of arguments
- argv: the arguments themselves
outputs:
- integer 0
*/
int main(int argc, char *argv[])
{
    int opt;
    // Specifying expected options for the main script
    while (1)
    {
        // define options
        static struct option long_options[] = {
            {"scan",      required_argument, 0,  's' },
            {"parse",     required_argument, 0,  'p' },
            {"print",     required_argument, 0,  't' },
            {"resolve",   required_argument, 0,  'r' },
            {"typecheck", required_argument, 0,  'y' },
            {"codegen",   required_argument, 0,  'c' },
            {0,                           0, 0,   0  }
        };
        int long_index = 0;

        // get arguments from command line, see if they match our options
        opt = getopt_long_only(argc, argv, "s:p:t:r:y:c:", long_options, &long_index);
        if (opt == -1)
            break;

        // Open bminor file
        yyin = fopen(optarg,"r");
        // Handle file's existence
        if(!yyin) {
            printf("could not open %s\n",optarg);
            exit(1);
        }
        
        // call different functions depending on option chosen
        switch (opt) 
        {
            case 's' :
            {
                // initialize scanner
                scan(yyin);
                break;        
            }
            case 'p' :
            {
                // initialize parser
                parse(yyin);
                break;
            }
            case 't' :
            {
                // initialize pretty printer
                prettyprint(yyin);
                break;
            }
            case 'r' :
            {
                // initialize resolver
                resolve(yyin);
                break;
            }
            case 'y' :
            {
                // initialize typechecker
                typecheck(yyin);
                // printf("we have exited typechecking.\n");
                break;
            }
            case 'c' :
            {
                // initialize code generator
                if (!argv[3])
                {
                    printf("error: please enter an output file name.\n");
                    exit(1);
                }
                FILE *outfil;
                outfil = fopen(argv[3],"w+");
                codegen(yyin, outfil);
                break;
            }
            default:
            {
                printf("No argument provided.\n");
                return 1;
            }
        }
    }

    return 0;
}
