#include "../include/production_table.h"

prod_table *new_prod_table() {
  prod_table *t = (prod_table *)malloc(sizeof(prod_table));
  t->prods = (prod *)malloc(sizeof(prod_table) * MAX_PRODS);
  t->prods_len = 0;

  if (t == NULL)
    return NULL;

  for (int i = 0; i < MAX_PRODS; i++) {
    prod n;
    n.var = i + PRODS_INDEX_SHIFT;
    n.first_rhs = NULL;
    t->prods[i] = n;
  }

  return t;
}

int add_new_prod(prod_table *t, char var, const char *rhs) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR)
    return INCORRECT_VAR_SIGN;
  if (rhs == NULL)
    return NULL_RULE_RECEIVED;
  if (t == NULL)
    return NULL_TABLE_RECEIVED;

  prod_rhs *new_rhs = (prod_rhs *)malloc(sizeof(prod_rhs));

  if (strcmp(rhs, EPSILON_DEFINITION_1) == 0 ||
      strcmp(rhs, EPSILON_DEFINITION_2) == 0) {
    new_rhs->rhs = (char *)malloc(sizeof(char) * 1);
    new_rhs->rhs[0] = EPSILON;
  } else {
    new_rhs->rhs = (char *)malloc(sizeof(char) * strlen(rhs));
    strcpy(new_rhs->rhs, rhs);
  }

  new_rhs->next = NULL;

  int index = var - PRODS_INDEX_SHIFT;

  if (t->prods[index].first_rhs != NULL) {
    new_rhs->next = t->prods[index].first_rhs;
  }

  t->prods[index].first_rhs = new_rhs;
  t->prods_len++;

  return SUCCESS_ADD_RULE;
}

prod get_prod(prod_table *t, char var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR) {
    prod fallback;
    fallback.var = '\0';
    fallback.first_rhs = NULL;
    return fallback;
  }

  int index = var - PRODS_INDEX_SHIFT;
  return t->prods[index];
}

int has_epsilon_rhs(prod_table *t, char var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR)
    return INCORRECT_VAR_SIGN;

  int index = var - PRODS_INDEX_SHIFT;
  prod p = t->prods[index];
  prod_rhs *curr = p.first_rhs;

  while (curr != NULL) {
    if (curr->rhs[0] == (char)EPSILON) {
      return EPS_PROD_FOUND;
    }

    curr = curr->next;
  }

  return EPS_PROD_NOT_FOUND;
}

void free_prod_table(prod_table *t) {
  for (int i = 0; i < MAX_PRODS; i++) {
    free_prod_rhs(t->prods[i].first_rhs);
  }

  free(t->prods);
  free(t);
}

void free_prod_rhs(prod_rhs *rhs) {
  if (rhs == NULL)
    return;
  free_prod_rhs(rhs->next);
  free(rhs->rhs);
  free(rhs);
}

prod_rhs_stack *new_prod_rhs_stack(int max) {
  prod_rhs_stack *s = (prod_rhs_stack *)malloc(sizeof(prod_rhs_stack));

  if (s == NULL)
    return NULL;

  s->data = (prod_rhs **)malloc(sizeof(prod_rhs *) * max);
  s->top = -1;
  s->max = max;

  if (s->data == NULL) {
    if (s != NULL)
      free(s);

    return NULL;
  }

  return s;
}

void free_prod_rhs_stack(prod_rhs_stack *s) {
  free(s->data);
  free(s);
}

int prod_rhs_stack_is_full(prod_rhs_stack *s) { return s->top == s->max - 1; }
int prod_rhs_stack_is_empty(prod_rhs_stack *s) { return s->top == -1; }

prod_rhs *prod_rhs_stack_top(prod_rhs_stack *s) { return s->data[s->top]; }

int prod_rhs_stack_push(prod_rhs_stack *s, prod_rhs *rhs) {
  int new_top = ++s->top;

  if (new_top > s->max - 1)
    return -1;

  s->data[new_top] = rhs;

  return 0;
}

int prod_rhs_stack_pop(prod_rhs_stack *s, prod_rhs **rhs) {
  if (s->top < 0)
    return -1;

  if (rhs != NULL) {
    (*rhs) = s->data[s->top];
  }

  s->top--;
  return 0;
}
