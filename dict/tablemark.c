/* Tablemark (bench, table, it's pretty much the same thing.
 * Not specifically intended to measure the speeds of different table
 * functions, just approximately compare CPUs.
 *
 */

#include <unistd.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dict.h"

const int max_keys = 250000;
const int min_key_len = 5;
const int max_key_len = 10;
const int step = 128;

char **init_keys (int max)
{
  int i;
  char **keys;
  char *buff;
  buff = malloc (max_key_len + 1);
  keys = malloc (max * sizeof *keys);
  for (i = 0; i < max; i++)
    {
      int len = min_key_len + rand()%(max_key_len-min_key_len);
      int j;
      for (j = 0; j < len; j++)
        buff[j] = 'a' + rand()%('z' - 'a');
      buff[j] = '\0';
      keys[i] = strdup(buff);
    }
  free (buff);
  return keys;
}


double tv_diff (struct timeval *res, struct timeval *x, struct timeval *y) {
  struct timeval r;
  if (!res)
    res = &r;
  res->tv_sec = x->tv_sec - y->tv_sec;
  res->tv_usec = x->tv_usec - y->tv_usec;
  if (res->tv_usec > x->tv_usec)
    /* Underflow in usecs, borrow a second */
    res->tv_sec -= 1;
  return (double)res->tv_sec + (double)res->tv_usec / 1000000;
}

void test_round (int rounds, char **keys, int num_keys)
{
  int i, r;
  Dict *d = dict_new (&strkeyfuncs);
  int count = 0;
  double t;
  struct rusage usage0, usage1;
  DictEntry *de;
  /* Populate dictionary */
  
  for (i = 0; i < num_keys; i++)
    if (rand() & 1)
      dict_set (d, keys[i], keys[rand() % num_keys]);

  getrusage (0, &usage0);
  
  for (r = 0; r < rounds; r++)

    {
      char *s = dict_get (d, keys[rand() % num_keys]);
      count += (s != NULL);
    }

  getrusage (0, &usage1);

  dict_free (d);

  /* Print out line */
  t = tv_diff(NULL, &usage1.ru_utime, &usage0.ru_utime);

  printf ("%d %f\n", num_keys, (double )rounds / t);
  fflush (stdout);
  
}

int main (int argc, char *argv[])
{
  int i, num_keys;
  char **keys;
  int rounds = 1000000;
  if (argc == 2) {
    rounds = atoi (argv[1]);
    fprintf (stderr, "using %d rounds\n", rounds);
  }
  keys = init_keys (max_keys);
  if (0)
    {
      /* Print keys */
      for (i = 0; i < max_keys; i++)
        fprintf (stdout, "%s\n", keys[i]);
    }
  for (num_keys = step; num_keys < max_keys; num_keys += step)
    test_round(rounds, keys, num_keys);
}
