/* Dictionary type
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include "dict.h"


/* Key functions for regular strings as keys. Strings are copied and
   owned by the dictionary */
static unsigned
strhash (const char *c)
{
  unsigned int hash = 0;
  const int bits = sizeof (unsigned) * CHAR_BIT;
  while (*c)
    {
      hash += (*c) + (hash << 3) + (hash >> (bits - 3));
      c++;
    }
  return hash;
}

DictKeyFuncs strkeyfuncs = {
  (DictKeyCmpFn) strcmp,
  (DictKeyHashFn) strhash,
  (DictKeyDupFn) strdup,
  (DictKeyFreeFn) free
};

/* Key functions suitable for use with statically allocated strings
   which must exist for the lifetime of the dictionary */
DictKeyFuncs staticstrkeyfuncs = {
  (DictKeyCmpFn) strcmp,
  (DictKeyHashFn) strhash,
  (DictKeyDupFn) NULL,
  (DictKeyFreeFn) NULL,
};

/* Key functions to use unique pointers as keys. No copying necessary,
   simple comparison and hashing. */
int
ptrcmp (void *a, void *b)
{
  if (a < b)
    return -1;
  else if (a > b)
    return 1;
  else
    return 0;
}

int
ptrhash (void *a)
{
  unsigned *ip = (unsigned *)&a;
  int i;
  unsigned hash = 0;
  /* Assuming the pointer is a round number of integers, fold them
   * together simply by adding. On 32-bit machines, or machines with
   * 64-bit ints and pointers, this should be a no-op.
   */
  for (i = 0; i < sizeof a / sizeof (*ip); i++)
    hash += ip[i];
  return hash;
}

DictKeyFuncs ptrkeyfuncs = {
  (DictKeyCmpFn) ptrcmp,
  (DictKeyHashFn) ptrhash,
  NULL,
  NULL
};


/* ------------------------------------------------------------
 * Dictionary data structures
 */

typedef struct DictNode DictNode;

struct Dict
{
  DictNode **slots;
  unsigned l2_n_slots;
  DictKeyFuncs *keyfuncs;
  int n_entries;
  int rehash_benefit;
};

struct DictNode
{
  DictEntry entry;
  unsigned hash;
  DictNode *children[2];
};

static unsigned
hash_to_index (Dict * d, unsigned hash)
{
  return ((hash + (hash >> d->l2_n_slots))
          & ((1u << d->l2_n_slots) -1));
}

static void
dict_dump_nodes (Dict *d, FILE *out,
                 void (*print) (FILE *out, const void *k, void *value),
                 DictNode *n, int indent, int childno)
{
  int i;
  if (n->children[0])
    dict_dump_nodes (d, out, print, n->children[0], indent + 4, 0);

  for (i = 0; i < indent; i++)
    fputc (' ', out);
  if (childno == 0)
    fprintf (out, ".-> ");
  else if (childno == 1)
    fprintf (out, "'-> ");
  else
    fprintf (out, "|-> ");
  fprintf (out, "hash=0x%x ", n->hash);
  if (print)
    print (out, n->entry.key, n->entry.value);
  else
    fprintf (out, "'%s' => %p", (const char *)n->entry.key,
             n->entry.value);
  fputc ('\n', out);

  if (n->children[1])
    dict_dump_nodes (d, out, print, n->children[1], indent + 4, 1);

}

void
dict_dump (Dict * d, FILE * out,
	   void (*print) (FILE * out, const void *k, void *value))
{
  int i;
  fprintf (out, "Dictionary at %p\n", d);
  for (i = 0; i < (1u << d->l2_n_slots); i++)
    {
      DictNode *n = d->slots[i];
      fprintf (out, "[%d]:\n", i);
      if (n)
        dict_dump_nodes (d, out, print, n, 0, 2);
    }
  fprintf (out, "n_entries=%d, rehash_benefit=%d\n",
           d->n_entries, d->rehash_benefit);
}

static const char *dict_dump_fmt;
static void
print_dict_dump_fmt (FILE *out, const void *k, void *value)
{
  fprintf (out, dict_dump_fmt, k, value);
}

void
dict_dumpf (Dict *d, FILE *out, const char *fmt)
{
  dict_dump_fmt = fmt;
  dict_dump (d, out, print_dict_dump_fmt);
}

