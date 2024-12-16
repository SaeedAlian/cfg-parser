#include "../include/ll1.h"

ll1_table *new_ll1_table(grammar *g, ff_table *fft) {
  if (g == NULL || fft == NULL)
    return NULL;

  ll1_table *nt = (ll1_table *)malloc(sizeof(ll1_table));
  if (nt == NULL)
    return NULL;

  nt->vars_len = g->vars_len;
  nt->terminals_len = g->terminals_len;

  ll1_hashmap *ll1_hm = new_ll1_hashmap(nt->vars_len);
  if (ll1_hm == NULL) {
    free(nt);
    return NULL;
  }

  for (int i = 0; i < nt->vars_len; i++) {
    char var = g->vars[i];

    int index = var - PRODS_INDEX_SHIFT;
    var_firsts fr = fft->firsts[index];
    var_follows fl = fft->follows[index];
    int has_epsilon_first = var_has_epsilon_rhs(g, var);
    production p = g->productions_table->productions[index];
    production_rhs *epsilon_production_rhs = NULL;

    rhs_hashmap *rhs_hm = new_rhs_hashmap(nt->terminals_len + 1);
    if (rhs_hm == NULL) {
      free_ll1_hashmap(ll1_hm);
      free(nt);
      return NULL;
    }

    for (int j = 0; j < fr.firsts_len; j++) {
      first fi = fr.firsts[j];

      if (fi.c == EPSILON) {
        epsilon_production_rhs = fi.rhs;
        continue;
      }

      int res = insert_into_rhs_hashmap(rhs_hm, fi.c, fi.rhs);
      if (res != HASHMAP_INSERT_SUCCESS) {
        free_rhs_hashmap(rhs_hm);
        free_ll1_hashmap(ll1_hm);
        free(nt);
        return NULL;
      }
    }

    if (has_epsilon_first && epsilon_production_rhs != NULL) {
      for (int j = 0; j < fl.follows_len; j++) {
        follow fo = fl.follows[j];

        int res = insert_into_rhs_hashmap(rhs_hm, fo.c, epsilon_production_rhs);
        if (res != HASHMAP_INSERT_SUCCESS) {
          free_rhs_hashmap(rhs_hm);
          free_ll1_hashmap(ll1_hm);
          free(nt);
          return NULL;
        }
      }
    }

    int res = insert_into_ll1_hashmap(ll1_hm, var, rhs_hm);
    if (res != HASHMAP_INSERT_SUCCESS) {
      free_rhs_hashmap(rhs_hm);
      free_ll1_hashmap(ll1_hm);
      free(nt);
      return NULL;
    }
  }

  nt->terminals = (char *)malloc(sizeof(char) * nt->terminals_len + 1);
  if (nt->terminals == NULL) {
    free_ll1_hashmap(ll1_hm);
    free(nt);
    return NULL;
  }

  nt->vars = (char *)malloc(sizeof(char) * nt->vars_len + 1);
  if (nt->vars == NULL) {
    free_ll1_hashmap(ll1_hm);
    free(nt->terminals);
    free(nt);
    return NULL;
  }

  if (strcpy(nt->vars, g->vars) == NULL ||
      strcpy(nt->terminals, g->terminals) == NULL) {
    free_ll1_hashmap(ll1_hm);
    free(nt->terminals);
    free(nt->vars);
    free(nt);
    return NULL;
  }

  nt->table = ll1_hm;
  return nt;
}

ff_table *new_ff_table(grammar *g) {
  if (g == NULL)
    return NULL;

  ff_table *nt = (ff_table *)malloc(sizeof(ff_table));
  if (nt == NULL)
    return NULL;

  nt->firsts = (var_firsts *)malloc(sizeof(var_firsts) * MAX_PRODS);
  if (nt->firsts == NULL) {
    free(nt);
    return NULL;
  }

  nt->follows = (var_follows *)malloc(sizeof(var_follows) * MAX_PRODS);
  if (nt->follows == NULL) {
    free(nt->firsts);
    free(nt);
    return NULL;
  }

  for (int i = 0; i < MAX_PRODS; i++) {
    nt->firsts[i].firsts = NULL;
    nt->follows[i].follows = NULL;
    nt->firsts[i].firsts_len = FOLLOW_LEN_NOT_CALCULATED;
    nt->follows[i].follows_len = FOLLOW_LEN_NOT_CALCULATED;

    production_table *t = g->productions_table;
    production p = t->productions[i];

    if (p.first_rhs == NULL) {
      nt->firsts[i].var = '\0';
      nt->follows[i].var = '\0';
    } else {
      nt->firsts[i].var = p.var;
      nt->follows[i].var = p.var;
    }
  }

  return nt;
}

