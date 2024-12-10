#include "../include/production_table.h"

prod_table new_prod_table() {
  prod *t = (prod *)malloc(sizeof(prod) * MAX_PRODS);

  if (t == NULL)
    return NULL;

  for (int i = 0; i < MAX_PRODS; i++) {
    prod n;
    n.var = i + PRODS_INDEX_SHIFT;
    n.first_rhs = NULL;
    t[i] = n;
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
  new_rhs->rhs = rhs;
  new_rhs->next = NULL;

  int index = var - PRODS_INDEX_SHIFT;

  if ((*t)[index].first_rhs != NULL) {
    new_rhs->next = (*t)[index].first_rhs;
  }

  (*t)[index].first_rhs = new_rhs;

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
  return (*t)[index];
}

int has_epsilon_rhs(prod_table *t, char var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR)
    return INCORRECT_VAR_SIGN;

  int index = var - PRODS_INDEX_SHIFT;
  prod p = (*t)[index];
  prod_rhs *curr = p.first_rhs;

  while (curr != NULL) {
    if (strcmp(curr->rhs, "epsilon") == 0) {
      return EPS_PROD_FOUND;
    }

    curr = curr->next;
  }

  return EPS_PROD_NOT_FOUND;
}

void free_prod_table(prod_table *t) {
  for (int i = 0; i < MAX_PRODS; i++) {
    free_prod_rhs((*t)[i].first_rhs);
  }

  free(*t);
}

void free_prod_rhs(prod_rhs *rhs) {
  if (rhs == NULL)
    return;
  free_prod_rhs(rhs->next);
  free(rhs);
}
