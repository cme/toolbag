#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define lock lock_mem__lock
#define locks lock_mem__locks

typedef struct lock lock;
struct lock {
  int *p;
  int v;
  lock *next;
};

lock *locks;

void lock_mem(void *p)
{
  lock *nl = malloc (sizeof *nl);
  nl->p = (int *)p;
  nl->v = *(nl->p);
  nl->next = locks;
  locks = nl;
}

#define CHECK_LOCKED() check_locked(__FILE__, __LINE__)
void check_locked (char *file, int line)
{
  lock *l;
  for (l = locks; l; l = l->next)
    {
      if (*(l->p) != l->v)
        {
          fprintf (stderr, "%s:%d\n", file, line);
          fprintf (stderr, "XXX Locked memory with wrong value:\n");
          fprintf (stderr, " *%p = 0x%x (should be 0x%x)\n",
                   l->p, *l->p, l->v);
          assert(*(l->p) == l->v);
        }
    }
}