int calculate_firsts(grammar *g, ff_table *fft) {
  production_table *t = g->productions_table;

  for (int i = 0; i < MAX_PRODS; i++) {
    production p = t->productions[i];

    if (p.first_rhs != NULL) {
      int res = find_first(t, fft, p.var);
      if (res != SUCCESS_ON_FIRST_CALC)
        return res;
    }
  }

  return SUCCESS_ON_FIRST_CALC;
}

int calculate_follows(grammar *g, ff_table *fft) {
  production_table *t = g->productions_table;

  for (int i = 0; i < MAX_PRODS; i++) {
    production p = t->productions[i];

    if (p.first_rhs != NULL) {
      var_follows curr = fft->follows[i];

      if (curr.follows == NULL &&
          curr.follows_len == FOLLOW_LEN_NOT_CALCULATED && curr.var != '\0') {
        int res = find_follow(t, &fft, p.var, g->start_var);

        if (res == FOLLOW_ALREADY_CALCULATED)
          continue;
        if (res != SUCCESS_ON_FOLLOW_CALC)
          return res;
      }
    }
  }

  return SUCCESS_ON_FOLLOW_CALC;
}

int find_first(production_table *t, ff_table *fft, char var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR) {
    return INCORRECT_VAR_SIGN;
  }

  production_rhs_stack *s = new_production_rhs_stack(t->len);

  if (s == NULL) {
    return ERROR_ON_FIRST_CALC;
  }

  int l = 0;
  int max = 5;
  first *f = (first *)malloc(sizeof(first) * max);

  if (f == NULL) {
    free_production_rhs_stack(s);
    return ERROR_ON_FIRST_CALC;
  }

  int index = var - PRODS_INDEX_SHIFT;
  production p = t->productions[index];
  production_rhs *curr_rhs = p.first_rhs;
  production_rhs *selected_rhs = p.first_rhs;

  if (curr_rhs == NULL || fft->firsts[index].var == '\0') {
    free(f);
    free_production_rhs_stack(s);
    return UNDEFINED_PRODUCTION;
  }

  while (curr_rhs != NULL) {
    char fi = curr_rhs->rhs[0];

    if (fi >= MIN_PROD_CHAR && fi <= MAX_PROD_CHAR) {
      if (curr_rhs->next) {
        if (production_rhs_stack_push(s, curr_rhs->next) != 0) {
          free(f);
          free_production_rhs_stack(s);
          return UNDEFINED_PRODUCTION;
        }
      }

      int nindex = fi - PRODS_INDEX_SHIFT;
      production np = t->productions[nindex];
      curr_rhs = np.first_rhs;
      continue;
    }

    if (check_first_duplicate(f, l, fi) == DUPLICATED_FIRST_NOT_FOUND) {
      if (l == max) {
        max *= 2;
        first *temp = (first *)realloc(f, sizeof(first) * max);

        if (temp == NULL) {
          free(f);
          free_production_rhs_stack(s);
          return ERROR_ON_FIRST_CALC;
        }

        f = temp;
      }

      f[l].c = fi;
      f[l].rhs = selected_rhs;
      l++;
    }

    if (curr_rhs->next == NULL) {
      if (production_rhs_stack_is_empty(s)) {
        curr_rhs = NULL;
      } else {
        if (production_rhs_stack_pop(s, &curr_rhs) != 0) {
          free(f);
          free_production_rhs_stack(s);
          return UNDEFINED_PRODUCTION;
        }
      }
    } else {
      curr_rhs = curr_rhs->next;
    }

    if (curr_rhs != NULL && curr_rhs->for_var == var)
      selected_rhs = curr_rhs;
  }

  fft->firsts[index].firsts = f;
  fft->firsts[index].firsts_len = l;

  free_production_rhs_stack(s);
  return SUCCESS_ON_FIRST_CALC;
}

