/* Module FILEDIAL.C
   Contains the dialogue necessary for importing a file into a datafile.
*/
#include <allegro.h>
#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"
#include "filedat.h"
#include "fileimpq.h"
#include "fileimpq.ht"
#include "labldata.h"

typedef struct t_typelistwr {
   t_typelist *t;
   int checked;
} t_typelistwr;

typedef struct t_impdial {
   int nt;
   t_typelistwr **tw;
   int nm;
   t_typelistwr **mtw;
   int cdi;
   int idw;       /* ref to window objcet */
   int idc;       /* ref to container objcet */
   int ided;      /* ref to edit box of the name */
   int all;
   int prettyname;
   int *relpath;
   const char *orgfn;
   char **editname;
   int ok;
   int *wx;       /* used to remember the window position */
   int *wy;
} t_impdial;

char const*const*txt;

static int DepthBits2Index(int cd)
{
   switch (cd) {
   case 8:
      return 0;
      break;
   case 15:
      return 1;
      break;
   case 16:
      return 2;
      break;
   case 24:
      return 3;
      break;
   }
   return 4;
}

static int Index2DepthBits(int i)
{
   switch (i) {
   case 0:
      return 8;
      break;
   case 1:
      return 15;
      break;
   case 2:
      return 16;
      break;
   case 3:
      return 24;
      break;
   }
   return 32;
}

static void AddTypeComponents(int n, t_typelistwr **tw)
{
   int i;
   char *s, ts[5];

   for (i = 0; i < n; i++) {
      TypeToString(ts, tw[i]->t->type);
      s = msprintf("%s [%s]", tw[i]->t->desc, ts);
      AddCheck(DOWNLEFT|EQUALWIDTH, s, &tw[i]->checked);
      Release(s);
   }
}

static void SetChecked(t_impdial *di, t_typelistwr **tw, int n, int *ptp, int pn)
{
   int i,j;

   for (j = 0; j < n; j++)
      tw[j]->checked = 0;
   for (i = 0; i < pn; i++) {
      for (j = 0; j < n; j++) {
         if (ptp[i] == tw[j]->t->type) {
            di->tw[j]->checked = 1;
            break;
         }
      }
   }
}

static void PrettyCheck(void *data)
{
   t_impdial *di = data;
   char *ext;

   *di->editname = ResizeMem(char, *di->editname, strlen(di->orgfn)+1);
   strcpy(*di->editname, di->orgfn);
   ext = get_extension(*di->editname);
   if (di->prettyname) {
      strupr(*di->editname);
      if (ext > *di->editname)
         ext[-1] = '_';
   } else {
      if (ext > *di->editname && *ext)
         ext[-1] = 0;
   }
   Refresh(di->ided);
}

static void RadioSelect(void *data)
{
   t_impdial *di = data;

   EmptyContainer(di->idc);
   SelectContainer(di->idc);

   if (di->all) {
      AddTypeComponents(di->nt, di->tw);
   } else {
      AddTypeComponents(di->nm, di->mtw);
   }
   DisplayWin();
}

static void ConfirmImport(void *data)
{
   t_impdial *di = data;
   int i, ch, f = 0, dummy;

   for (i = ch = 0; i < di->nt; i++) {
      if (di->tw[i]->checked) {
         ch++;
         f |= di->tw[i]->t->type == DAT_FONT;
      }
   }
   GetWinInfo(di->idw, di->wx, di->wy, &dummy, &dummy);
   if (ch == 0) {
      Req("", txt[YOU_MUST_CHECK_A_TYPE]);
   } else if (di->cdi && f) {
      Req("", txt[FONT_REQUIRES_BPP_OK]);
   } else {
      di->ok = 1;
      StopProcessEvents();
      CloseWin(NULL);
   }
}


static void RejectImport(void *data)
{
   t_impdial *di = data;

   di->ok = 0;
   StopProcessEvents();
   CloseWin(NULL);
}

