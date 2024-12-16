#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NO_INPUT -1
#define EMPTY_INPUT 0
#define TOO_LONG -2
#define OK 1

int get_input(const char *prmpt, char *buff, size_t size);

#endif