int find_follow(production_table *t, ff_table **fft, char var, char start_var) {
  if (var < MIN_PROD_CHAR || var > MAX_PROD_CHAR) {
    return INCORRECT_VAR_SIGN;
  }

  production_rhs_stack *s = new_production_rhs_stack(t->len * 2);
  char_stack *matches = new_char_stack(t->len * 2);

  if (s == NULL) {
    return ERROR_ON_FOLLOW_CALC;
  }

  int l = 0;
  int max = 5;
  follow *f = (follow *)malloc(sizeof(follow) * max);

  if (f == NULL) {
    free_char_stack(matches);
    free_production_rhs_stack(s);
    return ERROR_ON_FOLLOW_CALC;
  }

  int index = var - PRODS_INDEX_SHIFT;
  var_follows *fl = &(*fft)->follows[index];
  int is_calculated = fl->follows_len != FOLLOW_LEN_CALCULATING &&
                      fl->follows_len != FOLLOW_LEN_NOT_CALCULATED;

  if ((*fft)->follows[index].var == '\0') {
    free(f);
    free_char_stack(matches);
    free_production_rhs_stack(s);
    return UNDEFINED_PRODUCTION;
  }

  if (is_calculated) {
    free(f);
    free_char_stack(matches);
    free_production_rhs_stack(s);
    return FOLLOW_ALREADY_CALCULATED;
  }

  fl->follows_len = FOLLOW_LEN_CALCULATING;

  if (var == start_var) {
    f[l].c = TERMINATE_SYMBOL;
    l++;
  }

  for (int i = 0; i < MAX_PRODS; i++) {
    production p = t->productions[i];
    production_rhs *curr = p.first_rhs;

    while (curr != NULL) {
      char *match = strchr(curr->rhs, var);
      if (match != NULL) {
        if (production_rhs_stack_push(s, curr) != 0) {
          free(f);
          free_char_stack(matches);
          free_production_rhs_stack(s);
          return ERROR_ON_FOLLOW_CALC;
        }
        if (char_stack_push(matches, match) != 0) {
          free(f);
          free_char_stack(matches);
          free_production_rhs_stack(s);
          return ERROR_ON_FOLLOW_CALC;
        }
      }

      curr = curr->next;
    }
  }

  if (production_rhs_stack_is_empty(s)) {
    fl->follows = f;
    fl->follows_len = l;

    free_char_stack(matches);
    free_production_rhs_stack(s);
    return SUCCESS_ON_FOLLOW_CALC;
  }

  production_rhs *top;
  char *match;

  while (!char_stack_is_empty(matches) && !production_rhs_stack_is_empty(s)) {
    if (production_rhs_stack_pop(s, &top) != 0) {
      free(f);
      free_char_stack(matches);
      free_production_rhs_stack(s);
      return ERROR_ON_FOLLOW_CALC;
    }

    if (char_stack_pop(matches, &match) != 0) {
      free(f);
      free_char_stack(matches);
      free_production_rhs_stack(s);
      return ERROR_ON_FOLLOW_CALC;
    }

    char *next = (match + sizeof(char));

    if ((*next == '\0') || (*next >= MIN_PROD_CHAR && *next <= MAX_PROD_CHAR)) {
      char *curr_char = next;
      int is_reached_non_epsilon_var = 0;

      while (*curr_char >= MIN_PROD_CHAR && *curr_char <= MAX_PROD_CHAR) {
        var_firsts curr_char_firsts =
            (*fft)->firsts[*curr_char - PRODS_INDEX_SHIFT];

        if (curr_char_firsts.firsts == NULL || curr_char_firsts.var == '\0') {
          free(f);
          free_char_stack(matches);
          free_production_rhs_stack(s);
          return ERROR_ON_FOLLOW_CALC;
        }

        int contains_epsilon = 0;

        for (int i = 0; i < curr_char_firsts.firsts_len; i++) {
          first fi = curr_char_firsts.firsts[i];

          if (fi.c == EPSILON) {
            contains_epsilon = 1;
            continue;
          }

          if (check_follow_duplicate(f, l, fi.c) == DUPLICATED_FOLLOW_FOUND) {
            continue;
          }

          if (l == max) {
            max *= 2;
            follow *temp = (follow *)realloc(f, sizeof(follow) * max);

            if (temp == NULL) {
              free(f);
              free_char_stack(matches);
              free_production_rhs_stack(s);
              return ERROR_ON_FOLLOW_CALC;
            }

            f = temp;
          }

          f[l].c = fi.c;
          l++;
        }

        if (!contains_epsilon) {
          is_reached_non_epsilon_var = 1;
          break;
        }

        curr_char = curr_char + sizeof(char);
      }

      if (*curr_char != '\0' && !is_reached_non_epsilon_var) {
        if (check_follow_duplicate(f, l, *curr_char) ==
            DUPLICATED_FOLLOW_NOT_FOUND) {
          if (l == max) {
            max *= 2;
            follow *temp = (follow *)realloc(f, sizeof(follow) * max);

            if (temp == NULL) {
              free(f);
              free_char_stack(matches);
              free_production_rhs_stack(s);
              return ERROR_ON_FOLLOW_CALC;
            }

            f = temp;
          }

          f[l].c = *curr_char;
          l++;
        }
      }

      if (*curr_char == '\0' && !is_reached_non_epsilon_var) {
        char top_lhs = top->for_var;

        if (top_lhs != *match) {
          var_follows top_lhs_follows =
              (*fft)->follows[top_lhs - PRODS_INDEX_SHIFT];

          int lhs_follows_is_calculated =
              top_lhs_follows.follows_len != FOLLOW_LEN_CALCULATING &&
              top_lhs_follows.follows_len != FOLLOW_LEN_NOT_CALCULATED;

          int lhs_follows_is_calculating =
              top_lhs_follows.follows_len == FOLLOW_LEN_CALCULATING;

          if (lhs_follows_is_calculating) {
            fl->follows = NULL;
            fl->follows_len = FOLLOW_LEN_NOT_CALCULATED;

            free(f);
            free_char_stack(matches);
            free_production_rhs_stack(s);
            return GRAMMAR_IS_NOT_LL1;
          } else if (lhs_follows_is_calculated) {
            for (int i = 0; i < top_lhs_follows.follows_len; i++) {
              follow fo = top_lhs_follows.follows[i];

              if (check_follow_duplicate(f, l, fo.c) ==
                  DUPLICATED_FOLLOW_FOUND) {
                continue;
              }

              if (l == max) {
                max *= 2;
                follow *temp = (follow *)realloc(f, sizeof(follow) * max);

                if (temp == NULL) {
                  free(f);
                  free_char_stack(matches);
                  free_production_rhs_stack(s);
                  return ERROR_ON_FOLLOW_CALC;
                }

                f = temp;
              }

              f[l].c = fo.c;
              l++;
            }
          } else {
            int res = find_follow(t, fft, top_lhs, start_var);

            if (res != SUCCESS_ON_FOLLOW_CALC) {
              free(f);
              free_char_stack(matches);
              free_production_rhs_stack(s);
              return res;
            }

            var_follows updated_top_lhs_follows =
                (*fft)->follows[top_lhs - PRODS_INDEX_SHIFT];

            for (int i = 0; i < updated_top_lhs_follows.follows_len; i++) {
              follow fo = updated_top_lhs_follows.follows[i];

              if (check_follow_duplicate(f, l, fo.c) ==
                  DUPLICATED_FOLLOW_FOUND) {
                continue;
              }

              if (l == max) {
                max *= 2;
                follow *temp = (follow *)realloc(f, sizeof(follow) * max);

                if (temp == NULL) {
                  free(f);
                  free_char_stack(matches);
                  free_production_rhs_stack(s);
                  return ERROR_ON_FOLLOW_CALC;
                }

                f = temp;
              }

              f[l].c = fo.c;
              l++;
            }
          }
        }
      }
    } else {
      if (check_follow_duplicate(f, l, *next) == DUPLICATED_FOLLOW_NOT_FOUND) {
        if (l == max) {
          max *= 2;
          follow *temp = (follow *)realloc(f, sizeof(follow) * max);

          if (temp == NULL) {
            free(f);
            free_char_stack(matches);
            free_production_rhs_stack(s);
            return ERROR_ON_FOLLOW_CALC;
          }

          f = temp;
        }

        f[l].c = *next;
        l++;
      }
    }
  }

  fl->follows = f;
  fl->follows_len = l;
  free_char_stack(matches);
  free_production_rhs_stack(s);
  return SUCCESS_ON_FOLLOW_CALC;
}

