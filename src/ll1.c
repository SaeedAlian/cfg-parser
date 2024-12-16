#include "../include/ll1.h"

int create_parse_tree_with_string(ll1_table *table,
                                  ll1_parse_tree **output_tree, char start_var,
                                  const char *str, int str_len) {
  if (str_len == 0 || str == NULL || table == NULL)
    return STRING_PARSE_ERROR;

  char_stack *char_s = new_char_stack(table->terminals_len * 2);

  if (char_s == NULL)
    return STRING_PARSE_ERROR;

  ll1_parse_node_stack *node_s = new_ll1_parse_node_stack(char_s->max);
  if (node_s == NULL) {
    free_char_stack(char_s);
    return STRING_PARSE_ERROR;
  }

  ll1_parse_tree *tree = new_ll1_parse_tree(start_var, char_s->max);
  if (tree == NULL) {
    free_char_stack(char_s);
    free_ll1_parse_node_stack(node_s);
    return STRING_PARSE_ERROR;
  }

  int i = 0;
  char *curr_char;
  ll1_parse_node *curr_node;
  char *string = (char *)str;

  if (char_stack_push(char_s, &start_var) != 0) {
    free_char_stack(char_s);
    free_ll1_parse_node_stack(node_s);
    free_ll1_parse_tree(tree);
    return STRING_PARSE_ERROR;
  }
  if (ll1_parse_node_stack_push(node_s, tree->root) != 0) {
    free_char_stack(char_s);
    free_ll1_parse_node_stack(node_s);
    free_ll1_parse_tree(tree);
    return STRING_PARSE_ERROR;
  }

  while (!ll1_parse_node_stack_is_empty(node_s) &&
         !char_stack_is_empty(char_s) && i < str_len) {
    if (char_stack_pop(char_s, &curr_char) != 0) {
      free_char_stack(char_s);
      free_ll1_parse_node_stack(node_s);
      free_ll1_parse_tree(tree);
      return STRING_PARSE_ERROR;
    }
    if (ll1_parse_node_stack_pop(node_s, &curr_node) != 0) {
      free_char_stack(char_s);
      free_ll1_parse_node_stack(node_s);
      free_ll1_parse_tree(tree);
      return STRING_PARSE_ERROR;
    }

    if (*curr_char == string[i]) {
      i++;
      continue;
    }

    if (*curr_char != curr_node->c) {
      free_char_stack(char_s);
      free_ll1_parse_node_stack(node_s);
      free_ll1_parse_tree(tree);
      return STRING_PARSE_ERROR;
    }

    rhs_hashmap *rhs_hm;
    if (search_ll1_hashmap(table->table, *curr_char, &rhs_hm) ==
        HASHMAP_KEY_FOUND_SUCCESS) {
      production_rhs *rhs;

      if (search_rhs_hashmap(rhs_hm, string[i], &rhs) ==
          HASHMAP_KEY_FOUND_SUCCESS) {
        int rhs_len = strlen(rhs->rhs);

        if (rhs->rhs[0] == EPSILON) {
          if (ll1_parse_tree_add_child(tree, curr_node, EPSILON,
                                       tree->root->max_children) !=
              PARSE_TREE_ADD_NODE_SUCCESS) {
            free_char_stack(char_s);
            free_ll1_parse_node_stack(node_s);
            free_ll1_parse_tree(tree);
            return STRING_PARSE_ERROR;
          }
          continue;
        }

        for (int i = rhs_len - 1; i >= 0; i--) {
          char *c = &rhs->rhs[i];
          if (char_stack_push(char_s, c) != 0) {
            free_char_stack(char_s);
            free_ll1_parse_node_stack(node_s);
            free_ll1_parse_tree(tree);
            return STRING_PARSE_ERROR;
          }
          if (ll1_parse_tree_add_child(tree, curr_node, *c,
                                       tree->root->max_children) !=
              PARSE_TREE_ADD_NODE_SUCCESS) {
            free_char_stack(char_s);
            free_ll1_parse_node_stack(node_s);
            free_ll1_parse_tree(tree);
            return STRING_PARSE_ERROR;
          }
        }

        for (int i = 0; i < curr_node->children_len; i++) {
          if (ll1_parse_node_stack_push(node_s, curr_node->children[i]) != 0) {
            free_char_stack(char_s);
            free_ll1_parse_node_stack(node_s);
            free_ll1_parse_tree(tree);
            return STRING_PARSE_ERROR;
          }
        }
      } else {
        free_char_stack(char_s);
        free_ll1_parse_node_stack(node_s);
        free_ll1_parse_tree(tree);
        return STRING_PARSE_ERROR;
      }
    } else {
      free_char_stack(char_s);
      free_ll1_parse_node_stack(node_s);
      free_ll1_parse_tree(tree);
      return STRING_PARSE_ERROR;
    }
  }

  if (i < str_len - 1) {
    free_char_stack(char_s);
    free_ll1_parse_node_stack(node_s);
    free_ll1_parse_tree(tree);
    return STRING_PARSE_ERROR;
  }

  *output_tree = tree;
  free_char_stack(char_s);
  free_ll1_parse_node_stack(node_s);
  return STRING_PARSE_SUCCESS;
}

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

