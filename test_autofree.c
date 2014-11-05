#include "autofree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  AUTOFREE;
  char *buffer = AF_ALLOC(BUFSIZ);
  sprintf(buffer, "Hello %d", 10);
  fprintf(stdout, "%s\n", AF_ADD(strdup(buffer)));
  AF_END;
}
