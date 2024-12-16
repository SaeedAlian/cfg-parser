#include "../include/util.h"

int get_input(const char *prmpt, char *buff, size_t size) {
  if (prmpt != NULL) {
    printf("%s", prmpt);
  }

  if (fgets(buff, size, stdin) == NULL)
    return NO_INPUT;

  int l = strlen(buff);

  if (l == 0)
    return EMPTY_INPUT;

  if (buff[l - 1] != '\n') {
    int c;

    while ((c = getchar() != '\n') && (c != EOF))
      return TOO_LONG;
  }

  buff[l - 1] = '\0';

  return OK;
}