int check_first_duplicate(first *f, int f_len, char c) {
  for (int i = 0; i < f_len; i++) {
    if (f[i].c == c)
      return DUPLICATED_FIRST_FOUND;
  }

  return DUPLICATED_FIRST_NOT_FOUND;
}

int check_follow_duplicate(follow *f, int f_len, char c) {
  for (int i = 0; i < f_len; i++) {
    if (f[i].c == c)
      return DUPLICATED_FOLLOW_FOUND;
  }

  return DUPLICATED_FOLLOW_NOT_FOUND;
}

ll1_hashmap_node *new_ll1_hashmap_node(char k, rhs_hashmap *v) {
  ll1_hashmap_node *n = (ll1_hashmap_node *)malloc(sizeof(ll1_hashmap_node));

  if (n == NULL)
    return NULL;

  n->key = k;
  n->data = v;
  n->next = NULL;

  return n;
}

ll1_hashmap *new_ll1_hashmap(int max) {
  ll1_hashmap *n = (ll1_hashmap *)malloc(sizeof(ll1_hashmap));
  if (n == NULL)
    return NULL;

  n->max = max;

  n->nodes = (ll1_hashmap_node **)malloc(sizeof(ll1_hashmap_node *) * max);
  if (n->nodes == NULL) {
    free(n);
    return NULL;
  }

  for (int i = 0; i < n->max; i++)
    n->nodes[i] = NULL;

  return n;
}

