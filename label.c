#include "label.h"

#include <stdio.h>
#include <stdlib.h>

/*
- these will generate a large num of unique anonymous labels that indicate:
 - targets of jumps
 - targets of conditional branches
  - int label_create();
  - const char * label_name( int label );

- label_create : increments a global counter and returns the current value
- label_name   : returns that label in a string form, ex. label 15 = ".L15"
*/

int label_count = 0;

int func_label = 0; // label of the function we're currently in

int label_create() 
{
	// increment global counter and return the current value
	return label_count++;
}

int label_create_func()
{
	func_label = label_count++;
	return func_label;
}

const char * label_name(int label)
{
	// return that label in a string form
	char *label_str = malloc(sizeof(char)*5);

	sprintf(label_str, ".L%i", label); // I don't think this is right

	return label_str;
}