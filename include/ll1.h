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
#define HASHMAP_INSERT_FAILED -1
#define HASHMAP_INSERT_SUCCESS 1
#define HASHMAP_KEY_FOUND_ERROR -2
#define HASHMAP_KEY_NOT_FOUND -1
#define HASHMAP_KEY_FOUND_SUCCESS 1
#define HASHMAP_INSERT_DUPLICATE -2
#define DUPLICATED_FIRST_FOUND 1
#define DUPLICATED_FIRST_NOT_FOUND 0
#define DUPLICATED_FOLLOW_FOUND 1
#define DUPLICATED_FOLLOW_NOT_FOUND 0
#define PARSE_TREE_ADD_NODE_SUCCESS 1
#define PARSE_TREE_ADD_NODE_ERROR -1
#define STRING_PARSE_SUCCESS 1
#define STRING_PARSE_ERROR -1

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

typedef struct rhs_hashmap_node {
  char key;
  production_rhs *data;
  struct rhs_hashmap_node *next;
} rhs_hashmap_node;

typedef struct rhs_hashmap {
  int max;
  rhs_hashmap_node **nodes;
} rhs_hashmap;

typedef struct ll1_hashmap_node {
  char key;
  rhs_hashmap *data;
  struct ll1_hashmap_node *next;
} ll1_hashmap_node;

typedef struct ll1_hashmap {
  int max;
  ll1_hashmap_node **nodes;
} ll1_hashmap;

typedef struct ll1_table {
  int vars_len;
  char *vars;
  int terminals_len;
  char *terminals;
  ll1_hashmap *table;
} ll1_table;

typedef struct ll1_parse_node {
  char c;
  int max_children;
  int children_len;
  struct ll1_parse_node *parent;
  struct ll1_parse_node **children;
} ll1_parse_node;

typedef struct ll1_parse_node_queue {
  int rear;
  int front;
  int max;
  ll1_parse_node **data;
} ll1_parse_node_queue;

typedef struct ll1_parse_node_stack {
  int top;
  int max;
  ll1_parse_node **data;
} ll1_parse_node_stack;

typedef struct ll1_parse_tree {
  int nodes;
  ll1_parse_node *root;
} ll1_parse_tree;

void free_ll1_parse_node_stack(ll1_parse_node_stack *s);
void free_ll1_parse_node_queue(ll1_parse_node_queue *q);
void free_ll1_parse_node(ll1_parse_node *n);
void free_ll1_parse_tree(ll1_parse_tree *t);
void free_rhs_hashmap_node(rhs_hashmap_node *n);
void free_rhs_hashmap(rhs_hashmap *hm);
void free_ll1_hashmap_node(ll1_hashmap_node *n);
void free_ll1_hashmap(ll1_hashmap *hm);
void free_ff_table(ff_table *t);
void free_ll1_table(ll1_table *t);

rhs_hashmap_node *new_rhs_hashmap_node(char k, production_rhs *v);
rhs_hashmap *new_rhs_hashmap(int max);
int rhs_hashmap_hash_func(rhs_hashmap *hm, char k);
int insert_into_rhs_hashmap(rhs_hashmap *hm, char k, production_rhs *v);
int search_rhs_hashmap(rhs_hashmap *hm, char k, production_rhs **output);
void print_rhs_hashmap_node(rhs_hashmap_node *n);
void print_rhs_hashmap(rhs_hashmap *hm);

ll1_hashmap_node *new_ll1_hashmap_node(char k, rhs_hashmap *v);
ll1_hashmap *new_ll1_hashmap(int max);
int ll1_hashmap_hash_func(ll1_hashmap *hm, char k);
int insert_into_ll1_hashmap(ll1_hashmap *hm, char k, rhs_hashmap *v);
int search_ll1_hashmap(ll1_hashmap *hm, char k, rhs_hashmap **output);
void print_ll1_hashmap_node(ll1_hashmap_node *n);
void print_ll1_hashmap(ll1_hashmap *hm);

ll1_parse_node_queue *new_ll1_parse_node_queue(int max);
int ll1_parse_node_queue_is_full(ll1_parse_node_queue *q);
int ll1_parse_node_queue_is_empty(ll1_parse_node_queue *q);
int ll1_parse_node_queue_increase(ll1_parse_node_queue *q);
ll1_parse_node *ll1_parse_node_queue_front(ll1_parse_node_queue *q);
int ll1_parse_node_queue_enqueue(ll1_parse_node_queue *q, ll1_parse_node *node);
int ll1_parse_node_queue_dequeue(ll1_parse_node_queue *q,
                                 ll1_parse_node **node);
int ll1_parse_node_queue_length(ll1_parse_node_queue *q);

ll1_parse_node_stack *new_ll1_parse_node_stack(int max);
int ll1_parse_node_stack_is_full(ll1_parse_node_stack *s);
int ll1_parse_node_stack_is_empty(ll1_parse_node_stack *s);
int ll1_parse_node_stack_increase(ll1_parse_node_stack *s);
ll1_parse_node *ll1_parse_node_stack_top(ll1_parse_node_stack *s);
int ll1_parse_node_stack_push(ll1_parse_node_stack *s, ll1_parse_node *c);
int ll1_parse_node_stack_pop(ll1_parse_node_stack *s, ll1_parse_node **c);

ll1_parse_node *new_ll1_parse_node(ll1_parse_node *parent, char val,
                                   int max_children);
int ll1_parse_tree_add_child(ll1_parse_tree *t, ll1_parse_node *node, char val,
                             int max_children);

ll1_parse_tree *new_ll1_parse_tree(char start_var, int max_children);

int check_first_duplicate(first *f, int f_len, char c);
int check_follow_duplicate(follow *f, int f_len, char c);

ff_table *new_ff_table(grammar *g);
int find_first(production_table *t, ff_table *fft, char var);
int calculate_firsts(grammar *g, ff_table *fft);
int find_follow(production_table *t, ff_table **fft, char var, char start_var);
int calculate_follows(grammar *g, ff_table *fft);

ll1_table *new_ll1_table(grammar *g, ff_table *fft);

int create_parse_tree_with_string(ll1_table *table,
                                  ll1_parse_tree **output_tree, char start_var,
                                  const char *str, int str_len);

void print_ll1_parse_node(ll1_parse_node *n, int level);
void print_ll1_parse_tree(ll1_parse_tree *t);
void print_ff_table(ff_table *t);
void print_ll1_table(ll1_table *t);

#endif
