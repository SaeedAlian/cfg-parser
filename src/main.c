#include "../include/production_table.h"
#include <stdio.h>

int main() {
  prod_table t = new_prod_table();
  add_new_prod(&t, 'S', "AB");
  add_new_prod(&t, 'A', "aA");
  add_new_prod(&t, 'A', "epsilon");
  add_new_prod(&t, 'A', "epsilon");
  add_new_prod(&t, 'A', "epsilon");
  add_new_prod(&t, 'B', "bB");

  printf("%c\n", get_prod(&t, 'A').var);
  printf("%s\n", get_prod(&t, 'A').first_rhs->rhs);

  printf("%c\n", get_prod(&t, 'A').var);
  printf("%s\n", get_prod(&t, 'A').first_rhs->next->rhs);

  printf("%c\n", get_prod(&t, 'B').var);
  printf("%s\n", get_prod(&t, 'B').first_rhs->rhs);

  printf("%i\n", has_epsilon_rhs(&t, 'A'));
  printf("%i\n", has_epsilon_rhs(&t, 'B'));

  free_prod_table(&t);

  return 0;
}
