/* $Id: guniq.c 281 2006-08-02 18:44:40Z colin $
 * guniq -- globally unique'ify input, with instance count.
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "dict.h"

static int show_counts = 0;
static int show_dot = 0;

void dot_count (FILE *out, const void *key, void *value)
{
  fprintf (out, "%s: %d", (const char *)key, *(int *)value);
}

int main (int argc, char *argv[])
{
  Dict *lines = dict_new (NULL);
  DictEntry *de;
  int i;

  for (i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i], "-c"))
        show_counts = 1;
      else if (!strcmp(argv[i], "-d"))
        show_dot = 1;
      else
        {
          fprintf (stderr, "Syntax: %s [-c] [-d]\n", argv[0]);
          return EXIT_FAILURE;
        }
    }

  /* Iterate over input lines. */
  while (!feof(stdin))
    {
      char buffer[BUFSIZ];
      
      if (fgets(buffer, BUFSIZ, stdin))
	{
	  char *end;
	  int *data;
	  /* chomp the input */
	  end = buffer;
	  while (*end && *end != '\n') end++;
	  *end = '\0';

          data = dict_get (lines, buffer);
          if (!data)
            {
              fprintf (stdout, "%s\n", buffer);
              data = malloc (sizeof (int));
              *data = 0;
              dict_insert (lines, buffer, data);
            }
          (*data)++;
	}
    }
  if (show_counts)
    {
      /* Iterate over hash and emit counts and strings. */
      for (de = dict_first (lines); de; de = dict_next (lines, de))
        fprintf (stdout, "%10d %s\n",
                 *(int *)de->value, (const char *)de->key);
    }
  if (show_dot)
    {
      dict_dump_dot (lines, stdout, dot_count);
    }

  dict_free (lines);
  return EXIT_SUCCESS;
}
