/* ------------------------------------------------------------
 * Dictionary abstract data type.
 */

#ifndef __dict_h
#define __dict_h

#include <stdio.h>
#include <stdbool.h>

typedef struct Dict Dict;

/* Key manipulation functions for use by dictionary type. */
typedef void *(*DictKeyDupFn) (const void *);
typedef void (*DictKeyFreeFn) (const void *);
typedef unsigned (*DictKeyHashFn) (const void *);
typedef int (*DictKeyCmpFn) (const void *, const void *);

typedef struct DictKeyFuncs DictKeyFuncs;
struct DictKeyFuncs
{
  DictKeyCmpFn cmp_fn;
  DictKeyHashFn hash_fn;
  DictKeyDupFn dup_fn;
  DictKeyFreeFn free_fn;
};

/* Key functions to use strings. This is the default if NULL is
   passed to dict_new(). */
extern DictKeyFuncs strkeyfuncs;

/* Key functions to use unique pointers as keys. */
extern DictKeyFuncs ptrkeyfuncs;

/* Use static constant strings as keys: no need to copy or release
   keys. */
extern DictKeyFuncs staticstrkeyfuncs;


/* ------------------------------------------------------------
 * Dictionary methods
 */

/* Create new dictionary. If no (NULL) key function struct is given,
   assume keys are strings. */
extern Dict *dict_new (DictKeyFuncs *);

/* Get element of the dictionary */
extern void *dict_get (Dict *, const void *);

/* Does a dictionary contain a (possibly NULL) entry for a key? */
extern bool dict_has_key (Dict * d, const void *k);

/* Set element in the dictionary */
extern void dict_set (Dict *, const void *, void *);

/* Insert element in the dictionary. Caller guarantees that the key is
   not a duplicate. */
extern void dict_insert (Dict *, const void *, void *);

/* Delete an item from the dictionary */
extern void dict_delete (Dict *, const void *);

/* Free the entire dictionary. */
extern void dict_free (Dict *);

/* Number of entries in the dictionary */
extern unsigned int dict_n_entries (Dict *);

/* Amount of memory allocated to dictionary */
extern unsigned int dict_allocated_bytes (Dict *);

/* Insert multiple entries. For use primarily as a constructor. */
extern void dict_insert_entries (Dict *, /* const void *, void *, */ ...);

/* Set multiple entries. For use as a constructor if it cannot be
   easily guaranteed that all keys are unique. */
extern void dict_set_entries (Dict *, /* const void *, void *, */ ...);


/* ------------------------------------------------------------
 * Iterators
 * Typical use case:
 * for (de = dict_first (d); de; de = dict_next (d, de))
 * {
 *   ...;
 *   if (...)
 *   {
 *     // Early termination.
 *     dict_end (d, de);
 *     break;
 *   }
 * }
 */
typedef struct DictEntry DictEntry;
struct DictEntry
{
  const void *key;
  void *value;
};

extern DictEntry *dict_first (Dict * d);
extern DictEntry *dict_next (Dict * d, DictEntry * de);
extern void dict_end (Dict *d, DictEntry *de);

extern void dict_map (Dict * d, void (*fn) (DictEntry * de, void *cl),
		      void *cl);

/* Find the DictEntry for a given key. NULL if it does not exist.
 * This allows you to:
 *   - Determine if a key exists at the same time as looking up the value
 *   - Modify the value without repeating a lookup.
 * The key must NOT be modified using this method!
 */
extern DictEntry *dict_get_entry (Dict *d, const void *key);


/* ------------------------------------------------------------
 * 'Decode' utility for use in eg. switches.
 */
typedef struct DictDecode DictDecode;
struct DictDecode
{
  const char *key;
  int value;
};
extern int dict_decode (Dict ** d, DictDecode * dd, const char *key);



/* Dump contents and structure of dictionary. */
extern void dict_dump (Dict * d, FILE * out,
		       void (*print) (FILE * out, const void *k,
				      void *value));
extern void dict_dumpf (Dict * d, FILE * out, const char *fmt);

/* Dump dictionary in dot format */
extern void dict_dump_dot (Dict *d, FILE *out,
                           void (*print) (FILE * out, const void *k,
                                          void *value));


#endif

