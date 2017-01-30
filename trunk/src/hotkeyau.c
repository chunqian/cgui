/* Module HOTKEYAU
   Contains functions for the automatic hot-key allocation for labelled
   objects
*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "hotkey.h"
#include "hotkeyau.h"
#include "id.h"

#define LASTLETTER 40
#define GetLegalKey(legalkeys, p) legalkeys[((unsigned char)toupper(p))&0xff] - 1

typedef struct t_label {
   /* An array of flags that tells which letters 'A'...'Z'
      there are in the label. index -> the letters ascii */
   int text[LASTLETTER +1];
   int *curlet;
   /* Packed array of the letters */
   int letters[LASTLETTER+1];
   int id;
   int n;
   int *sol;
   int nlet;
} t_label;


static t_label *labels;
static int nref[LASTLETTER+1];
static int nol, maxnol, blocksize = 100;

/* static int maxmemsize=0x100000; */
static int usedhk[LASTLETTER+1];
static char legalkeys[256];
static char legalletters[LASTLETTER+1];

/* forward declaration: */
static void SetSingleLetterObject(int *usedhk, int j);

static int sortlabels(const void *p1, const void *p2)
{
   return ((t_label *) p1)->nlet - ((t_label *) p2)->nlet;
}

static void TakeHk(t_label *l, int hk)
{
   t_object *b;

   b = GetObject(l->id);
   if (b)
      SetButtonHk(legalletters[hk], b);
}

/* Find all that has not already get a hot-key */
static int FindAutoHK(int *usedhk, t_label *l, int n)
{
   int i;

   if (n > 0) {
      for (i = 0; i <= LASTLETTER; i++) {
         if (l->text[i] && !usedhk[i]) {
            /* The current char in text doesn't exist in used-hk-list, mark it to block for lower levels of recursion */
            if (FindAutoHK(usedhk, l + 1, n - 1)) {
               /* here we come on the way up in the recursion, i.e. all objects below (and all the ones above have found letters */
               TakeHk(l, i);
               return 1;
            } else {
               usedhk[i] = 0;
            }
         }
      }
      /* if we come here there was no solution found, return false at end */
   } else {
      return 1;
   }
   return 0;
}

/* This function "recursively" traverses the serach tree to find hotkeys for
   all labels in the list. The recursion is transformed to iteration, where
   the labels-array serves as stack. To avoid one inner-loop, the flag-array
   must be packed into a letter array, which will be done first. Tests will
   show that a large number of labels and that happens to be equal will show
   that it will take an "infinite" time to find the solution. The main
   approach is to change the window-layout since it really needs to be
   face-lifted, and broken down to modules. (This problem is unfortunately
   exponential) Unfortunately (again) this problem also requires a huge
   amount of memory to use dynamic progemming (at least for those cases
   needing it). The memory requirements are 2^strlen/8 for each level, where
   strlen is the total length of a string with those letters contained in the
   levels above (without duplicates). Althogh this is implemented - but only
   up to a certain level of memory consumption. Those levels that din't get
   any memory will drop the dynamic programming and just uses brute force.
   static int FindAutoHK2(int *usedhk, int nrlab) { int i,j,k,n; t_label *l;
   int cursol,bits[40]={0}; int nlet, mem=0,size; if (nrlab<1) return 1;

   for (i=nlet=0,l=labels; i<nrlab; i++,l++) { n = l->n; for (j=k=0;
   j<=LASTLETTER && n; j++) { if (l->text[j]) { n--; l->letters[k++] = j; if
   (!bits[j]) {      //this is for the dynamic programming bits[j] = 1<<nlet;
   nlet++; } } } //allocate memory for dynamic programming bookings
   l->letters[k] = -1; l->nlet = nlet; l->curlet = l->letters; size =
   sizeof(int)*(( (1<<nlet) >> 5) + 1); if (size+mem < maxmemsize) { if
   (l->sol) Release(l->sol); l->sol = GetMem0(size); mem += size; } else
   // if search space is too large for memory, then it will be return 0;// no
   end to this }

   for (i=cursol=0,l=labels; 1; ) { for (; *l->curlet != -1; l->curlet++) if
   (!usedhk[*l->curlet]) break; // The current letter doesn't exist in the
   used-hk-list if (*l->curlet == -1) { // not found: undo allocation and
   return to previous level if (i==0) return 0; // level 0 faild: no solution
   exists, return to caller i--; l--;           // return one level up in the
   search tree usedhk[*l->curlet++] = 0;  // unbook for future use // part of
   dynamic programming:indicate that this solution failed (skip if no memory)
   l->sol[cursol>>5] = 1 << (cursol&0x1f); cursol &= ~bits[*l->curlet]; } else
   { cursol |= bits[*l->curlet]; if (l->sol[cursol>>5] & (1 << (cursol&0x1f)))
   continue; usedhk[*l->curlet] = 1;   // book up to block for allocation in
   lower levels i++; l++;              // recursive call if (i==nrlab) return
   1;       // that's it } } } This function releases a specific letter from
   the counter of all labels that referes to it and also scratch the letter
   from the letter-array of it If such a label will be found to have only one
   letter left, it must take that one as its hot-key */
static void UnreferenceLetter(int *usedhk, int letter)
{
   int i, nor;

   nor = nref[letter];          /* don't search more than necessary */
   for (i = 0; i < nol && nor; i++) {
      if (labels[i].text[letter]) {
         labels[i].text[letter] = 0;
         nor--;
         if (--labels[i].n == 1) {
            SetSingleLetterObject(usedhk, i);
         }
      }
   }
   nref[letter] = 0;
}

