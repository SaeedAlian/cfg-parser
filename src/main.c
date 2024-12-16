#include "../include/grammar.h"
#include "../include/ll1.h"
#include "../include/util.h"

int main() {
  char str[1024];
  int c;

  c = get_input("Enter string: ", str, sizeof(str));

  if (c == NO_INPUT) {
    printf("No input provided for string\n");
    return -1;
  }

  if (c == TOO_LONG) {
    printf("String is too long\n");
    return -1;
  }

  grammar *g = new_grammar("SABCDI", "+*abcd", 'S');
  add_production(g, 'S', "AB");
  add_production(g, 'A', "CD");
  add_production(g, 'B', "+AB");
  add_production(g, 'B', "epsilon");
  add_production(g, 'C', "I");
  add_production(g, 'C', "(S)");
  add_production(g, 'D', "*CD");
  add_production(g, 'D', "epsilon");
  add_production(g, 'I', "a");
  add_production(g, 'I', "b");
  add_production(g, 'I', "c");
  add_production(g, 'I', "d");

  print_grammar(g);

  ff_table *fft = new_ff_table(g);
  calculate_firsts(g, fft);
  int e = calculate_follows(g, fft);
  if (e == GRAMMAR_IS_NOT_LL1) {
    printf("Grammar is not ll(1)\n");

    free_ff_table(fft);
    free_grammar(g);

    return 1;
  }
  print_ff_table(fft);

  ll1_table *ll1_t = new_ll1_table(g, fft);
  print_ll1_table(ll1_t);

  int str_len = strlen((char *)str);
  ll1_parse_tree *tree;
  int f =
      create_parse_tree_with_string(ll1_t, &tree, g->start_var, str, str_len);

  if (f == STRING_PARSE_SUCCESS) {
    print_ll1_parse_tree(tree);
  } else {
    printf("String cannot be parsed\n");
  }

  free_ll1_parse_tree(tree);
  free_ll1_table(ll1_t);
  free_ff_table(fft);
  free_grammar(g);

  return 0;
}
