#include "combo.h"
#include "obedbox.h"
#include "obdrop.h"
#include "id.h"
#include "cgui/mem.h"

struct t_editbox;

struct t_combo_box {
   int n;
   t_dropbox *dd;
   int *sel;
   const char * const *strings;
   struct t_editbox *edb;
};

static void ComboAction(t_combo_box *combo_box)
{
   DropDown(combo_box->dd);
   EditBoxUpdateString(combo_box->edb, combo_box->strings[*combo_box->sel]);
}

extern int AttachComboProperty(int id, int *sel, const char * const *strings, int n)
{
   t_editbox *edb;
   t_combo_box *combo_box;
   int ok;
   combo_box = GetMem0(t_combo_box, 1);
   edb = EditBoxAttachComboProperty(id, ComboAction, combo_box);
   if (edb) {
      combo_box->n = n;
      combo_box->dd = CreateDropDownData(GetObject(id), sel, strings, &combo_box->n);
      combo_box->sel = sel;
      combo_box->strings = strings;
      combo_box->edb = edb;
      ok = 1;
   } else {
      free(combo_box);
      ok = 0;
   }
   return ok;
}

