#ifndef _H_GRAMMAR
#define _H_GRAMMAR

#include <stdlib.h>
#include <string.h>

// consts
#define EPSILON_DEFINITION_1 "epsilon"
#define EPSILON_DEFINITION_2 "eps"
#define EPSILON -100
#define MAX_PRODS 26
#define PRODS_INDEX_SHIFT 65
#define MIN_PROD_CHAR 'A'
#define MAX_PROD_CHAR 'Z'

// return codes
#define SUCCESS_ADD_PROD 0
#define INCORRECT_VAR_SIGN -1
#define NULL_RULE_RECEIVED -2
#define NULL_GRAMMAR_RECEIVED -3
#define UNDEFINED_PRODUCTION -4
#define PRODUCTION_FOUND 1
#define EPS_PROD_FOUND 1
#define EPS_PROD_NOT_FOUND 0

typedef struct production_rhs {
  char *rhs;
  char for_var;
  struct production_rhs *next;
} production_rhs;

typedef struct production {
  char var;
  int len;
  production_rhs *first_rhs;
} production;

typedef struct production_table {
  production *productions;
  int len;
} production_table;

typedef struct production_rhs_stack {
  int top;
  int max;
  production_rhs **data;
} production_rhs_stack;

typedef struct grammar {
  char *vars;
  char *terminals;
  char start_var;
  production_table *productions_table;
} grammar;

void free_production_rhs_stack(production_rhs_stack *s);
void free_production_rhs(production_rhs *rhs);
void free_production_table(production_table *t);
void free_grammar(grammar *g);

production_rhs_stack *new_production_rhs_stack(int max);
int production_rhs_stack_is_full(production_rhs_stack *s);
int production_rhs_stack_is_empty(production_rhs_stack *s);
production_rhs *production_rhs_stack_top(production_rhs_stack *s);
int production_rhs_stack_push(production_rhs_stack *s, production_rhs *rhs);
int production_rhs_stack_pop(production_rhs_stack *s, production_rhs **rhs);

grammar *new_grammar(const char *vars, const char *terminals, char start_var);
int add_production(grammar *g, char var, const char *rhs);
int get_production(grammar *g, char var, production *prod);
int var_has_epsilon_rhs(grammar *g, char var);

#endif