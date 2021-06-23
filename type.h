#ifndef TYPE_H
#define TYPE_H

#include "param_list.h"

typedef enum {
	TYPE_VOID, 		// 0
	TYPE_BOOLEAN,   // 1
	TYPE_CHARACTER, // 2
	TYPE_INTEGER,   // 3
	TYPE_STRING,    // 4
	TYPE_ARRAY,     // 5
	TYPE_FUNCTION,  // 6
	TYPE_PROTO,     // 7
	TYPE_AUTO       // 8
} type_t;

struct type {
	type_t kind;
	struct param_list *params;
	struct type *subtype;
	int size;
};

struct type * type_create( type_t kind, struct type *subtype, struct param_list *params, int size );

struct type * type_copy( struct type *t );
struct type * subtype_copy( struct type *t ); 
int type_compare( struct type *a, struct type *b );
void type_delete( struct type *t );

void type_resolve( struct type *t, int print );

void type_print( struct type *t );



#endif
