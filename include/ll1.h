#ifndef _H_LL1
#define _H_LL1

#include "./grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// consts
#define TERMINATE_SYMBOL '$'
#define FOLLOW_LEN_NOT_CALCULATED -2
#define FOLLOW_LEN_CALCULATING -1

// return codes
#define GRAMMAR_IS_NOT_LL1 -3
#define ERROR_ON_FIRST_CALC -1
#define ERROR_ON_FOLLOW_CALC -1
#define SUCCESS_ON_FIRST_CALC 1
#define SUCCESS_ON_FOLLOW_CALC 1
#define FOLLOW_ALREADY_CALCULATED -2

typedef struct first {
  char var;
  int firsts_len;
  char *firsts;
} first;

typedef struct follow {
  char var;
  int follows_len;
  char *follows;
} follow;

typedef struct ff_table {
  first *firsts;
  follow *follows;
} ff_table;

void free_ff_table(ff_table *t);

ff_table *new_ff_table(grammar *g);
int find_first(production_table *t, ff_table *fft, char var);
int calculate_firsts(grammar *g, ff_table *fft);
int find_follow(production_table *t, ff_table **fft, char var, char start_var);
int calculate_follows(grammar *g, ff_table *fft);

void print_ff_table(ff_table *t);

#endif
