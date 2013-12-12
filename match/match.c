/* Simple text pattern matching */

#include "match.h"

int match (const char *pattern, const char *str)
{
  for (;;)
    {
      if (*pattern == '*')
	{
	  const char *p, *s;
	  /* '*' text ::= match the next instance of 'text' */
	  while (*pattern == '*')
	    pattern++;
          p = pattern; s = str;
          for (;;)
            {
              if (*p == '\0' && *s == '\0')
                /* Successfully matched end of string */
                return 1;
              else if (*p == '*')
                {
                  /* End of this pattern word, successfully matched. */
                  pattern = p;
                  str = s;
                  break;
                }
              else if (*s == '\0')
                {
                  /* Ran out of string. No match. */
                  return 0;
                }
              else if (*p != *s)
                {
                  /* Retry match from slightly further on. */
                  s = ++str;
                  p = pattern;
                }
              else
                {
                  /* Successfully matched a character! */
                  s++;
                  p++;
                }
            }
	}
      else
	{
          /* text ::= match text at current position. */
          if (*pattern == *str)
            {
              if (*pattern != '\0')
                {
                  /* Successful character match. */
                  str++;
                  pattern++;
                }
              else
                /* End of pattern, and end of string. */
                return 1;
            }
          else
            return 0;
	}
    }
}

