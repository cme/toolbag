/* Simple text pattern matching */
#ifndef __match_h
#define __match_h

/* Simple text pattern matching
 * 'pattern' is matched against 'str', with '*' as the traditional
 * variable-length wildcard. 'pattern' must match the *whole* of
 * 'str'. To find the match anywhere in the string, start and finish
 * the pattern with '*'. For example:
 * match ("hello*", "hello") == 1
 * match ("hello*", "hello there") == 1
 * match ("hello*", "why, hello there") == 0
 * match ("*hello", "why, hello there") == 0
 * match ("*hello*", "why, hello there") == 1
 */
extern int match(const char *pattern, const char *str);

#endif  /* __match_h */
