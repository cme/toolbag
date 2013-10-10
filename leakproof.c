/* Leakproof memory allocation.
 * A quick and dirty alternative to having to run through Valgrind.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* To do:
 * flags: short strings can be printed.
 * check end of memory on realloc/free/end.
 * sequential allocation identifiers
 */

#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#ifdef realloc
#undef realloc
#endif

const unsigned tag = 0xa110caed;
const char char_allocated = 0x69;
const char char_freed = 0xf0;

const unsigned check_zone_size = 0x100;
/* #define AGGRESSIVE_CHECK 1 */

#define N_LOG_ENTRIES 10

/* Private-ify identifiers */
#define block leakproof__block
#define blocks leakproof__blocks

#define LIST_APPEND(ROOT, NAME, NODE)                   \
  do {                                                  \
    (NODE)->prev_##NAME = (ROOT)->last_##NAME;          \
    (NODE)->next_##NAME = NULL;                         \
    if ((ROOT)->last_##NAME)                            \
      (ROOT)->last_##NAME->next_##NAME = (NODE);        \
    (ROOT)->last_##NAME = (NODE);                       \
    if (!(ROOT)->first_##NAME)                          \
      (ROOT)->first_##NAME = (NODE);                    \
  } while (0)

typedef struct block block;
struct block
{
  unsigned tag;
  int id;
  /* Allocation location */
  const char *file;
  unsigned int line;
  unsigned size;
  block *next_block;
  block *prev_block;
};

#if (N_LOG_ENTRIES >= 1)

typedef enum block_log_action {
  log_none, log_alloc, log_free, log_realloc_from, log_realloc_to
} block_log_action;

typedef struct block_log block_log;
struct block_log {
  block_log_action action;
  void *ptr;
  int size;
};
static block_log log_entries[N_LOG_ENTRIES];
static int next_log_entry;

static void log_op (block_log_action action, void *ptr, int size)
{
  log_entries[next_log_entry].action = action;
  log_entries[next_log_entry].ptr = ptr;
  log_entries[next_log_entry].size = size;
  next_log_entry++;
  if (next_log_entry >= N_LOG_ENTRIES)
    next_log_entry = 0;
}

void print_block_log (FILE *out)
{
  int i;
  fprintf (out, "=== Allocation log ===\n");
  for (i = 0; i < N_LOG_ENTRIES; i++)
    {
      block_log *l = &log_entries[(i + next_log_entry+1) % N_LOG_ENTRIES];
      switch (l->action)
        {
        case log_none:
          break;
        case log_alloc:
          fprintf (out, "%12s %p+%d\n", "alloc", l->ptr, l->size);
          break;
        case log_realloc_from:
          fprintf (out, "%12s %p+%d\n", "realloc from", l->ptr, l->size);
          break;
        case log_realloc_to:
          fprintf (out, "%12s %p+%d\n", "realloc to", l->ptr, l->size);
          break;
        case log_free:
          fprintf (out, "%12s %p+%d\n", "free", l->ptr, l->size);
          break;
        }
    }
}

#endif  /* N_LOG_ENTRIES >= 1 */

/* Pointer arithmetic */
#define BLOCK_CHECK1(b) ((char *)(b) + sizeof(block))
#define BLOCK_CONTENTS(b) ((char *)(b) + sizeof(block) + check_zone_size)
#define BLOCK_CHECK2(b) ((char *)(b) + sizeof(block) + check_zone_size + (b)->size)
#define BLOCK_FROM_CONTENTS(c) (block *)((char *)(c) - check_zone_size - sizeof (block))

static struct {
  block *first_block;
  block *last_block;
  int next_block;
} blocks;

static void print_block (FILE *out, block *b)
{
  if (!b || !(b+1))
    fprintf (out, "(null)\n");
  else if (b->tag != tag)
    fprintf (out, "(invalid block at %p)\n", b+1);
  else
    fprintf (out, "(%d) %p:%-5d %s:%d\n", b->id, b+1, b->size, b->file, b->line);
}

static void report_blocks (FILE *out)
{
  block *b;
  int count = 0;
  fprintf (out, "Allocated blocks:\n");
  for (b = blocks.first_block; b; b = b->next_block)
    {
      print_block (out, b);
      count++;
    }
  fprintf (out, "%d blocks currently allocated\n", count);
}

static void check_block (block *b)
{
  int i;
  int fail = 0;
  if (b->tag != tag)
    {
      fprintf (stderr, "Invalid block tag: \n");
      print_block (stderr, b);
    }
  for (i = 0; i < check_zone_size; i++)
    {
      if (BLOCK_CHECK1(b)[i] != char_allocated)
        {
          if (!fail)
            {
              fprintf (stderr, "Corrupt check zone in block:\n");
              print_block (stderr, b);
              fail = 1;
            }
          fprintf (stderr, "Address %p (base-%d): %x\n",
                   &(BLOCK_CHECK1(b)[i]), check_zone_size - i,
                   BLOCK_CHECK1(b)[i]);
        }
      if (BLOCK_CHECK2(b)[i] != char_allocated)
        {
          if (!fail)
            {
              fprintf (stderr, "Corrupt check zone in block:\n");
              print_block (stderr, b);
              fail = 1;
            }
          fprintf (stderr, "Address %p (base+%d, or %d past end): %x\n",
                   &(BLOCK_CHECK2(b)[i]), b->size + i,
                   i, BLOCK_CHECK2(b)[i]);
        }
    }
  if (fail)
    {
      print_block_log (stderr);
      exit (1);
    }
}

static void check_blocks (void)
{
  block *b;
  for (b = blocks.first_block; b; b = b->next_block)
    check_block (b);
}

static void leakproof_final (void)
{
  fprintf (stderr, "*** leakproof.c ***\n");
  check_blocks ();
  if (blocks.first_block)
    {
      fprintf (stderr, "****** Memory leak detected ******\n");
      report_blocks (stderr);
    }
}

static int leakproof_initialised;
static void leakproof_init (void)
{
  atexit (leakproof_final);
  leakproof_initialised = 1;
}

void *leakproof_malloc (const char *file, unsigned line, unsigned size)
{
  block *b;
#if (AGGRESSIVE_CHECK)
  check_blocks();
#endif
  if (!leakproof_initialised)
    leakproof_init ();
  b = malloc (sizeof (block) + size + 2 * check_zone_size);
  if (b)
    {
      LIST_APPEND (&blocks, block, b);
      b->id = blocks.next_block++;
      b->tag = tag;
      b->file = file;
      b->line = line;
      b->size = size;
      memset (BLOCK_CHECK1(b), char_allocated, b->size + 2 * check_zone_size);
#if (N_LOG_ENTRIES >= 1)
      log_op (log_alloc, BLOCK_CONTENTS(b), size);
#endif
      return BLOCK_CONTENTS(b);
    }
  else
    {
#if (N_LOG_ENTRIES >= 1)
      log_op (log_alloc, NULL, size);
#endif
      return NULL;
    }
}

void *leakproof_calloc (const char *file, unsigned line,
                        unsigned n, unsigned size)
{
  void *block = leakproof_malloc (file, line, n * size);
  if (block)
    memset (block, '\0', n * size);
  return block;
}

void leakproof_free (void *p)
{
  block *b = BLOCK_FROM_CONTENTS(p);
#if (N_LOG_ENTRIES >= 1)
  log_op (log_free, p, b->size);
#endif
#if (AGGRESSIVE_CHECK)
  check_blocks();
#else
  check_block (b);
#endif
  if (blocks.first_block == b)
    blocks.first_block = b->next_block;
  if (blocks.last_block == b)
    blocks.last_block = b->prev_block;
  if (b->next_block)
    b->next_block->prev_block = b->prev_block;
  if (b->prev_block)
    b->prev_block->next_block = b->next_block;
  memset (b, char_freed, sizeof *b + b->size + 2 * check_zone_size);
  free (b);
}

void *leakproof_realloc (void *p, unsigned size)
{
  block *old_block, *new_block;
  unsigned old_size = 0;
  char *c;
  old_block = BLOCK_FROM_CONTENTS(p);
#if (N_LOG_ENTRIES >= 1)
  log_op (log_realloc_from, old_block, old_block->size);
#endif
#if (AGGRESSIVE_CHECK)
  check_blocks();
#else
  check_block (old_block);
#endif
  old_size = old_block->size;
  new_block = realloc (old_block, sizeof(block) + size + 2 * check_zone_size);
  if (!new_block)
    return NULL;
  if (new_block != old_block)
    {
      if (new_block->prev_block)
        new_block->prev_block->next_block = new_block;
      if (new_block->next_block)
        new_block->next_block->prev_block = new_block;
      if (old_block == blocks.first_block)
        blocks.first_block = new_block;
      if (old_block == blocks.last_block)
        blocks.last_block = new_block;
    }
  new_block->size = size;
  if (old_size < size)
    {
      int i;
      c = BLOCK_CONTENTS(new_block);
      /* Newly exposed bytes in the client-visible space */
      for (i = old_size; i < size; i++)
        c[i] = char_allocated;
    }
  c = BLOCK_CHECK2(new_block);
  /* Zap the check block */
  memset(BLOCK_CHECK2(new_block), char_allocated, check_zone_size);
#if (N_LOG_ENTRIES >= 1)
  log_op (log_realloc_from, new_block, new_block->size);
#endif
  return BLOCK_CONTENTS (new_block);
}

char *leakproof_strdup(const char *file, unsigned line, const char *str)
{
  int len = strlen (str);
  char *s2 = leakproof_malloc (file, line, len+1);
  if (s2)
    strcpy(s2, str);
  return s2;
}

#ifdef malloc
#undef malloc
#endif
#define malloc(n) leakproof_malloc(__FILE__, __LINE__, (n))
#ifdef calloc
#undef calloc
#endif
#define calloc(n,s) leakproof_calloc(__FILE__, __LINE__, (n), (s))
#ifdef realloc
#undef realloc
#endif
#define realloc(p,n) leakproof_realloc((p), (n))
#ifdef free
#undef free
#endif
#define free(p) leakproof_free(p)
#ifdef strdup
#undef strdup
#endif
#define strdup(s) leakproof_strdup(__FILE__, __LINE__, (s))

/* Release identifiers */
#undef block
#undef blocks
