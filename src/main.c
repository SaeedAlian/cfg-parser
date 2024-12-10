#include "../include/ll1.h"
#include "../include/production_table.h"
#include <stdio.h>

int main() {
  prod_table *t = new_prod_table();
  add_new_prod(t, 'S', "AB");
  add_new_prod(t, 'A', "CD");
  add_new_prod(t, 'B', "+AB");
  add_new_prod(t, 'B', "epsilon");
  add_new_prod(t, 'C', "i");
  add_new_prod(t, 'C', "(S)");
  add_new_prod(t, 'D', "*CD");
  add_new_prod(t, 'D', "epsilon");

  printf("%c\n", get_prod(t, 'A').var);
  printf("%s\n", get_prod(t, 'A').first_rhs->rhs);

  printf("%c\n", get_prod(t, 'B').var);
  printf("%s\n", get_prod(t, 'B').first_rhs->rhs);

  printf("%i\n", has_epsilon_rhs(t, 'A'));
  printf("%i\n", has_epsilon_rhs(t, 'B'));

  int i;
  char *f = first(t, 'D', &i);

  for (int j = 0; j < i; j++) {
    printf("F %i -> %c\n", j, f[j]);
  }

  free(f);
  free_prod_table(t);

  return 0;
}