static void
dict_dump_dot_node (Dict *d, FILE *out,
                    void (*print) (FILE * out, const void *k, void *value),
                    DictNode *n)
{
  int i;
  /* Print the node */
  fprintf (out, "  \"%p\" [ label = \"", n);
  if (print)
    print (out, n->entry.key, n->entry.value);
  else
    fprintf (out, "%s: %p", (const char *)n->entry.key, n->entry.value);
  fprintf (out, "\"];\n");

  /* Child nodes? */
  for (i = 0; i < 2; i++)
    if (n->children[i])
      {
        dict_dump_dot_node (d, out, print, n->children[i]);
        fprintf (out, "  \"%p\" -> \"%p\";\n", n, n->children[i]);
      }
}


/* Dump dictionary in dot format */
void dict_dump_dot (Dict *d, FILE *out,
                    void (*print) (FILE * out, const void *k, void *value))
{
  int i;
  fprintf (out, "digraph \"dict\" {\n  rankdir=LR;\n");
  /* Print out the table */
  fprintf (out, "  root [ shape=record, label=\"");
  for (i = 0; i < (1u << d->l2_n_slots); i++)
    {
      fprintf (out, "%s<s%d>%d", i?"|":"", i, i);
      if ((i % 8) == 7)
        fprintf (out, "\\\n  ");
    }
  fprintf (out, "\"];\n");
  /* And the nodes */
  for (i = 0; i < (1u << d->l2_n_slots); i++)
    {
      if (d->slots[i])
        {
          fprintf (out, "  \"root\":s%d -> \"%p\"", i, d->slots[i]);
          dict_dump_dot_node (d, out, print, d->slots[i]);
        }
    }
  fprintf (out, "}\n");
}

Dict *
dict_new (DictKeyFuncs * funcs)
{
  Dict *d = malloc (sizeof *d);
  if (funcs)
    d->keyfuncs = funcs;
  else
    d->keyfuncs = &strkeyfuncs;
  d->l2_n_slots = 2;
  d->slots = calloc ((1u << d->l2_n_slots), sizeof *d->slots);
  d->n_entries = 0;
  d->rehash_benefit = 0;
  return d;
}

static DictNode **search (Dict *d, const void *k, unsigned hash, 
                          int *depth)
{
  DictNode **np = &(d->slots[hash_to_index (d, hash)]);
  DictNode *n;
  int heur_size = 0;
  int heur_depth = 0;
  *depth = 0;
  for (;;)
    {
      int cmp;

      n = *np;
      if (!n)
        return np;
      if (n->hash == hash)
        cmp = d->keyfuncs->cmp_fn (k, n->entry.key);
      else
        if (hash < n->hash)
          cmp = -1;
        else
          cmp = 1;
      if (cmp < 0)
        {
          if (n->children[1])
            heur_size = (heur_size << 1) + 1;
          else
            heur_depth ++;
          np = &(n->children[0]);
        }
      else if (cmp > 0)
        {
          if (n->children[0])
            heur_size = (heur_size << 1) + 1;
          else
            heur_depth ++;
          np = &(n->children[1]);
        }
      else
        return np;
      
      (*depth)++;
    }
}

static void
insert_nodes (Dict *d, DictNode *n)
{
  DictNode **np;
  int i;
  DictNode *children[2];
  int depth;
  for (i = 0; i < 2; i++)
    {
      children[i] = n->children[i];
      n->children[i] = NULL;
    }
  np = search (d, n->entry.key, n->hash, &depth);
  *np = n;
  for (i = 0; i < 2; i++)
    if (children[i])
      insert_nodes (d, children[i]);
}

static void
rehash (Dict * d, int size)
{
  int i;
  DictNode **old_slots;
  int n_old_slots;
  assert ((size & (size - 1)) == 0);
  old_slots = d->slots;
  n_old_slots = 1u << d->l2_n_slots;
  d->slots = calloc (size, sizeof *d->slots);
  d->l2_n_slots = ffs (size) - 1;
  for (i = 0; i < n_old_slots; i++)
    if (old_slots[i])
      insert_nodes (d, old_slots[i]);
  free (old_slots);
}

extern void dict_rehash_TEST (Dict *d, int size)
{
  rehash (d, size);
}