static void SetSingleLetterObject(int *usedhk, int j)
{
   int i;

   for (i = 0; i <= LASTLETTER; i++) {
      if (labels[j].text[i]) {
         TakeHk(labels + j, i);
         nol--;
         nref[i]--;
         labels[j] = labels[nol];
         UnreferenceLetter(usedhk, i);
         usedhk[i] = 1;
         break;
      }
   }
}

/* This function assign hot-keys to those objects that have only one possible
   letter */
static void FindSingleObjects(int *usedhk)
{
   int i;

   for (i = 0; i < nol; i++)    /* find such labels */
      if (labels[i].n == 1)     /* the label has only one possibility, take
                                   it */
         SetSingleLetterObject(usedhk, i);
}

/* This function decrement the counters for each letter in the label */
static void DecrementCounters(t_label * label, int *nref)
{
   int i, n;

   n = label->n;
   for (i = 0; i <= LASTLETTER && n; i++) {
      if (label->text[i]) {
         nref[i]--;
         n--;
      }
   }
}

/* This function allocates hot-keys for letters that occures only in one
   label. The letter occures only in one label, but that label may contain
   other letters. Decrementing the reference counters for these letters may
   lead to new letters going down to 1 (occurence in a single label) so this
   function should be called recursively (return value=found at least one
   such letter) The scratching of one letter from */
static int SetSingleKeys(int *usedhk)
{
   int i, j;
   int found = 0;

   for (i = 0; i <= LASTLETTER; i++) {
      if (nref[i] == 1) {       /* this is a letter that only occures in one label */
         for (j = 0; j < nol; j++) {   /* find that label */
            if (labels[j].text[i]) {
               TakeHk(labels + j, i);
               DecrementCounters(labels + j, nref);
               nol--;
               if (j<nol)
                  labels[j] = labels[nol];
               usedhk[i] = 1;
               found = 1;
            }
         }
      }
   }
   return found;
}

/* Scratches these letters in all labels that has already been occupied by
   labels with explicitly stated hotkeys */
static void ScratchUsedHK(int *usedhk)
{
   int i, j;

   for (i = 0; i <= LASTLETTER; i++) {
      if (usedhk[i]) {
         for (j = 0; j < nol && nref[i]; j++) {
            if (labels[j].text[i]) {
               nref[i]--;
               labels[j].text[i] = 0;
            }
         }
      }
   }
}

/* Adds letters from the string "p" to the array of letters (text) in the
   label "l" if it is not there already. */
static void CopyLabelPart(t_label * l, char *p)
{
   int c;

   while (*p) {
      for (; *p; p++) {
         c = GetLegalKey(legalkeys, *p);
         if (c >= 0) {
            if (l->text[c] == 0) {
               nref[c]++;
               l->text[c]++;
               l->n++;
            }
         }
      }
      p++;
   }
}

/* This function shall be called for each labelled object that want to
   participate in the automatic assignment of hot-keys. This must be done
   before the assigment starts. A new label record will be allocated from the
   dynamic array of labels. The reference to the object (id), the letters
   (text) and the number of unique letters (n) in the label will be set in
   the label record. The number of references to each letter (nref) will be
   maintained. If the object already has got a hot-key, instead the
   indication of used letters will be set */
extern void GetLabel(t_object *b)
{
   t_hotkey *hk;
   int c, haskey = 0;

   for (hk = b->hks; hk; hk = hk->next) {
      c = GetLegalKey(legalkeys, hk->key);
      if (c >= 0 && c <= LASTLETTER)
         usedhk[c] = 1;
      haskey = 1;
   }
   if (b->autohk && !haskey) {
      if (nol >= maxnol) {
         labels = ResizeMem(t_label, labels, maxnol + blocksize);
         memset(labels + maxnol, 0, blocksize * sizeof(t_label));
         maxnol += blocksize;
      }
      labels[nol].id = b->id;
      CopyLabelPart(labels + nol, b->label);
      nol++;
   }
}

/* This function will automatically set the hot-keys for all objects that has
   specified that they want to have automatic (i.e. all selectable
   text-labelled objects which has not explicitly specified hot-keys. */
extern void GenerateAutoHotKeys(t_node *nd)
{
   int org, i;

   if (*legalletters == 0)
      UseHotKeys("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
   nd->ob->hkfunc->GetLabel(nd->ob);

   /* Mark all hot-keys already occupied and the unwished ones */
   ScratchUsedHK(usedhk);
   FindSingleObjects(usedhk);
   while (SetSingleKeys(usedhk));
   org = nol;
   qsort(labels, nol, sizeof(t_label), sortlabels);

   /* if not successful: randomly exclude one label TODO! This may suc */
   while (FindAutoHK(usedhk, labels, nol) == 0 && nol)
      nol--;
/* TODO! When time: Re-do the entire hk-assignment using perfect matching in bipartite graph.
   TODO! Add the possibility to assign off-line (in the internationalization tool). */
   for (i = 0; i < org; i++)
      if (labels[i].sol)
         Release(labels[i].sol);
   Release(labels);
   nol = maxnol = 0;
   labels = NULL;
   memset(nref, 0, (LASTLETTER+1) * sizeof(int));
   memset(usedhk, 0, (LASTLETTER+1) * sizeof(int));
}

/* Application interface: */

extern void UseHotKeys(char *s)
{
   int i;

   memset(legalkeys, 0, 256);
   memmove(legalletters, s, LASTLETTER);
   for (i = 1; *s && i <= LASTLETTER; s++, i++)
      legalkeys[(unsigned char) *s] = i;
}

extern void AutoHotKeys(int mode)
{
   opwin->win->autohk = mode;
}
