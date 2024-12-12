#include "../include/grammar.h"

grammar *new_grammar(const char *vars, const char *terminals, char start_var) {
  if (terminals == NULL || vars == NULL)
    return NULL;

  grammar *g = (grammar *)malloc(sizeof(grammar));
  if (g == NULL)
    return NULL;

  g->productions_table = (production_table *)malloc(sizeof(production_table));
  if (g->productions_table == NULL) {
    free(g);
    return NULL;
  }

  g->productions_table->productions =
      (production *)malloc(sizeof(production) * MAX_PRODS);
  if (g->productions_table->productions == NULL) {
    free(g->productions_table);
    free(g);
    return NULL;
  }

  for (int i = 0; i < MAX_PRODS; i++) {
    production n;
    n.var = i + PRODS_INDEX_SHIFT;
    n.first_rhs = NULL;
    g->productions_table->productions[i] = n;
  }

  g->terminals_len = strlen(terminals);
  g->vars_len = strlen(vars);

  g->terminals = (char *)malloc(sizeof(char) * g->terminals_len + 1);
  if (g->terminals == NULL) {
    free(g->productions_table->productions);
    free(g->productions_table);
    free(g);
    return NULL;
  }

  g->vars = (char *)malloc(sizeof(char) * g->vars_len + 1);
  if (g->vars == NULL) {
    free(g->productions_table->productions);
    free(g->productions_table);
    free(g->terminals);
    free(g);
    return NULL;
  }

  if (strcpy(g->vars, vars) == NULL ||
      strcpy(g->terminals, terminals) == NULL) {
    free(g->productions_table->productions);
    free(g->productions_table);
    free(g->terminals);
    free(g->vars);
    free(g);
    return NULL;
  }

  if (strchr(vars, start_var) == NULL) {
    free(g->productions_table->productions);
    free(g->productions_table);
    free(g->terminals);
    free(g->vars);
    free(g);
    return NULL;
  }

  g->start_var = start_var;

  return g;
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

void print_grammar(grammar *g) {
  printf("Grammar:\n");
  printf("   Variables: ");

  for (int i = 0; i < g->vars_len; i++) {
    if (i == g->vars_len - 1) {
      printf("%c", g->vars[i]);
    } else {
      printf("%c,", g->vars[i]);
    }
  }

  printf("\n");
  printf("   Terminals: ");

  for (int i = 0; i < g->terminals_len; i++) {
    if (i == g->terminals_len - 1) {
      printf("%c", g->terminals[i]);
    } else {
      printf("%c,", g->terminals[i]);
    }
  }

  printf("\n");
  printf("   Start Variable: %c", g->start_var);
  printf("\n");

  printf("   Productions:\n");

  production_table *t = g->productions_table;

  production start = t->productions[g->start_var - PRODS_INDEX_SHIFT];
  print_production(start, 6);
  printf("\n");

  for (int i = 0; i < MAX_PRODS; i++) {
    production p = t->productions[i];
    if (p.first_rhs != NULL && p.var != g->start_var) {

      print_production(p, 6);
      printf("\n");
    }
  }
}

void print_production(production p, int space_indent) {
  for (int i = 0; i < space_indent; i++) {
    printf(" ");
  }

  printf("%c -> ", p.var);

  production_rhs *curr = p.first_rhs;

  while (curr != NULL) {
    if (curr->next == NULL) {
      if (curr->rhs[0] == EPSILON) {
        printf("epsilon");
      } else {
        printf("%s", curr->rhs);
      }
      break;
    } else {
      if (curr->rhs[0] == EPSILON) {
        printf("epsilon | ");
      } else {
        printf("%s | ", curr->rhs);
      }
      curr = curr->next;
    }
  }
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

void free_production_rhs(production_rhs *rhs) {
  if (rhs == NULL)
    return;
  free_production_rhs(rhs->next);
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

char_stack *new_char_stack(int max) {
  char_stack *s = (char_stack *)malloc(sizeof(char_stack));
  s->data = (char **)malloc(sizeof(char *) * max);
  s->top = -1;
  s->max = max;

  if (s->data == NULL) {
    if (s != NULL)
      free(s);

    return NULL;
  }

  return s;
}

void free_char_stack(char_stack *s) {
  free(s->data);
  free(s);
}

int char_stack_is_full(char_stack *s) { return s->top == s->max - 1; }
int char_stack_is_empty(char_stack *s) { return s->top == -1; }

char *char_stack_top(char_stack *s) { return s->data[s->top]; }

int char_stack_push(char_stack *s, char *state) {
  int new_top = ++s->top;

  if (new_top > s->max - 1)
    return -1;

  s->data[new_top] = state;

  return 0;
}

int char_stack_pop(char_stack *s, char **state) {
  if (s->top < 0)
    return -1;

  if (state != NULL) {
    (*state) = s->data[s->top];
  }

  s->top--;
  return 0;
}