/* Expanding the hash table costs approximately O(n.log(n)) in
   time. Expanding the table by doubling its size would, on average,
   half the number of entries in each bucket, reducing the height of
   each tree by approximately 1.
   
   Hence each time we access a node that is not the root node of the
   bucket, we reason that this access may have required one fewer
   node traversals, and count this towards the future benefit of
   rehashing.

   The cost of rehashing is approximated as the number of nodes in
   the table, plus the number of slots in the table, as each of
   these contributes a linear factor to the cost of the rehashing.
*/
#define CHECK_REHASH(d, depth) do {             \
if (depth != 0)                                 \
  {                                             \
    (d)->rehash_benefit++;                      \
    check_rehash ((d), (depth));                \
  }                                             \
 } while (0)

static void
check_rehash (Dict * d, unsigned depth)
{
  if (d->rehash_benefit > d->n_entries + (1u << d->l2_n_slots)
      && d->n_entries * 2 > (1u << d->l2_n_slots))
    {
      /* Profitable to double the size of the table. */
      rehash (d, 2u << d->l2_n_slots);
      d->rehash_benefit = 0;
    }
}

void *
dict_get (Dict * d, const void *k)
{
  unsigned hash = d->keyfuncs->hash_fn (k);
  int depth;
  DictNode **np = search(d, k, hash, &depth);
  void *res;
  if (*np)
    res = (*np)->entry.value;
  else
    res = NULL;
  CHECK_REHASH (d, depth);
  return res;
}

bool
dict_has_key (Dict * d, const void *k)
{
  unsigned hash = d->keyfuncs->hash_fn (k);
  int depth;
  DictNode **np = search (d, k, hash, &depth);
  bool res = (*np != NULL);
  CHECK_REHASH (d, depth);
  return res;
}

void
dict_set (Dict * d, const void *k, void *value)
{
  unsigned hash = d->keyfuncs->hash_fn (k);
  int depth;
  DictNode **np = search (d, k, hash, &depth);
  if (*np)
    (*np)->entry.value = value;
  else
    {
      DictNode *n = malloc (sizeof *n);
      if (d->keyfuncs->dup_fn)
        n->entry.key = d->keyfuncs->dup_fn (k);
      else
        n->entry.key = k;
      n->entry.value = value;
      n->hash = hash;
      n->children[0] = n->children[1] = NULL;
      *np = n;
      d->n_entries++;
    }
  CHECK_REHASH (d, depth);
}

void
dict_insert (Dict * d, const void *k, void *value)
{
  unsigned hash = d->keyfuncs->hash_fn (k);
  int depth;
  DictNode **np = search (d, k, hash, &depth);
  if (*np)
    {
      /* Just set */
      (*np)->entry.value = value;
    }
  else
    {
      DictNode *n = malloc (sizeof *n);
      if (d->keyfuncs->dup_fn)
        n->entry.key = d->keyfuncs->dup_fn (k);
      else
        n->entry.key = k;
      n->entry.value = value;
      n->hash = hash;
      n->children[0] = n->children[1] = NULL;
      *np = n;
      d->n_entries++;
    }
  CHECK_REHASH (d, depth);
}

void
dict_insert_entries (Dict *d, ...)
{
  va_list va;
  const void *key;
  void *value;
  va_start (va, d);
  while ((key = va_arg (va, const void *)))
    {
      value = va_arg (va, void *);
      dict_insert (d, key, value);
    }
  va_end (va);
}

void
dict_set_entries (Dict *d, ...)
{
  va_list va;
  const void *key;
  void *value;
  va_start (va, d);
  while ((key = va_arg (va, const void *)))
    {
      value = va_arg (va, void *);
      dict_set (d, key, value);
    }
  va_end (va);
}

void
dict_delete (Dict * d, const void *k)
{
  DictNode ** np, *n;
  int depth;
  unsigned hash = d->keyfuncs->hash_fn (k);
  np = search (d, k, hash, &depth);
  n = *np;
  if (!n)
    /* not found */
    return;
  if (n->children[0])
    {
      if (n->children[1])
        {
          /* Pick the successor (leftmost child of the right subtree)
             or predecessor (rightmost child of the left subtree) and
             replace this node's contents with that.
          */
          static int i = 0;
          DictNode *repl;
          np = &(n->children[i]);
          i ^= 1;               /* alternate left/right */
          while ((*np)->children[i])
            np = &(*np)->children[i];
          repl = *np;
          /* copy the adjacent node's data to N */
          n->entry = repl->entry;
          n->hash = repl->hash;
          *np = repl->children[i^1];
          free (repl);
          d->n_entries--;
        }
      else
        {
          *np = n->children[0];
	  if (d->keyfuncs->free_fn)
	    d->keyfuncs->free_fn (n->entry.key);
          free (n);
          d->n_entries--;
        }
    }
  else
    {
      *np = n->children[1];
      if (d->keyfuncs->free_fn)
        d->keyfuncs->free_fn (n->entry.key);
      free (n);
      d->n_entries--;
    }
}

