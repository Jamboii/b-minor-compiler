#ifndef SCRATCH_H
#define SCRATCH_H

void scratch_print();

int scratch_alloc();

void scratch_free( int r );

const char * scratch_name( int r );

#endif