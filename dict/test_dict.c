/* Test dictionary type */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dict.h"

struct test_item {
  char *key;
  char *value;
} test_items[] = {
  { "hello", "there" },
  { "hum hum hum", "hello!" },
  { "Hello", "THERE!" },
  { "LA LA LA", "thar" },
  { "zoom!", "zum" },
  { "ciddyum5", "cidd-yum-FIVE)" },
  { "EtKuvmiss", "(Et-Kuv-miss)" },
  { "5ghidEkoj", "(FIVE-ghid-Ek-oj)" },
  { "vienHetGea", "(vien-Het-Gea)" },
  { "TakriWidd2", "(Tak-ri-Widd-TWO)" },
  { "noyquipGia", "(noy-quip-Gia)" },
  { "yeFryemuc", "(ye-Fryem-uc)" },
  { "icduAbrutt", "(ic-du-Abr-utt)" },
  { "NeanAbquin", "(Nean-Ab-quin)" },
  { "RoquoHoa", "(Ro-quo-Hoa)" },
  { "ikbejhuj", "(ik-bej-huj)" },
  { "Ajuckbegho", "(Aj-uck-be-gho)" },
  { "ReAnwohaf", "(Re-An-wo-haf)" },
  { "NibFejDez3", "(Nib-Fej-Dez-THREE)" },
  { "RhoidDyg", "(Rhoid-Dyg)" },
  { "ogWuHogh_", "(og-Wu-Hogh-UNDERSCORE)" },
  { "cywidCaypt", "(cy-wid-Caypt)" },
  { "gardyeaw8", "(gard-yeaw-EIGHT)" },
  { "umUcnibPa", "(um-Uc-nib-Pa)" },
  { "vekkegtu", "(vek-keg-tu)" },
  { "ujfeuthal", "(uj-feuth-al)" },
  { "tepmagep5", "(tep-mag-ep-FIVE)" },
  { "LyripkaQui", "(Ly-rip-ka-Qui)" },
  { "ShanBykdav", "(Shan-Byk-dav)" },
  { "ninnyiteuk", "(ninn-yit-euk)" },
  { "NaykViWo", "(Nayk-Vi-Wo)" },
  { "ojnolbIj6", "(oj-nolb-Ij-SIX)" },
  { "GlujMamCu", "(Gluj-Mam-Cu)" },
  { "ugByalye", "(ug-Byal-ye)" },
  { "grurrogEn", "(grurr-og-En)" },
  { "fofidUck", "(fof-id-Uck)" },
  { "mevyimIl", "(mev-yim-Il)" },
  { "EepDagZoag", "(Eep-Dag-Zoag)" },
  { "caweshVad%", "(caw-esh-Vad-PERCENT_SIGN)" },
  { "cagyecJep-", "(cag-yec-Jep-HYPHEN)" },
  { "MyaSwosil", "(Mya-Swos-il)" },
  { "Oadcudyon", "(Oad-cud-yon)" },
  { "thiUnyef", "(thi-Un-yef)" },
  { "Viravboo", "(Vir-av-boo)" },
  { "ixhysyemUf", "(ix-hys-yem-Uf)" },
  { "snemurz", "(snem-urz-BACKSLASH)" },
  { "stazCoj7", "(staz-Coj-SEVEN)" },
  { "disEitya", "(dis-Eit-ya)" },
  { "GajThad1", "(Gaj-Thad-ONE)" },
  { "nooccynRy", "(nooc-cyn-Ry)" },
  { "fachucVeur", "(fac-huc-Veur)" },
  { "WozDinn`", "(Woz-Dinn-GRAVE)" },
  { "rabaynly", "(rab-ayn-ly)" },
  { "RyapuvFi", "(Ryap-uv-Fi)" },
  { NULL, NULL }
};

void print_str_str (FILE * out, const void *k, void *value)
{
  fprintf (out, "'%s': '%s'", (char *)k, (char *)value);
}