rhs_hashmap_node *new_rhs_hashmap_node(char k, production_rhs *v) {
  rhs_hashmap_node *n = (rhs_hashmap_node *)malloc(sizeof(rhs_hashmap_node));

  if (n == NULL)
    return NULL;

  n->key = k;
  n->data = v;
  n->next = NULL;

  return n;
}

rhs_hashmap *new_rhs_hashmap(int max) {
  rhs_hashmap *n = (rhs_hashmap *)malloc(sizeof(rhs_hashmap));
  if (n == NULL)
    return NULL;

  n->max = max;

  n->nodes = (rhs_hashmap_node **)malloc(sizeof(rhs_hashmap_node *) * max);
  if (n->nodes == NULL) {
    free(n);
    return NULL;
  }

  for (int i = 0; i < n->max; i++)
    n->nodes[i] = NULL;

  return n;
}

int insert_into_ll1_hashmap(ll1_hashmap *hm, char k, rhs_hashmap *v) {
  int index = ll1_hashmap_hash_func(hm, k);
  if (index >= hm->max)
    return HASHMAP_INSERT_FAILED;

  ll1_hashmap_node *new_node = new_ll1_hashmap_node(k, v);
  if (new_node == NULL)
    return HASHMAP_INSERT_FAILED;

  ll1_hashmap_node **curr_node = &hm->nodes[index];

  while ((*curr_node) != NULL) {
    if ((*curr_node)->key == k)
      return HASHMAP_INSERT_DUPLICATE;

    curr_node = &(*curr_node)->next;
  }

  *curr_node = new_node;

  return HASHMAP_INSERT_SUCCESS;
}

int search_ll1_hashmap(ll1_hashmap *hm, char k, rhs_hashmap **output) {
  int index = ll1_hashmap_hash_func(hm, k);
  if (index >= hm->max)
    return HASHMAP_KEY_FOUND_ERROR;

  ll1_hashmap_node *curr_node = hm->nodes[index];

  while (curr_node != NULL) {
    if (curr_node->key == k) {
      *output = curr_node->data;
      return HASHMAP_KEY_FOUND_SUCCESS;
    }

    curr_node = curr_node->next;
  }

  return HASHMAP_KEY_NOT_FOUND;
}

int insert_into_rhs_hashmap(rhs_hashmap *hm, char k, production_rhs *v) {
  int index = rhs_hashmap_hash_func(hm, k);
  if (index >= hm->max)
    return HASHMAP_INSERT_FAILED;

  rhs_hashmap_node *new_node = new_rhs_hashmap_node(k, v);
  if (new_node == NULL)
    return HASHMAP_INSERT_FAILED;

  rhs_hashmap_node **curr_node = &hm->nodes[index];

  while ((*curr_node) != NULL) {
    if ((*curr_node)->key == k)
      return HASHMAP_INSERT_DUPLICATE;

    curr_node = &(*curr_node)->next;
  }

  *curr_node = new_node;

  return HASHMAP_INSERT_SUCCESS;
}

int search_rhs_hashmap(rhs_hashmap *hm, char k, production_rhs **output) {
  int index = rhs_hashmap_hash_func(hm, k);
  if (index >= hm->max)
    return HASHMAP_KEY_FOUND_ERROR;

  rhs_hashmap_node *curr_node = hm->nodes[index];

  while (curr_node != NULL) {
    if (curr_node->key == k) {
      *output = curr_node->data;
      return HASHMAP_KEY_FOUND_SUCCESS;
    }

    curr_node = curr_node->next;
  }

  return HASHMAP_KEY_NOT_FOUND;
}

