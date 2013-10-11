#include "lock_mem.c"
int main (int argc, char *argv[])
{
  int a = 12;
  lock_mem(&a);
  CHECK_LOCKED();
  a = 13;
  CHECK_LOCKED();
}
