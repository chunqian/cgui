/* Module FILEREG.C
   Contains the functions for registration files (the app. can register some
   userdefined options for certain file types displayed by the file browser.
*/
#include <allegro.h>
#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "filedat.h"
#include "filereg.h"

typedef struct t_registration {
   char *pat;
   void (*Handler)(void *data, char *path);
   void *data;
   char *aname;
   char *icon;
   char *label;
   void (*Viewer)(void *pd, void *vd);
   int type;
} t_registration;

static int nr_regf;
static t_registration *regfiles;

static t_registration *GetRegRecord(const char *ext, int type)
{
   int i;

   if (type) {
      for (i=0; i<nr_regf; i++)
         if (regfiles[i].type == type)
            return regfiles + i;
   } else {
      for (i=0; i<nr_regf; i++)
         if (regfiles[i].type == 0 && stricmp(ext, regfiles[i].pat)==0)
            return regfiles + i;
   }
   return NULL;
}

extern t_actionfun GetRegAction(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->Handler;
   return NULL;
}

extern t_viewfun GetRegViewer(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->Viewer;
   return NULL;
}

extern char *GetRegActionName(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->aname;
   return NULL;
}

extern char *GetRegIcon(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->icon;
   return NULL;
}

extern char *GetRegLabel(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->label;
   return NULL;
}

/* Returns any data pointer associated with the call-back */
extern void *GetRegData(const char *s, int type)
{
   t_registration *r;

   r = GetRegRecord(s, type);
   if (r)
      return r->data;
   return NULL;
}

extern int IsRegistered(const char *s, int type)
{
   return GetRegRecord(s, type) != NULL;
}

static void DestroyARegistation(t_registration *rf)
{
   if (rf->pat)
      Release(rf->pat);
   if (rf->label)
      Release(rf->label);
   if (rf->icon)
      Release(rf->icon);
   if (rf->aname)
      Release(rf->aname);
}

static void FreeRegfiles(void *data)
{
   int i;
   (void)data;
   for (i=0; i<nr_regf; i++)
      DestroyARegistation(regfiles + i);
   Release(regfiles);
}

static int MakeARegistration(t_registration *rf, const char *ext, int type, void (*Handler)(void *data, char *path), void *data, const char *aname,
                             const char *icon, const char *label, void (*Viewer)(void *pd, void *vd))
{
   char *tmp;

   if (icon && *icon) {
      tmp = GetMem(char, strlen(icon)+2);
      *tmp = '#';
      if (*icon=='#')
         strcpy(tmp + 1, icon + 1);
      else
         strcpy(tmp + 1, icon);
      rf->icon = tmp;
   } else {
      rf->icon = NULL;
   }
   rf->type = type;
   if (type == 0)
      rf->pat = MkString(ext);
   else
      rf->pat = NULL;
   if (label)
      rf->label = MkString(label);
   else
      rf->label = NULL;
   rf->Handler = Handler;
   rf->data = data;
   if (Handler && aname)
      rf->aname = MkString(aname);
   else
      rf->aname = NULL;
   rf->Viewer = Viewer;
   return 1;
}

extern int RegisterFileType(const char *ext, void (*Handler)(void *data, char *path), void *data, const char *aname, const char *icon,
                            const char *label, void (*Viewer)(void *privatedata, void *viewdata))
{
   int i, pos = 0, ok = 0, type = 0;

   if (regfiles == NULL)
      HookCguiDeInit(FreeRegfiles, NULL);
   if (ext && *ext) {
      if (*ext == '#') {
         type = StringToType(ext + 1);
         for (i=0; i<nr_regf; i++) {
            if (regfiles[i].type == type) {
               pos = i;
               DestroyARegistation(regfiles + pos);
               break;
            }
         }
      } else {
         for (i=0; i<nr_regf; i++) {
            if (strcmp(ext, regfiles[i].pat)==0) {
               pos = i;
               DestroyARegistation(regfiles + pos);
               break;
            }
         }
      }
      if (type != -1) {
         if (i >= nr_regf) {
            pos = nr_regf;
            nr_regf++;
            regfiles = ResizeMem(t_registration, regfiles, nr_regf);
         }
         ok = MakeARegistration(regfiles + pos, ext, type, Handler, data, aname, icon, label, Viewer);
      }
   }
   return ok;
}
