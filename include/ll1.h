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
#define DUPLICATED_FIRST_FOUND 1
#define DUPLICATED_FIRST_NOT_FOUND 0
#define DUPLICATED_FOLLOW_FOUND 1
#define DUPLICATED_FOLLOW_NOT_FOUND 0

typedef struct first {
  char c;
  production_rhs *rhs;
} first;

typedef struct follow {
  char c;
} follow;

typedef struct var_firsts {
  char var;
  int firsts_len;
  first *firsts;
} var_firsts;

typedef struct var_follows {
  char var;
  int follows_len;
  follow *follows;
} var_follows;

typedef struct ff_table {
  var_firsts *firsts;
  var_follows *follows;
} ff_table;

void free_ff_table(ff_table *t);

int check_first_duplicate(first *f, int f_len, char c);
int check_follow_duplicate(follow *f, int f_len, char c);

ff_table *new_ff_table(grammar *g);
int find_first(production_table *t, ff_table *fft, char var);
int calculate_firsts(grammar *g, ff_table *fft);
int find_follow(production_table *t, ff_table **fft, char var, char start_var);
int calculate_follows(grammar *g, ff_table *fft);

void print_ff_table(ff_table *t);

#endif
