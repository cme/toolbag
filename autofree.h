/* Autofree */

#ifndef __autofree_h
#define __autofree_h

typedef struct AutoFree AutoFree;

AutoFree *af_new(void);
void af_free(AutoFree*);
void *af_add(AutoFree *, void *);
void *af_alloc(AutoFree *, unsigned size);

#define AUTOFREE AutoFree *__af = af_new()
#define AF_END af_free(__af)
#define AF_ADD(x) af_add(__af, x)
#define AF_ALLOC(x) af_alloc(__af, x)

/*
 * Use case is:
 * {
 *   AutoFree *af = af_new();
 *   char *buffer = af_alloc(af, 1024);
 *   char *str = af_add(strcpy(input));
 *   for (char *s = str; *s; s++)
 *     *s = toupper(*s);
 * ...
 *   af_free(af);
 * }
 */


#endif

