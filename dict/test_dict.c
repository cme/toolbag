/* Test dictionary type */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dict.h"
#include <unistd.h>

struct test_item {
  char *key;
  char *value;
};

struct test_item test_items[] = {
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

bool fail = false;
int verbose = 0;
int updated = 0;

Dict *test_commands(Dict *d, FILE *in)
{
  extern void dict_rehash_TEST (Dict *d, int size);
  extern void dict_lock_rehash_TEST (int);
  extern void dict_lock_rebalance_TEST (int);

  while (!feof (in))
    {
      char buffer[BUFSIZ];
      char buffer2[BUFSIZ];
      updated = 0;
      /* Get word from input */
      if (isatty (fileno (in)))
        fprintf (stdout, "> ");
      if (fscanf (in, "%s", buffer) != 1)
        break;
      /* Decode commands */
      if (!strcmp (buffer, "set"))
        {
          updated = 1;
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (fscanf (in, "%s", buffer2) != 1)
            break;
          if (dict_has_key (d, buffer))
            free (dict_get (d, buffer));
          dict_set (d, buffer, strdup (buffer2));
        }
      else if (!strcmp (buffer, "insert"))
        {
          updated = 1;
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (fscanf (in, "%s", buffer2) != 1)
            break;
          dict_insert (d, buffer, strdup (buffer2));
        }
      else if (!strcmp (buffer, "get"))
        {
          updated = 1;          /* Possibly implicitly updated by rebalance or rehash */
          if (fscanf (in, "%s", buffer) != 1)
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
          sprintf (fname, "dict_%04d.dot", count++);
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
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (fscanf (in, "%s", buffer2) != 1)
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
          if (fscanf (in, "%s", buffer) != 1)
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
          updated = 1;
          if (fscanf (in, "%s", buffer) != 1)
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
          updated = 1;
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
          updated = 1;
          if (fscanf (in, "%s", buffer) != 1)
            break;
          dict_rehash_TEST (d, atoi (buffer));
        }
      else if (!strcmp (buffer, "lock_rehash"))
        {
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (!strcmp(buffer, "true"))
            dict_lock_rehash_TEST (true);
          else if (!strcmp(buffer, "false"))
            dict_lock_rehash_TEST (false);
          else
            printf ("Syntax: lock_rehash true|false\n");
        }
      else if (!strcmp (buffer, "lock_rebalance"))
        {
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (!strcmp(buffer, "true"))
            dict_lock_rebalance_TEST (true);
          else if (!strcmp(buffer, "false"))
            dict_lock_rebalance_TEST (false);
          else
            printf ("Syntax: lock_rebalance true|false\n");
        }
      else if (!strcmp (buffer, "lock"))
        {
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (!strcmp (buffer, "rehash"))
            dict_lock_rehash_TEST (true);
          else if (!strcmp (buffer, "rebalance"))
            dict_lock_rebalance_TEST (true);
        }
      else if (!strcmp (buffer, "unlock"))
        {
          if (fscanf (in, "%s", buffer) != 1)
            break;
          if (!strcmp (buffer, "rehash"))
            dict_lock_rehash_TEST (false);
          else if (!strcmp (buffer, "rebalance"))
            dict_lock_rebalance_TEST (false);
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
          if (fscanf (in, "%s", buffer) != 1)
            break;
          val = dict_decode (&d, dd, buffer);
          printf ("Decoded value '%s' -> %d\n", buffer, val);
        }
      else if (!strcmp (buffer, "sequence"))
        {
          char c;
          for (c ='A'; c <= 'z'; c++)
            {
              char k[2] = { c, '\0' };
              sprintf(buffer, "The answer is '%c' (code %d)", c, c);
              dict_set (d, k, strdup(buffer));
              if (c == 'Z')
                c = 'a' -1;
            }
        }
      else if (!strcmp (buffer, "test"))
        {
          int i, j;
          char *res;
          /* Populate with test data, checking all the time. */
          updated = 1;
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
      else if (!strcmp (buffer, "include"))
        {
          FILE *in2;
          if (fscanf(in, "%s", buffer) != 1)
            break;
          in2 = fopen(buffer, "r");
          if (!in2)
            {
              fprintf (stderr, "Cannot include file '%s'\n", buffer);
            }
          else
            {
              d = test_commands (d, in2);
              fclose (in2);
            }
        }
      else if (!strcmp (buffer, "n_entries"))
        {
          printf ("n_entries: %u\n", dict_n_entries (d));
        }
      else if (!strcmp (buffer, "allocated_bytes"))
        {
          DictEntry *de;
          unsigned keys_total = 0, values_total = 0, dict_total = 0;
          for (de = dict_first (d); de; de = dict_next(d, de))
            {
              keys_total += strlen(de->key) + 1;
              values_total += strlen(de->value) + 1;
            }
          dict_total = dict_allocated_bytes (d);
          printf ("allocated_bytes: %u (%d keys, %d values, %d structure)\n",
                  keys_total + values_total + dict_total,
                  keys_total, values_total, dict_total);
        }
      else if (!strcmp (buffer, "help"))
        {
          printf ("Commands:\n"
                  "    set <key> <value>\t// store text in dictionary\n"
                  "    insert <key> <value>\t// insert text, assuming key doesn't already exist\n"
                  "    get <key>\t// lookup dictionary, print result\n"
                  "    dump\t// dump dictionary structure\n"
                  "    dot\t// dump structure to file in dot format\n"
                  "    check <key> <value>\t// check that dictionary has key-value pair\n"
                  "    checknull <key> <value>\t// check that key has no value\n"
                  "    delete <key>\t// delete entry associated with a key\n"
                  "    exit\n"
                  "    free\t// free and reallocate dictionary\n"
                  "    list\t// list contents of dictionary\n"
                  "    rehash <n>\t// rehash dictionary with n buckets (must be power of 2)\n"
                  "    decode (one|two|three|*)\t// test decoding\n"
                  "    verbose\n"
                  "    terse\n"
                  "    test\t// populate with test data, check integrity\n"
                  "    include <file>\t// read commands from file\n"
                  "    n_entries \t// show number of entries in dictionary\n"
                  "    allocated_bytes \t// show number of bytes allocated\n"
                  "    sequence \tinsert test data, in sorted order\n"
                  "    lock_rehash <true|false> \tdisable or enable rehashing\n"
                  "    lock_rebalance <true|false> \tdisable or enable tree rebalancing\n"
                  "    lock <rehash|rebalance>\t disable rehashing or rebalancing\n"
                  "    unlock <rehash|rebalance>\t enable rehashing or rebalancing\n");
        }
      else
        {
          printf ("Unknown command '%s'\n", buffer);
        }

      if (verbose && updated)
        {
          fprintf (stdout, "\nDictionary:\n");
          dict_dump (d, stdout, print_str_str);
          fprintf (stdout, "\n");
        }
    }
  return d;
}

int
test (void)
{
  Dict *d = dict_new (NULL);
  DictEntry *de;
  dict_insert_entries (d,
                       "hello", strdup("there"),
                       "a", strdup("b"),
                       NULL);
                       
  d = test_commands (d, stdin);
  if (d)
    {
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
  return test ();
}

