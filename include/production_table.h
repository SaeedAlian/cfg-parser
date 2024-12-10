#ifndef _H_PRODUCTION_TABLE
#define _H_PRODUCTION_TABLE

#include <stdlib.h>
#include <string.h>

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
  const char *rhs;
  struct prod_rhs *next;
} prod_rhs;

typedef struct prod {
  char var;
  prod_rhs *first_rhs;
} prod;

typedef prod *prod_table;

prod_table new_prod_table();
int add_new_prod(prod_table *t, char var, const char *rhs);
prod get_prod(prod_table *t, char var);
int has_epsilon_rhs(prod_table *t, char var);
void free_prod_rhs(prod_rhs *rhs);
void free_prod_table(prod_table *t);

#endif
