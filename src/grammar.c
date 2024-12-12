#include "../include/grammar.h"

grammar *new_grammar(const char *vars, const char *terminals, char start_var) {
  grammar *new = (grammar *)malloc(sizeof(grammar));
  if (new == NULL)
    return NULL;

  new->productions_table = (production_table *)malloc(sizeof(production_table));
  if (new->productions_table == NULL) {
    free(new);
    return NULL;
  }

  new->productions_table->productions =
      (production *)malloc(sizeof(production) * MAX_PRODS);
  if (new->productions_table->productions == NULL) {
    free(new->productions_table);
    free(new);
    return NULL;
  }

  for (int i = 0; i < MAX_PRODS; i++) {
    production n;
    n.var = i + PRODS_INDEX_SHIFT;
    n.first_rhs = NULL;
    new->productions_table->productions[i] = n;
  }

  new->terminals = (char *)malloc(sizeof(char) * strlen(terminals) + 1);
  if (new->terminals == NULL) {
    free(new->productions_table->productions);
    free(new->productions_table);
    free(new);
    return NULL;
  }

  new->vars = (char *)malloc(sizeof(char) * strlen(vars) + 1);
  if (new->vars == NULL) {
    free(new->productions_table->productions);
    free(new->productions_table);
    free(new->terminals);
    free(new);
    return NULL;
  }

  if (strcpy(new->vars, vars) == NULL ||
      strcpy(new->terminals, terminals) == NULL) {
    free(new->productions_table->productions);
    free(new->productions_table);
    free(new->terminals);
    free(new->vars);
    free(new);
    return NULL;
  }

  new->start_var = start_var;

  return new;
}

int add_production(grammar *g, char var, const char *rhs) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR)
    return INCORRECT_VAR_SIGN;
  if (rhs == NULL)
    return NULL_RULE_RECEIVED;
  if (g == NULL)
    return NULL_GRAMMAR_RECEIVED;

  production_rhs *new_rhs = (production_rhs *)malloc(sizeof(production_rhs));

  if (strcmp(rhs, EPSILON_DEFINITION_1) == 0 ||
      strcmp(rhs, EPSILON_DEFINITION_2) == 0) {
    new_rhs->rhs = (char *)malloc(sizeof(char) * 1);
    new_rhs->rhs[0] = EPSILON;
  } else {
    new_rhs->rhs = (char *)malloc(sizeof(char) * strlen(rhs));
    strcpy(new_rhs->rhs, rhs);
  }

  new_rhs->for_var = var;
  new_rhs->next = NULL;

  int index = var - PRODS_INDEX_SHIFT;

  production_table *t = g->productions_table;

  if (t->productions[index].first_rhs != NULL) {
    new_rhs->next = t->productions[index].first_rhs;
  }

  t->productions[index].first_rhs = new_rhs;
  t->len++;

  return SUCCESS_ADD_PROD;
}

int get_production(grammar *g, char var, production *prod) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR) {
    return INCORRECT_VAR_SIGN;
  }

  production_table *t = g->productions_table;
  int index = var - PRODS_INDEX_SHIFT;
  production p = t->productions[index];

  if (p.first_rhs == NULL) {
    return UNDEFINED_PRODUCTION;
  }

  *prod = p;
  return PRODUCTION_FOUND;
}

int prod_has_epsilon_rhs(grammar *g, char var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR)
    return INCORRECT_VAR_SIGN;

  production_table *t = g->productions_table;

  int index = var - PRODS_INDEX_SHIFT;
  production p = t->productions[index];

  if (p.first_rhs == NULL) {
    return UNDEFINED_PRODUCTION;
  }

  production_rhs *curr = p.first_rhs;

  while (curr != NULL) {
    if (curr->rhs[0] == (char)EPSILON) {
      return EPS_PROD_FOUND;
    }

    curr = curr->next;
  }

  return EPS_PROD_NOT_FOUND;
}

void free_grammar(grammar *g) {
  free(g->vars);
  free(g->terminals);
  free_production_table(g->productions_table);
  free(g);
}

void free_production_table(production_table *t) {
  for (int i = 0; i < MAX_PRODS; i++) {
    free_production_rhs(t->productions[i].first_rhs);
  }

  free(t->productions);
  free(t);
}

void free_prod_rhs(production_rhs *rhs) {
  if (rhs == NULL)
    return;
  free_prod_rhs(rhs->next);
  free(rhs->rhs);
  free(rhs);
}

production_rhs_stack *new_production_rhs_stack(int max) {
  production_rhs_stack *s =
      (production_rhs_stack *)malloc(sizeof(production_rhs_stack));

  if (s == NULL)
    return NULL;

  s->data = (production_rhs **)malloc(sizeof(production_rhs *) * max);
  s->top = -1;
  s->max = max;

  if (s->data == NULL) {
    if (s != NULL)
      free(s);

    return NULL;
  }

  return s;
}

void free_production_rhs_stack(production_rhs_stack *s) {
  free(s->data);
  free(s);
}

int production_rhs_stack_is_full(production_rhs_stack *s) {
  return s->top == s->max - 1;
}
int production_rhs_stack_is_empty(production_rhs_stack *s) {
  return s->top == -1;
}

production_rhs *production_rhs_stack_top(production_rhs_stack *s) {
  return s->data[s->top];
}

int production_rhs_stack_push(production_rhs_stack *s, production_rhs *rhs) {
  int new_top = ++s->top;

  if (new_top > s->max - 1)
    return -1;

  s->data[new_top] = rhs;

  return 0;
}

int production_rhs_stack_pop(production_rhs_stack *s, production_rhs **rhs) {
  if (s->top < 0)
    return -1;

  if (rhs != NULL) {
    (*rhs) = s->data[s->top];
  }

  s->top--;
  return 0;
}
