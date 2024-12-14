#include "../include/ll1.h"

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
        production_rhs_stack_push(s, curr_rhs->next);
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
        production_rhs_stack_pop(s, &curr_rhs);
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
        production_rhs_stack_push(s, curr);
        char_stack_push(matches, match);
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
    production_rhs_stack_pop(s, &top);
    char_stack_pop(matches, &match);

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

