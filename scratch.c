#include "scratch.h"

#include <stdio.h>
#include <stdlib.h>

/*
- supporting functions
  - int scratch_alloc();
  - void scratch_free( int r );
  - const char * scratch_name( int r );

- take scratch values and place them into a table of which register they are, their name, and whether they're in use
- scratch_alloc : finds an unused register in the table, marks it as in use, and returns the register number "r"
  - if it cannot find a free register, just emit an error message and halt
- scratch_free  : marks an indicated register as available, frees it up in the table
- scratch_name  : returns the name of a register given its number "r"
- running out of scratch registers is possible but unlikely (these would be our local var regs x9-x15)
*/

// declare a global table for scratch values
// To keep the calling conventions simple, calls to functions with more than six arguments may fail with a "too many arguments" error
int reg_table[6] = {0,0,0,0,0,0};

/* scratch_alloc: find unused register in the table, mark it as in use, return the register number "r"*/
int scratch_alloc()
{
	for (int i=0;i<6;i++)
	{
		if (reg_table[i] == 0) // check if reg i is unused
		{
			reg_table[i] = 1;
			// scratch_print();
			return i;
		}
	}

	// loop exits, no available registers
	printf("codegen error: too many registers in use.\n");
	exit(1);
}

/* scratch_free: marks indicated register as available, frees it up in the table */
void scratch_free(int r)
{
	if (r >= 0 && r < 6) reg_table[r] = 0;
	else
	{
		printf("codegen error: register %i does not exist\n", r);
		// exit(1);
	}

	// scratch_print();
}

/* scratch_name: returns the name of a register given its number "r" */
const char * scratch_name(int r)
{
	// name of the register is based on r
	switch (r)
	{
		case 0:
		    return "x0";
		case 1:
		    return "x1";
		case 2:
		    return "x2";
		case 3:
		    return "x3";
		case 4:
		    return "x4";
		case 5:
		    return "x5";
		case 19:
			return "x19";
	}
	printf("codegen error: register %i does not exist\n", r);
	exit(1);
}

void scratch_print()
{
	printf("scratch table: [");
	for (int i=0;i<6;i++)
	{
		printf("%i ",reg_table[i]);
	}
	printf("]\n");
}