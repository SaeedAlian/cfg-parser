#ifndef _H_PRODUCTION_TABLE
#define _H_PRODUCTION_TABLE

#include <stdlib.h>
#include <string.h>

#define EPSILON_DEFINITION_1 "epsilon"
#define EPSILON_DEFINITION_2 "eps"
#define EPSILON -100
#define MAX_PRODS 26
#define PRODS_INDEX_SHIFT 65
#define MIN_PROD_CHAR 'A'
#define MAX_PROD_CHAR 'Z'
#define SUCCESS_ADD_RULE 0
#define INCORRECT_VAR_SIGN -1
#define NULL_RULE_RECEIVED -2
#define NULL_TABLE_RECEIVED -3
#define EPS_PROD_FOUND 1
#define EPS_PROD_NOT_FOUND 0

typedef struct prod_rhs {
  char *rhs;
  struct prod_rhs *next;
} prod_rhs;

typedef struct prod {
  char var;
  int rhs_len;
  prod_rhs *first_rhs;
} prod;

typedef struct prod_table {
  prod *prods;
  int prods_len;
} prod_table;

typedef struct prod_rhs_stack {
  int top;
  int max;
  prod_rhs **data;
} prod_rhs_stack;

void free_prod_rhs_stack(prod_rhs_stack *s);

prod_rhs_stack *new_prod_rhs_stack(int max);
int prod_rhs_stack_is_full(prod_rhs_stack *s);
int prod_rhs_stack_is_empty(prod_rhs_stack *s);
prod_rhs *prod_rhs_stack_top(prod_rhs_stack *s);
int prod_rhs_stack_push(prod_rhs_stack *s, prod_rhs *rhs);
int prod_rhs_stack_pop(prod_rhs_stack *s, prod_rhs **rhs);

prod_table *new_prod_table();
int add_new_prod(prod_table *t, char var, const char *rhs);
prod get_prod(prod_table *t, char var);
int has_epsilon_rhs(prod_table *t, char var);
void free_prod_rhs(prod_rhs *rhs);
void free_prod_table(prod_table *t);

#endif