int
test (void)
{
  bool fail = false;
  Dict *d = dict_new (NULL);
  int verbose = 0;
  dict_insert_entries (d,
                       "hello", strdup("there"),
                       "a", strdup("b"),
                       NULL);
                       
  while (!feof (stdin))
    {
      char buffer[BUFSIZ];
      char buffer2[BUFSIZ];
      /* Get word from input */
      if (verbose)
        {
          fprintf (stdout, "\nDictionary:\n");
          dict_dump (d, stdout, print_str_str);
          fprintf (stdout, "\n");
        }
      if (!scanf ("%s", buffer))
	break;
      /* Decode commands */
      if (!strcmp (buffer, "set"))
	{
	  if (!scanf ("%s", buffer))
	    break;
	  if (!scanf ("%s", buffer2))
	    break;
          if (dict_has_key (d, buffer))
            free (dict_get (d, buffer));
	  dict_set (d, buffer, strdup (buffer2));
	}
      else if (!strcmp (buffer, "insert"))
	{
	  if (!scanf ("%s", buffer))
	    break;
	  if (!scanf ("%s", buffer2))
	    break;
	  dict_insert (d, buffer, strdup (buffer2));
	}
      else if (!strcmp (buffer, "get"))
	{
	  if (!scanf ("%s", buffer))
	    break;
	  printf ("'%s' => '%s'\n", buffer, (char *) dict_get (d, buffer));
	}
      else if (!strcmp (buffer, "dump"))
	{
	  dict_dump (d, stdout, print_str_str);
	}
      else if (!strcmp (buffer, "verbose"))
        verbose = 1;
      else if (!strcmp (buffer, "terse"))
        verbose = 0;
      else if (!strcmp (buffer, "dot"))
	{
          FILE *out;
          static int count = 1;
          char fname[BUFSIZ];
          sprintf (fname, "dict_%d.dot", count++);
          fprintf (stdout, "Writing '%s'\n", fname);
          out = fopen (fname, "w");
          if (!out)
            {
              fprintf (stderr, "Cannot open '%s' for writing\n",
                       fname);
            }
          else
            {
              dict_dump_dot (d, out, print_str_str);
              fclose (out);
            }
	}
      else if (!strcmp (buffer, "check"))
	{
	  char *res;
	  if (!scanf ("%s", buffer))
	    break;
	  if (!scanf ("%s", buffer2))
	    break;
	  res = dict_get (d, buffer);
	  if (!res || strcmp (buffer2, res))
	    {
              if (res)
                printf ("Check fail: '%s' => '%s', should be '%s'\n",
                        buffer, (char *) dict_get (d, buffer), buffer2);
              else
                printf ("Check fail: '%s' => NULL, should be '%s'\n",
                        buffer, buffer2);
	      fail = true;
	    }
	}
      else if (!strcmp (buffer, "checknull"))
	{
	  char *res;
	  if (!scanf ("%s", buffer))
	    break;
	  res = dict_get (d, buffer);
	  if (res != NULL)
	    {
              printf ("Check fail: '%s' => '%s', should be NULL\n",
                      buffer, (char *) dict_get (d, buffer));
	      fail = true;
	    }
	}
      else if (!strcmp (buffer, "delete"))
	{
	  if (!scanf ("%s", buffer))
	    break;
          if (dict_has_key (d, buffer))
            free (dict_get (d, buffer));
	  dict_delete (d, buffer);
	}
      else if (!strcmp (buffer, "exit") || !strcmp (buffer, "quit"))
	{
	  printf ("Exiting\n");
	  break;
	}
      else if (!strcmp (buffer, "free"))
	{
          DictEntry *de;
          for (de = dict_first (d); de; de = dict_next (d, de))
            free (de->value);
	  dict_free (d);
	  d = dict_new (NULL);
	  printf ("Cleared dictionary\n");
	}
      else if (!strcmp (buffer, "list"))
	{
	  DictEntry *de;
          int count = 0;
	  for (de = dict_first (d); de; de = dict_next (d, de))
	    {
	      printf ("'%s' -> '%s'\n", (char *) de->key, (char *) de->value);
              count++;
	    }
          printf ("count=%d\n", count);
	}
      else if (!strcmp (buffer, "rehash"))
	{
	  extern void dict_rehash_TEST (Dict *d, int size);
	  if (!scanf ("%s", buffer))
	    break;
	  dict_rehash_TEST (d, atoi (buffer));
	}
      else if (!strcmp (buffer, "decode"))
	{
	  DictDecode dd[] = {
	    {"one", 1},
	    {"two", 2},
	    {"three", 3},
	    {NULL, 0}
	  };
	  static Dict *d;
	  int val;
	  if (!scanf ("%s", buffer))
	    break;
	  val = dict_decode (&d, dd, buffer);
	  printf ("Decoded value '%s' -> %d\n", buffer, val);
	}
      else if (!strcmp (buffer, "test"))
        {
          int i, j;
          char *res;
          /* Populate with test data, checking all the time. */
          for (i = 0; test_items[i].key; i++)
            {
              dict_set (d, test_items[i].key, strdup(test_items[i].value));
              for (j = 0; test_items[j].key; j++)
                {
                  res = dict_get (d, test_items[j].key);
                  if (j <= i)
                    {
                      if (res == NULL)
                        {
                          printf ("Test fail: key '%s' should exist, but doesn't.\n",
                                  test_items[j].key);
                          fail = true;
                        }
                      else if (strcmp(res, test_items[j].value))
                        {
                          printf ("Test fail: key '%s' should be '%s' but is '%s'\n",
                                  test_items[j].key, test_items[j].value, res);
                          fail = true;
                        }
                    }
                  else
                    {
                      if (res != NULL)
                        {
                          printf ("Test fail: spurious value for key '%s'\n",
                                   test_items[j].key);
                          fail = true;
                        }
                    }
                }
            }
          if (fail)
            printf ("Insert test failed.\n");
          else
            printf ("Insert test passed.\n");
        }
      else if (!strcmp (buffer, "help"))
        {
          printf ("Commands:\n"
                  "    set <key> <value>\n"
                  "    insert <key> <value>\n"
                  "    get <key>\n"
                  "    dump\n"
                  "    dot // dump structure in dot format\n"
                  "    check <key> <value>\n"
                  "    checknull <key> <value>\n"
                  "    delete <key>\n"
                  "    exit\n"
                  "    free\n"
                  "    list\n"
                  "    rehash <n>\n"
                  "    decode (one|two|three|*)\n"
                  "    verbose\n"
                  "    terse\n"
                  "    test // populate with test data\n");
        }
      else
	{
	  printf ("Unknown command '%s'\n", buffer);
	}
    }

  {
    DictEntry *de;
    for (de = dict_first (d); de; de = dict_next (d, de))
      free (de->value);
    dict_free (d);
  }
  if (fail)
    fprintf (stdout, "FAILED\n");
  else
    fprintf (stdout, "PASSED\n");
  return fail ? 1 : 0;
}

int
main ()
{
  Dict *d;
  d = dict_new (NULL);
  dict_set (d, "Hello", "there");
  dict_set (d, "foo", "bar");
  printf ("Hello -> %s\n", (char *) dict_get (d, "Hello"));
  printf ("foo -> %s\n", (char *) dict_get (d, "foo"));
  printf ("bar -> %s\n", (char *) dict_get (d, "bar"));
  dict_set (d, "bar", "tron");
  dict_dumpf (d, stdout, "%s -> %s");
  printf ("OK\n");
  dict_free (d);
  return test ();
}