/* Queries (conditional) the user for the type(s) of object(s) to import to.
   There are three cases to handle concerning matching of the fileextension:
   - there are multiple object types that matches the extension, then these will be displayed for the user to select among (the user may optionally
     extend the view to show all the alternatives).
   - there is a single object type that matches, then if `always' is
     a) zero, the matching one is returned.
     b) non-zero, then the user will have to select among all existing types.
   - no object type matches, then the user will have to select among all existimg types.
   The parameter `types' will point to an array of integers (caller is responsible for freeing memory) each containg an object type. `nt' points
   to the number of types. If caller want the file to be skipped, the value pointed to by `nt' will be zero and `types' will point to NULL. The value
   pointed to by `objname' is an address to the string that the user prefere to use for the object (caller is responsible for freeing the memory). If
   the user reject the import then the memory pointed to by `objname' will be set to NULL.
   If `curname' is not a NULL-pointer then the user will first be prompted if the existing object is to be updated or replaced. If the user selected
   "update" the integer pointed to by `rep' is set to TRUE, and the locations pointed to by `types' and `objname' are not updated */
extern void QueryImportData(const char *curname, int *rep, const char *fn, int **types, int *nt, char **objname, int *bpp, int *relpath, int always)
{
   int i, j, id;
   char *ext, tmp[10];
   static const char *dummies[SIZE_file_import_query];
   static int winx=-1000, winy, cd = -1, prev_types[32], nprevtp, prevall;
   static int prevsingle, prevprettyname;
   int nr;
   t_typelist *tl;
   t_impdial *di;

   *rep = 0;
   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "file_import_query", &nr);
   if (nr < SIZE_file_import_query) {
      for (i = 0; i < SIZE_file_import_query; i++)
         dummies[i] = "??";
      txt = dummies;
   }
   if (cd == -1)
      cd = bitmap_color_depth(screen);

   if (curname) {
      *rep = Request("", 0, 0, txt[SHALL_THE_EXISTING_NN_BE_UPDATED_YES_NO], curname) == 0;
      if (*rep)
         return;
   }
   di = GetMem0(t_impdial, 1);
   di->wx = &winx;
   di->wy = &winy;
   di->prettyname = prevprettyname;
   di->relpath = relpath;
   di->cdi = DepthBits2Index(cd);
   di->orgfn = fn;
   di->editname = objname;
   PrettyCheck(di);
   ext = get_extension(fn);
   di->nt = GetDataTypeList(ext, &tl);
   di->tw = GetMem0(t_typelistwr*, di->nt);
   di->mtw = GetMem0(t_typelistwr*, di->nt);
   for (i = 0; i <  di->nt; i++) {
      di->tw[i] = GetMem0(t_typelistwr, 1);
      di->tw[i]->t = tl + i;
      if (tl[i].matchext) {
         di->mtw[di->nm++] = di->tw[i];
      }
   }

   if (di->nt) {
      if (di->nm == 1) {
         if (always) {
            SetChecked(di, di->tw, di->nt, &di->mtw[0]->t->type, 1);
            di->all = 1;
         } else {
            di->mtw[0]->checked = 1;
         }
      } else if (di->nm > 1) {
         if (prevsingle) {
         } else {
            di->all = prevall;
            SetChecked(di, di->tw, di->nt, prev_types, nprevtp);
         }
      } else { /* Unknown type, no one matched */
         di->all = 1;
         di->tw[di->nt - 1]->checked = 1; /* select binary as default */
      }
   }

   /* Conditional dialogue */
   if (di->nm == 1 && !always) {
      di->ok = 1;
   } else {
      di->idw = MkDialogue(ADAPTIVE, txt[PROPERTIES_OF_IMPORTED_OBJECT], 0);
      di->ided = AddEditBox(DOWNLEFT, 200, txt[IMPORT_NAME], FPTRSTR, 0, objname);
      MkRadioContainer(DOWNLEFT, &di->cdi, R_HORIZONTAL|EQUALWIDTH);
      for (i = 0; i < 5; i++) {
         sprintf(tmp, " %d ", Index2DepthBits(i));
         AddRadioButton(tmp);
      }
      EndRadioContainer();
      id = AddCheck(RIGHT, txt[PRETTIFY_NAME], &di->prettyname);
      AddHandler(id, PrettyCheck, di);
      AddCheck(DOWNLEFT, txt[RELATIVE_PATH], di->relpath);
      di->idc = StartContainer(DOWNLEFT, ADAPTIVE, txt[IMPORT_TYPE], CT_BORDER);
      if (di->all)
         AddTypeComponents(di->nt, di->tw);
      else
         AddTypeComponents(di->nm, di->mtw);
      JoinTabChain(di->idc);
      EndContainer();
      if (di->nm > 1) {
         id = MkRadioContainer(DOWNLEFT, &di->all, R_HORIZONTAL);
         AddRadioButton(txt[MATCHING_TYPES]);
         AddRadioButton(txt[ALL_TYPES]);
         EndRadioContainer();
         AddHandler(id, RadioSelect, di);
      }
      AddButton(DOWNLEFT, txt[DO_IMPORT], ConfirmImport, di);
      AddButton(RIGHT, txt[SKIP_IMPORT], RejectImport, di);
      if (winx!=-1000)
         SetWindowPosition(winx, winy);
      DisplayWin();
      ProcessEvents();
   }

   if (di->ok) {
      *nt = 0;
      for (i = 0; i < di->nt; i++)
         if (di->tw[i]->checked)
            (*nt)++;
      *types = GetMem(int, *nt);
      for (i = j = 0; i < di->nt; i++)
         if (di->tw[i]->checked)
            (*types)[j++] = di->tw[i]->t->type;
      cd = Index2DepthBits(di->cdi);
      if (j < 32)
         nprevtp = j;
      else
         nprevtp = 32;
      for (i = 0; i < nprevtp; i++)
          prev_types[i] = (*types)[i];
      prevall = di->all;
      prevprettyname = di->prettyname;
   } else {
      Release(*objname);
      *nt = 0;
      *types = NULL;
      *objname = NULL;
   }
   *bpp = cd;
   for (i = 0; i < di->nt; i++)
      Release(di->tw[i]);
   Release(di->tw);
   Release(di->mtw);
   Release(di);
   Release(tl);
}

