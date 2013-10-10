/* timers.c */
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Timer Timer;
struct Timer {
  char *id;
  struct tms start;
  clock_t total;
  Timer *next;
};


Timer *timers;

void finalise_timers (void)
{
  Timer *t;
  const char *fname = "timers.times";
  FILE *f;
  fprintf (stderr, "=== timers ===\n");
  for (t = timers; t; t = t->next)
    {        
      fprintf (stderr, "Timer '%s': %d\n", t->id, t->total);
      if (t->start.tms_utime != 0)
        {
          fprintf (stderr, "XXX Timer is still running XXX\n");
          abort();
        }
    }
  /* Read in timers */
  f = fopen (fname, "r");
  if (f)
    {
      while (!feof (f))
        {
          char buffer[BUFSIZ], buffer2[BUFSIZ];
          char *c;
          int n;
          fgets (buffer, BUFSIZ, f);
          if (feof (f))
            break;
          /* chomp */
          fgets (buffer2, BUFSIZ, f);
          n = atoi (buffer2);
          for (c = buffer; *c; c++)
            if (*c == '\n' || *c == '\r')
              *c = '\0';
          /* Find it in the timers */
          for (t = timers; t; t = t->next)
            {
              if (!strcmp(t->id, buffer))
                {
                  t->total += n;
                  break;
                }
            }
          /* Add it to our list if it's not there */
          if (!t)
            {
              t = alloca (sizeof *t);
              t->next = timers;
              timers = t;
              t->id = alloca (strlen(buffer)+1);
              strcpy (t->id, buffer);
              t->total = n;
            }
        }
      fclose (f);
      fprintf (stderr, "=== cumulativive ===\n");
      for (t = timers; t; t = t->next)
        fprintf (stderr, "Timer '%s': %d\n", t->id, t->total);
    }
  f = fopen (fname, "w");
  for (t = timers; t; t = t->next)
    fprintf (f, "%s\n%d\n", t->id, t->total);
  fclose (f);
}

void start_timer (Timer *t, char *id)
{
  if (!t->id)
    {
      /* First call to start this timer. */
      t->id = id;

      /* Is this the first timer? */
      if (!timers)
        atexit (finalise_timers);
      t->next = timers;
      timers = t;
    }
  if (t->start.tms_utime != 0)
    {
      fprintf (stderr, "XXX counter '%s' restarted without finish\n", t->id);
      abort();
    }
  times (& t->start);
}

void stop_timer (Timer *t)
{
  struct tms stop;
  times(&stop);
  t->total += stop.tms_utime - t->start.tms_utime;
  t->start.tms_utime = 0;
}

/* XXX header or c&p or #include */
#define BEGIN_TIMER(s) do { static Timer __t; start_timer (&__t, s);
#define END_TIMER stop_timer (&__t); } while (0);
#define NEXT_TIMER(s) END_TIMER BEGIN_TIMER(s)

