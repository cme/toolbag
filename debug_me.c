/* Start up debugger on current process.
 *
 * Usage is something like:
 *
 *   #include "debug_me.h"
 *   ...
 *   int func() {
 *     if (case_number == interesting_case)
 *       debug_me();
 *   }
 *
 * When func() gets to interesting_case, an xterm window with gdb will
 * be fired up, and the process will wait for the debugger to attach.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static volatile int wait;

/* To continue execution:
 *   (gdb) call debug_me_continue()
 */
void debug_me_continue(void) {
  fprintf(stderr, " connected\n");
  wait = 0;
}
void debug_me(void) {
  char pidbuffer[BUFSIZ];
  char pathbuffer[BUFSIZ];
  int forkval;

  wait = 1;

  sprintf(pidbuffer, "%d", getpid());
  sprintf(pathbuffer, "/proc/%d/exe", getpid());

  forkval = fork();
  if (forkval == 0) {
    /* Child. Start xterm. */
    execlp("xterm", "xterm", "-fg", "orange", "-bg", "black",
           "-e", "gdb",
           "-ex", "call debug_me_continue()",
           pathbuffer, pidbuffer, NULL);
    perror("debug_me");
    exit(2);
  } else {
    /* Parent process. */
    fprintf(stderr, "Waiting for debugger...");
    while (wait) {
      /* fprintf(stderr, ".");
         sleep(1); */
    }
    fprintf(stderr, "Continuing...\n");
  }
}

