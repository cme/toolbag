toolbag
=======

Colin's bag of handy, undocumented, unreliable tools.

dict
----

A dictionary / table data structure. Dynamically balances between
a top-level hash table and (non-deterministically) balanced binary
trees for buckets.

Rehashing decisions are based on costs of rehashing vs. the difference
in cost of performing historical accesses on the new table vs. the
old table.

(Though profiling shows that the previous implementation which used
linked lists for hash table buckets was quicker as well as smaller)

match
-----

Dead simple wildcard text matching.

leakproof
---------

Track memory allocations and complain about any leaks or writes past
the end. Because sometimes it's quicker to #include "leakproof.c" than
it is to configure Valgrind.

lock_mem
--------

Another memory debugging tool. "Lock" the contents of some memory that
you don't expect to change, and then generate an error when it *does*
change.