int ll1_hashmap_hash_func(ll1_hashmap *hm, char k) { return k % hm->max; }
int rhs_hashmap_hash_func(rhs_hashmap *hm, char k) { return k % hm->max; }

void print_ll1_table(ll1_table *t) {
  printf("LL1 Table:\n\n");
  printf("      |");

  int padding;
  production_rhs *max_len_rhs = NULL;

  for (int i = 0; i < t->vars_len; i++) {
    char var = t->vars[i];
    rhs_hashmap *rhs_hm;

    production_rhs *curr_rhs = NULL;

    if (search_ll1_hashmap(t->table, var, &rhs_hm) ==
        HASHMAP_KEY_FOUND_SUCCESS) {

      for (int j = 0; j < t->terminals_len + 1; j++) {
        char terminal;

        if (j == t->terminals_len) {
          terminal = TERMINATE_SYMBOL;
        } else {
          terminal = t->terminals[j];
        }

        if (search_rhs_hashmap(rhs_hm, terminal, &curr_rhs) ==
            HASHMAP_KEY_FOUND_SUCCESS) {
          if (max_len_rhs == NULL) {
            max_len_rhs = curr_rhs;
            continue;
          }

          if (strlen(max_len_rhs->rhs) < strlen(curr_rhs->rhs)) {
            max_len_rhs = curr_rhs;
          }
        }
      }
    }
  }

  if (max_len_rhs != NULL) {
    padding = strlen(max_len_rhs->rhs) + 1;

    if (padding % 2 == 1)
      padding++;
  } else {
    padding = 6;
  }

  int totalspace = padding * 2 + 1;

  for (int i = 0; i < t->terminals_len + 1; i++) {
    char terminal;

    if (i == t->terminals_len) {
      terminal = TERMINATE_SYMBOL;
    } else {
      terminal = t->terminals[i];
    }

    for (int j = 0; j < padding; j++) {
      printf(" ");
    }

    printf("%c", terminal);

    for (int j = 0; j < padding; j++) {
      printf(" ");
    }

    printf("|");
  }

  for (int i = 0; i < t->vars_len; i++) {
    printf("\n");

    printf("    -");
    for (int j = 0; j < t->terminals_len + 1; j++) {
      for (int k = 0; k < totalspace + 2; k++)
        printf("-");
    }

    char var = t->vars[i];
    printf("\n");
    printf("    ");
    printf("%c |", var);

    rhs_hashmap *rhs_hm;
    if (search_ll1_hashmap(t->table, var, &rhs_hm) ==
        HASHMAP_KEY_FOUND_SUCCESS) {
      production_rhs *rhs;

      for (int j = 0; j < t->terminals_len + 1; j++) {
        char terminal;

        if (j == t->terminals_len) {
          terminal = TERMINATE_SYMBOL;
        } else {
          terminal = t->terminals[j];
        }

        if (search_rhs_hashmap(rhs_hm, terminal, &rhs) ==
            HASHMAP_KEY_FOUND_SUCCESS) {
          int rhs_len;

          if (rhs->rhs[0] == EPSILON) {
            rhs_len = 3 + 5;
          } else {
            rhs_len = strlen(rhs->rhs) + 5;
          }

          int pd = (totalspace - rhs_len);

          for (int k = 0; k < pd / 2; k++)
            printf(" ");

          if (rhs->rhs[0] == EPSILON) {
            printf("%c -> eps", var);
          } else {
            printf("%c -> %s", var, rhs->rhs);
          }

          for (int k = 0; k < pd / 2; k++)
            printf(" ");

          if (pd % 2 == 1) {
            printf(" ");
          }

          printf("|");
        } else {
          for (int k = 0; k < totalspace; k++)
            printf(" ");

          printf("|");
        }
      }
    }
  }

  printf("\n");
}

