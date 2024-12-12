#include "../include/grammar.h"
#include "../include/ll1.h"

int main() {
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
  calculate_follows(g, fft);
  print_ff_table(fft);

  free_ff_table(fft);
  free_grammar(g);

  return 0;
}
