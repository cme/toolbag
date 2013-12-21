#ifndef APPEND
/* Append a VALUE to an ARRAY of length LEN.
** It is assumed that the space allocated to the array is sufficient
** to hold 2^n elements, thus the space is reallocated when the
** current length is an exact power of two. The allocated space is
** doubled to retain the 2^n property.
*/

#define APPEND(array,len,value)                                         \
do                                                                      \
  {                                                                     \
    if (len != 0)                                                       \
      {                                                                 \
        /* If we've filled the power-of-two-sized array, realloc */     \
        if (((len) & ((len)-1)) == 0) /* len is 2^n */                  \
          array = realloc((array), 2 * (len) * sizeof *(array));        \
      }                                                                 \
    else                                                                \
      {                                                                 \
        /* Was it already allocated? */                                 \
        if (array)                                                      \
          (array) = realloc ((array), sizeof *(array));                 \
        else                                                            \
          (array) = malloc (sizeof *(array));                           \
      }                                                                 \
    (array)[(len)] = (value);                                           \
    (len) += 1;                                                         \
  }                                                                     \
 while (0)
#endif