void print_ff_table(ff_table *t) {
  printf("FF Table:\n");
  printf("   Firsts:\n");

  for (int i = 0; i < MAX_PRODS; i++) {
    var_firsts fr = t->firsts[i];

    if (fr.var != '\0' && fr.firsts != NULL) {
      printf("      %c = {", fr.var);

      for (int j = 0; j < fr.firsts_len; j++) {
        first f = fr.firsts[j];

        if (j == fr.firsts_len - 1) {
          if (f.c == EPSILON) {
            printf("epsilon");
          } else {
            printf("%c", f.c);
          }
        } else {
          if (f.c == EPSILON) {
            printf("epsilon,");
          } else {
            printf("%c,", f.c);
          }
        }
      }
      printf("}");
      printf("\n");
    }
  }

  printf("\n");
  printf("   Follows:\n");

  for (int i = 0; i < MAX_PRODS; i++) {
    var_follows fl = t->follows[i];

    if (fl.var != '\0' && fl.follows != NULL) {
      printf("      %c = {", fl.var);

      for (int j = 0; j < fl.follows_len; j++) {
        follow f = fl.follows[j];

        if (j == fl.follows_len - 1) {
          if (f.c == EPSILON) {
            printf("epsilon");
          } else {
            printf("%c", f.c);
          }
        } else {
          if (f.c == EPSILON) {
            printf("epsilon,");
          } else {
            printf("%c,", f.c);
          }
        }
      }
      printf("}");
      printf("\n");
    }
  }
}

void print_ll1_hashmap_node(ll1_hashmap_node *n) {
  printf("      %c: \n", n->key);
  rhs_hashmap *hm = n->data;

  for (int i = 0; i < hm->max; i++) {
    rhs_hashmap_node *curr_node = hm->nodes[i];

    while (curr_node != NULL) {
      printf("         %c: ", curr_node->key);
      production p;
      p.var = curr_node->data->for_var;
      p.first_rhs = curr_node->data;
      p.first_rhs->next = NULL;
      p.len = 1;
      print_production(p, 0);
      printf("\n");
      curr_node = curr_node->next;
    }
  }
}

void print_ll1_hashmap(ll1_hashmap *hm) {
  printf("LL1 Hashmap:\n");
  printf("   Nodes:\n");

  for (int i = 0; i < hm->max; i++) {
    ll1_hashmap_node *curr_node = hm->nodes[i];

    while (curr_node != NULL) {
      print_ll1_hashmap_node(curr_node);
      printf("\n");
      curr_node = curr_node->next;
    }
  }
}

void print_rhs_hashmap_node(rhs_hashmap_node *n) {
  printf("      %c: ", n->key);
  production p;
  p.var = n->data->for_var;
  p.first_rhs = n->data;
  p.first_rhs->next = NULL;
  p.len = 1;
  print_production(p, 0);
}

void print_rhs_hashmap(rhs_hashmap *hm) {
  printf("RHS Hashmap:\n");
  printf("   Nodes:\n");

  for (int i = 0; i < hm->max; i++) {
    rhs_hashmap_node *curr_node = hm->nodes[i];

    while (curr_node != NULL) {
      print_rhs_hashmap_node(curr_node);
      printf("\n");
      curr_node = curr_node->next;
    }
  }
}

void free_ll1_table(ll1_table *t) {
  free_ll1_hashmap(t->table);
  free(t->terminals);
  free(t->vars);
  free(t);
}

void free_ff_table(ff_table *t) {
  for (int i = 0; i < MAX_PRODS; i++) {
    var_firsts fr = t->firsts[i];
    var_follows fl = t->follows[i];

    if (fr.var != '\0' && fr.firsts != NULL) {
      free(fr.firsts);
    }

    if (fl.var != '\0' && fl.follows != NULL) {
      free(fl.follows);
    }
  }

  free(t->firsts);
  free(t->follows);
  free(t);
}

void free_ll1_hashmap(ll1_hashmap *hm) {
  for (int i = 0; i < hm->max; i++)
    free_ll1_hashmap_node(hm->nodes[i]);

  free(hm->nodes);
  free(hm);
}

void free_ll1_hashmap_node(ll1_hashmap_node *n) {
  if (n == NULL)
    return;
  free_ll1_hashmap_node(n->next);
  free_rhs_hashmap(n->data);
  free(n);
}

void free_rhs_hashmap(rhs_hashmap *hm) {
  for (int i = 0; i < hm->max; i++)
    free_rhs_hashmap_node(hm->nodes[i]);

  free(hm->nodes);
  free(hm);
}

void free_rhs_hashmap_node(rhs_hashmap_node *n) {
  if (n == NULL)
    return;
  free_rhs_hashmap_node(n->next);
  free(n);
}
