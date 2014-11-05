#include <stdio.h>
#include <stdlib.h>

#include "autofree.h"

typedef struct BlockList BlockList;
struct BlockList {
  void *block;
  BlockList *next;
};

struct AutoFree {
  struct BlockList *l;
};


AutoFree *af_new(void) {
  AutoFree *af = malloc(sizeof *af);
  af->l = NULL;
  return af;
}

void af_free(AutoFree *af) {
  BlockList *l;
  for (l = af->l; l; l = l->next)
    free(l->next);
  free(af);
}

/* Add a block to the autofree list */
void *af_add(AutoFree *af, void *block) {
  BlockList *l = malloc(sizeof *l);
  l->next = af->l;
  l->block = block;
  af->l = l;
  return block;
}

void *af_alloc(AutoFree *af, unsigned size) {
  void *block = calloc(1, size);
  return af_add(af, block);
}
