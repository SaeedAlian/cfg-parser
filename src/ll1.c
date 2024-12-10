#include "../include/ll1.h"

char *first(prod_table *t, char var, int *len) {
  prod_rhs_stack *s = new_prod_rhs_stack(t->prods_len);

  int l = 0;
  int max = 5;
  char *f = (char *)malloc(sizeof(char) * max);

  int index = var - PRODS_INDEX_SHIFT;
  prod p = t->prods[index];
  prod_rhs *curr_rhs = p.first_rhs;

  while (curr_rhs != NULL) {
    char first = curr_rhs->rhs[0];

    if (first >= MIN_PROD_CHAR && first <= MAX_PROD_CHAR) {
      if (curr_rhs->next) {
        prod_rhs_stack_push(s, curr_rhs->next);
      }

      int nindex = first - PRODS_INDEX_SHIFT;
      prod np = t->prods[nindex];
      curr_rhs = np.first_rhs;
      continue;
    }

    if (l == max) {
      max *= 2;
      f = (char *)realloc(f, sizeof(char) * max);
    }

    f[l++] = first;

    if (curr_rhs->next == NULL) {
      if (prod_rhs_stack_is_empty(s)) {
        curr_rhs = NULL;
      } else {
        prod_rhs_stack_pop(s, &curr_rhs);
      }
    } else {
      curr_rhs = curr_rhs->next;
    }
  }

  *len = l;
  free_prod_rhs_stack(s);
  return f;
}
