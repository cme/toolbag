#include <stdio.h>
#include <stdlib.h>
#include "match.h"

int main(int argc, char *argv[])
{
  char b[BUFSIZ];
  int matches = 0;              /* number of lines matching */
  if (argc == 1)
    {
      fprintf (stderr, "syntax: %s [pattern...]\n", argv[0]);
      return 2;
    }
  while (!feof(stdin))
    {
      int i;

      if (fgets(b, BUFSIZ, stdin))
	{
	  /* chomp */
	  for (i = 0; b[i] && b[i] != '\n'; i++);
	  b[i] = '\0';
	  
	  for (i = 1; i < argc; i++)
	    {
	      if (match(argv[i], b))
		{
                  //		  fprintf(stdout, "%s\n", b);
                  //		  matches++;
                  //		  break;
                  fprintf (stdout, "'%s' =~ '%s'\n", argv[i], b);
		}
	    }
        }
    }

  return matches? EXIT_SUCCESS : EXIT_FAILURE;
}