typedef struct t_dpwd {
   char *pwd;
   int cancel;
   int errors;
   int idpwdc;
   const char *path;
   struct t_diskobj *dobj;
} t_dpwd;

static void CancelPassword(void *data)
{
   t_dpwd *dpwd = data;

   dpwd->cancel = 1;
   StopProcessEvents();
   CloseWin(NULL);
}

static void ConfirmPassword(void *data)
{
   t_dpwd *dpwd = data;

   if (CheckPassword(dpwd->path, dpwd->dobj, dpwd->pwd)) {
      StopProcessEvents();
      CloseWin(NULL);
   } else if (dpwd->errors < 3) {
      Req("", txt[ERROR_IN_PASSWORD_RETRY_CLOSE]);
      dpwd->errors++;
   } else
      CancelPassword(dpwd);
}

static void KeyPressedPwd(void *data)
{
   t_dpwd *dpwd = data;
   int scan, ascii, curpos;

   GetEditData(&scan, &ascii, &curpos);
   if (scan == KEY_ENTER && ascii == '\r')
      GenEvent(ConfirmPassword, dpwd, 0, 0);
}

static void CleanPwdList(void *data)
{
   int *id = data;

   ClearPasswords();
   DeActivate(*id);
}

extern char *InquirePassword(const char *path, struct t_diskobj *dobj, int *save)
{
   static const char *dummies[SIZE_file_import_query];
   char *pwd;
   int id, nr, i, npwd;
   t_dpwd *dpwd;

   dpwd = GetMem0(t_dpwd, 1);
   dpwd->path = path;
   dpwd->dobj = dobj;
   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "file_import_query", &nr);
   if (nr < SIZE_file_import_query) {
      for (i = 0; i < SIZE_file_import_query; i++)
         dummies[i] = "??";
      txt = dummies;
   }
   GetPasswordList(&npwd);
   MkDialogue(ADAPTIVE, "", 0);
   id = AddEditBox(DOWNLEFT, 200, txt[ENTER_PWD], FPTRSTR, 0, &dpwd->pwd);
   AddHandler(id, KeyPressedPwd, dpwd);
   AddCheck(DOWNLEFT, txt[SAVE_PWD_LOCAL], save);
   if (npwd)
      dpwd->idpwdc = AddButton(RIGHT, txt[REMOVE_PREVIOUS_PWDS], CleanPwdList, &dpwd->idpwdc);
   AddButton(DOWNLEFT, txt[OK], ConfirmPassword, dpwd);
   AddButton(RIGHT, txt[CANCEL], CancelPassword, dpwd);
   DisplayWin();
   ProcessEvents();
   if (dpwd->cancel) {
      Release(dpwd->pwd);
      dpwd->pwd = NULL;
   }
   pwd = dpwd->pwd;
   Release(dpwd);
   return pwd;
}