void dict_free_nodes (Dict *d, DictNode *n)
{
  int i;
  if (d->keyfuncs->free_fn)
    d->keyfuncs->free_fn (n->entry.key);
  for (i = 0; i < 2; i++)
    if (n->children[i])
      dict_free_nodes (d, n->children[i]);
  free (n);
}

void
dict_free (Dict * d)
{
  int i;
  for (i = 0; i < (1u << d->l2_n_slots); i++)
    if (d->slots[i])
      dict_free_nodes (d, d->slots[i]);
  free (d->slots);
  free (d);
}

/* ------------------------------------------------------------
 * Iterators
 *
 * The buckets are tree-structured, so a simple linear iterator is
 * insufficient. Instead, we'll keep a stack of entries.
 */

typedef struct DictEntryStack DictEntryStack;
struct DictEntryStack
{
  DictEntry entry;
  DictNode *node;
  DictEntryStack *up;
};

DictEntry *
dict_first (Dict * d)
{
  int i;
  for (i = 0; i < (1u << d->l2_n_slots); i++)
    if (d->slots[i])
      {
        DictEntryStack *des = malloc (sizeof *des);
        des->node = d->slots[i];
        des->up = NULL;
        des->entry = des->node->entry;
        return (DictEntry *)des;
      }
  return NULL;
}

DictEntry *
dict_next (Dict *d, DictEntry *de)
{
  DictEntryStack *des = (DictEntryStack *)de;
  if (des->node->children[0])
    {
      if (des->node->children[1])
        {
          /* Two children. Push one onto the stack. */
          DictEntryStack *des2 = malloc (sizeof *des2);
          des2->node = des->node->children[1];
          des2->up = des->up;
          des->up = des2;
        }
      des->node = des->node->children[0];
      des->entry = des->node->entry;
      return (DictEntry *)des;
    }
  else if (des->node->children[1])
    {
      des->node = des->node->children[1];
      des->entry = des->node->entry;
      return (DictEntry *)des;
    }
  else if (des->up)
    {
      DictEntryStack *old = des;
      des = des->up;
      free (old);
      /* Prepare entry */
      des->entry = des->node->entry;
      return (DictEntry *)des;
    }
  else
    {
      /* Stack empty. Next bucket. */
      int i;
      for (i = hash_to_index (d, des->node->hash) + 1;
           i < (1u << d->l2_n_slots);
           i++)
        {
          if (d->slots[i])
            {
              des->node = d->slots[i];
              des->entry = des->node->entry;
              return (DictEntry *)des;
            }
        }
      /* Didn't find any more */
      free (des);
      return NULL;
    }
}

void dict_end (Dict *d, DictEntry *de)
{
  /* Cancel iteration over a dictionary. Just free up the
     DictEntryStack. */
  DictEntryStack *des, *old;
  des = (DictEntryStack *)de;
  while (des)
    {
      old = des;
      des = des->up;
      free (old);
    }
}

void
dict_map (Dict * d, void (*fn) (DictEntry *, void *), void *cl)
{
  DictEntry *e;
  for (e = dict_first (d); e; e = dict_next (d, e))
    fn (e, cl);
}

/* Find the DictEntry for a given key. NULL if it does not exist. */
DictEntry *
dict_get_entry (Dict *d, const void *k)
{
  unsigned hash = d->keyfuncs->hash_fn (k);
  int depth;
  DictNode **np = search (d, k, hash, &depth);
  DictEntry *res = NULL;
  if (*np)
    res = &((*np)->entry);
  else
    res = NULL;
  CHECK_REHASH (d, depth);
  return res;
}


/* Decode strings to integers, initialised from some array. */
int
dict_decode (Dict ** d, DictDecode * dd, const char *key)
{
  DictDecode *res;
  if (!*d)
    {
      int i;
      *d = dict_new (&staticstrkeyfuncs);
      for (i = 0; dd[i].key; i++)
	dict_insert (*d, dd[i].key, &dd[dd[i].value]);
    }
  res = (DictDecode *) dict_get (*d, key);
  if (res)
    return res - dd;
  else
    return -1;
}
