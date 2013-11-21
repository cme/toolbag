/* Simple text pattern matching */

#include "match.h"
#include <ctype.h>

int match(const char *pattern, const char *str)
{
  for (;;)
    {
      if (*pattern == '\0')
        {
          if (*str == '\0')
            return 1;
          else
            return 0;
        }
      else if (*pattern == '*')
        {
          while (*pattern == '*')
            pattern++;
          if (!pattern)
            return 1;
          while (*str)
            if (*str == *pattern
                && match(pattern+1, str+1))
              return 1;
            else
              str++;
        }
      else if (*pattern == *str)
        str++, pattern++;
      else
        return 0;
    }
}

