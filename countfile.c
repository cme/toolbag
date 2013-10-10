/* Maintain a counter in a file.
** Anticipated use is in debugging eg. compiler optimisations, where
** we want to perform only a certain number of transformations across
** multiple process invocations.
*/

#include <stdio.h>

int countfile_get_count (const char *name)
{
  FILE *in;
  int count;
  in = fopen (name, "r");
  if (!in)
    {
      fprintf (stderr, "Cannot open count file '%s'\n", name);
      exit (1);
    }
  fscanf (in, "%d", &count);
  fclose (in);
  return count;
}

void countfile_set (const char *name, int count)
{
  FILE *out = fopen (name, "w");
  if (!out)
    {
      fprintf (stderr, "Cannot open count file '%s'\n", name);
      exit (1);
    }
  fprintf (out, "%d\n", count);
  fclose (out);
}