ll1_parse_node *new_ll1_parse_node(ll1_parse_node *parent, char val,
                                   int max_children) {
  ll1_parse_node *n = (ll1_parse_node *)malloc(sizeof(ll1_parse_node));

  if (n == NULL)
    return NULL;

  n->parent = parent;
  n->c = val;
  n->max_children = max_children;
  n->children_len = 0;
  n->children =
      (ll1_parse_node **)malloc(sizeof(ll1_parse_node *) * n->max_children);

  if (n->children == NULL) {
    free(n);
    return NULL;
  }

  return n;
}

ll1_parse_tree *new_ll1_parse_tree(char start_var, int max_children) {
  ll1_parse_node *root = new_ll1_parse_node(NULL, start_var, max_children);
  if (root == NULL)
    return NULL;

  ll1_parse_tree *tree = (ll1_parse_tree *)malloc(sizeof(ll1_parse_tree));
  if (tree == NULL) {
    free_ll1_parse_node(root);
    return NULL;
  }

  tree->root = root;
  tree->nodes = 1;

  return tree;
}

int ll1_parse_tree_add_child(ll1_parse_tree *t, ll1_parse_node *node, char val,
                             int max_children) {
  ll1_parse_node *new_node = new_ll1_parse_node(node, val, max_children);

  if (new_node == NULL)
    return PARSE_TREE_ADD_NODE_ERROR;

  if (node->children_len >= max_children - 1) {
    node->max_children *= 2;
    ll1_parse_node **temp = (ll1_parse_node **)realloc(
        node->children, sizeof(ll1_parse_node *) * node->max_children);

    if (temp == NULL)
      return PARSE_TREE_ADD_NODE_ERROR;

    node->children = temp;
  }

  node->children[node->children_len++] = new_node;
  t->nodes++;

  return PARSE_TREE_ADD_NODE_SUCCESS;
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

void print_ll1_parse_tree(ll1_parse_tree *t) {
  if (t->root == NULL || t->nodes < 1) {
    printf("Empty Tree\n");
    return;
  }

  print_ll1_parse_node(t->root, 0);
}

void print_ll1_parse_node(ll1_parse_node *n, int level) {
  int padding_factor = 4;

  for (int i = 0; i < level; i++) {
    for (int j = 0; j < level; j++) {
      if (i == j)
        printf("|-");
    }
  }

  if (n->c == EPSILON) {
    printf("eps");
  } else {
    printf("%c", n->c);
  }

  if (n->children_len == 0) {
    printf(" --- ");
  }

  printf("\n");

  for (int i = 0; i < n->children_len; i++)
    print_ll1_parse_node(n->children[i], level + 1);
}

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

void free_ll1_parse_node(ll1_parse_node *n) {
  if (n == NULL)
    return;

  for (int i = 0; i < n->children_len; i++) {
    free_ll1_parse_node(n->children[i]);
  }

  free(n->children);
  free(n);
}

void free_ll1_parse_tree(ll1_parse_tree *t) {
  free_ll1_parse_node(t->root);
  free(t);
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

ll1_parse_node_queue *new_ll1_parse_node_queue(int max) {
  ll1_parse_node_queue *q =
      (ll1_parse_node_queue *)malloc(sizeof(ll1_parse_node_queue));

  if (q == NULL)
    return NULL;

  q->data = (ll1_parse_node **)malloc(sizeof(ll1_parse_node *) * max);
  q->front = -1;
  q->rear = -1;
  q->max = max;

  if (q->data == NULL) {
    if (q != NULL)
      free(q);

    return NULL;
  }

  return q;
}

void free_ll1_parse_node_queue(ll1_parse_node_queue *q) {
  free(q->data);
  free(q);
}

int ll1_parse_node_queue_is_full(ll1_parse_node_queue *q) {
  return (q->rear + 1) % q->max == q->front;
}

int ll1_parse_node_queue_is_empty(ll1_parse_node_queue *q) {
  return q->front == -1;
}

int ll1_parse_node_queue_increase(ll1_parse_node_queue *q) {
  q->max *= 2;
  ll1_parse_node **temp =
      (ll1_parse_node **)realloc(q->data, sizeof(ll1_parse_node *) * q->max);

  if (temp == NULL)
    return -1;

  int overflow = q->front - q->rear;

  if (overflow > 0) {
    int end = q->max / 2;
    for (int i = 0; i <= overflow; i++) {
      q->data[end++] = q->data[i];
    }
    q->rear = end - 1;
  }

  q->data = temp;
  return 0;
}

ll1_parse_node *ll1_parse_node_queue_front(ll1_parse_node_queue *q) {
  return q->data[q->front];
}

int ll1_parse_node_queue_enqueue(ll1_parse_node_queue *q,
                                 ll1_parse_node *state) {
  if ((q->rear + 1) % q->max == q->front) {
    int res = ll1_parse_node_queue_increase(q);
    if (res != 0)
      return res;
  }

  int new_rear = (q->rear + 1) % q->max;

  if (q->front == -1)
    q->front = (q->front + 1) % q->max;

  q->data[new_rear] = state;
  q->rear = new_rear;

  return 0;
}

int ll1_parse_node_queue_dequeue(ll1_parse_node_queue *q,
                                 ll1_parse_node **state) {
  if (q->front == -1)
    return -1;

  if (state != NULL)
    (*state) = q->data[q->front];

  if (q->front == q->rear) {
    q->front = q->rear = -1;
  } else {
    q->front = (q->front + 1) % q->max;
  }

  return 0;
}

int ll1_parse_node_queue_length(ll1_parse_node_queue *q) {
  if (q->front == -1 && q->rear == -1)
    return 0;

  int len = q->rear - q->front + 1;

  if (len < 0)
    return -len;
  return len;
}

ll1_parse_node_stack *new_ll1_parse_node_stack(int max) {
  ll1_parse_node_stack *s =
      (ll1_parse_node_stack *)malloc(sizeof(ll1_parse_node_stack));
  s->data = (ll1_parse_node **)malloc(sizeof(ll1_parse_node *) * max);
  s->top = -1;
  s->max = max;

  if (s->data == NULL) {
    if (s != NULL)
      free(s);

    return NULL;
  }

  return s;
}

void free_ll1_parse_node_stack(ll1_parse_node_stack *s) {
  free(s->data);
  free(s);
}

int ll1_parse_node_stack_is_full(ll1_parse_node_stack *s) {
  return s->top == s->max - 1;
}
int ll1_parse_node_stack_is_empty(ll1_parse_node_stack *s) {
  return s->top == -1;
}

int ll1_parse_node_stack_increase(ll1_parse_node_stack *s) {
  s->max *= 2;
  ll1_parse_node **temp =
      (ll1_parse_node **)realloc(s->data, sizeof(ll1_parse_node *) * s->max);

  if (temp == NULL)
    return -1;

  s->data = temp;
  return 0;
}

ll1_parse_node *ll1_parse_node_stack_top(ll1_parse_node_stack *s) {
  return s->data[s->top];
}

int ll1_parse_node_stack_push(ll1_parse_node_stack *s, ll1_parse_node *c) {
  int new_top = ++s->top;

  if (new_top > s->max - 1) {
    int res = ll1_parse_node_stack_increase(s);
    if (res != 0)
      return res;
  }

  s->data[new_top] = c;

  return 0;
}

int ll1_parse_node_stack_pop(ll1_parse_node_stack *s, ll1_parse_node **c) {
  if (s->top < 0)
    return -1;

  if (c != NULL) {
    (*c) = s->data[s->top];
  }

  s->top--;
  return 0;
}
