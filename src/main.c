#include "../include/grammar.h"

int main() {
  grammar *g = new_grammar("SABCD", "+*i", 'S');
  add_production(g, 'S', "AB");
  add_production(g, 'A', "CD");
  add_production(g, 'B', "+AB");
  add_production(g, 'B', "epsilon");
  add_production(g, 'C', "i");
  add_production(g, 'C', "(S)");
  add_production(g, 'D', "*CD");
  add_production(g, 'D', "epsilon");

  print_grammar(g);

  free_grammar(g);

  return 0;
}
