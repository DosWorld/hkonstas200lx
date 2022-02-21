/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  FSEL.C
   Author:  Gilles Kohl
   Started: 31. Jan 95
   Subject: The PAL File selection dialog box and support functions
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* --------------------------------------------------------------------
                         MSDOS stuff
   -------------------------------------------------------------------- */
#if defined _MSC_VER || defined _QC
#include <dos.h>
#include <direct.h>
#define findfirst(a,b,c) _dos_findfirst(a,c,b)
#define findnext(a) _dos_findnext(a)
#define ffblk find_t
#define ff_name name
#else
#include <dir.h>
#endif
#include <io.h>


/* --------------------------------------------------------------------
                         local includes
   -------------------------------------------------------------------- */
#include "pal.h"
#include "palpriv.h"

/* --------------------------------------------------------------------
                         Constant definitions
   -------------------------------------------------------------------- */
/* DIALOG RESOURCE HEADER FILE */

/*[PDE:MyDlg]*/

#define EDFILE 0
#define STDIR 1
#define LBFILES 2
#define LBDIRS 3
#define BNOK 4
#define BNCANCEL 5

/* --------------------------------------------------------------------
                             types
   -------------------------------------------------------------------- */

/* define a context structure to avoid global variables */
typedef struct {
   char OrgDir[80]; /* users original starting directory */
   char CurDir[80]; /* current directory shown by dir list box */
   char DefPat[20]; /* original pattern passed by user */
   DIALOG *pDlg;    /* pointer to dialog */
} FSCTX;

/* --------------------------------------------------------------------
                           Default Dialog
   -------------------------------------------------------------------- */

/*[PDE:FselDlg]*/

DLGITEM FselDlgItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 10,  9,454, 14,   0L, "File to &Open: ", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 12, 28,446, 14, IF_STLFT, "Directory: ", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 11, 44,186,114,   0L, "&Files",    NULL,       IhListBox, sizeof(IHLISTBOX), NULL },
{256, 44,205,114,   0L, "&Directories", NULL,       IhListBox, sizeof(IHLISTBOX), NULL },
{468,  8, 80, 23, IF_RET, "  OK  ",    NULL,       IhButton, sizeof(IHBUTTON), NULL },
{468, 37, 80, 21, IF_ESC, "Cancel",    NULL,       IhButton, sizeof(IHBUTTON), NULL }
};

DIALOG FselDlg = {
554, 173,
0L,
0,
6,
FselDlgItems,
DhStandard,
NULL,
NULL,
0
};

/* --------------------------------------------------------------------
                           functions
   -------------------------------------------------------------------- */

/* show current directories and files, update Path edit item */
static void UpdateFiles(FSCTX *pCtx, char *WildCard)
{
   struct ffblk ffb;
   int i;
   int CurDrive;
   char buf[80];

   /* switch off displaying in the listboxes while we update them */
   LbSetShow(pCtx->pDlg, LBFILES, FALSE);
   LbSetShow(pCtx->pDlg, LBDIRS, FALSE);

   /* update edit item according to pattern */
   SetDlgItem(pCtx->pDlg, EDFILE, EDSI_TXT, WildCard);
   LbClear(pCtx->pDlg, LBDIRS);  /* erase directory listbox */
   LbClear(pCtx->pDlg, LBFILES); /* erase files listbox */

   /* now, add regular files to file listbox */
   for(i = findfirst(WildCard, &ffb, 0); i == 0; i = findnext(&ffb)) {
      LbAddString(pCtx->pDlg, LBFILES, LBPOS_SORT, ffb.ff_name);
   }

   /* add directories to dir listbox */
   for(i = findfirst("*.*", &ffb, FA_DIREC); i == 0; i = findnext(&ffb)) {
      if(!(ffb.ff_attrib & FA_DIREC)) continue; /* skip files */

      /* do not add '.', add '..' only if not at the root */
      if(strcmp(ffb.ff_name, ".") == 0 || (strcmp(ffb.ff_name, "..") == 0
         && strlen(pCtx->CurDir) < 4)) continue;
      LbAddString(pCtx->pDlg, LBDIRS, LBPOS_SORT, ffb.ff_name);
   }

   CurDrive = PalGetDisk();
   /* add drives at the bottom */
   for(i = 0; i <= 26; i++) {
      PalSetDisk(i);
      if(PalGetDisk() == i) {
         sprintf(buf, "[-%c-]", 'A'+i);
         LbAddString(pCtx->pDlg, LBDIRS, LBPOS_LAST, buf);
      }
   }
   PalSetDisk(CurDrive);
   LbSetPos(pCtx->pDlg, LBFILES, 0);
   LbSetPos(pCtx->pDlg, LBDIRS, 0);

   /* switch on displaying in the listboxes again */
   LbSetShow(pCtx->pDlg, LBFILES, TRUE);
   LbSetShow(pCtx->pDlg, LBDIRS, TRUE);
}

/* Change to another drive and directory, update static directory item.
   This function will also optionally save/restore the original dir */
static void GotoPath(FSCTX *pCtx, char *Path, int Save)
{
   int Restore;
   char buf[80];
   int ret;

   /* if Path is NULL, restore original, remember that we did this */
   if(Restore = !Path) Path = pCtx->OrgDir;
   if(Save) PalGetCurDir(-1, pCtx->OrgDir);

   SetErrorHandler(NOE_ERRH);
   ret = access(Path, 0);
   SetErrorHandler(NOE_ERRH);
   if(ret != 0) return;

   /* if we have a drive, change to there */
   if(Path[1] == ':') {
      PalSetDisk(toupper(*Path)-'A');
      Path += 2;
   }
   PalChDir(Path); /* go to this directory */

   /* and retrieve where we ended up - let DOS do the work */
   PalGetCurDir(-1, pCtx->CurDir);
   if(Restore) return; /* only restoring ? Then that's all */

   /* prepare and set static dir item */
   sprintf(buf, "Directory: %s", pCtx->CurDir);
   SetDlgItem(pCtx->pDlg, STDIR, STSI_LBL, buf);
}

/* go to a drive, given by letter, updating the other items */
static void GotoDrive(FSCTX *pCtx, int Drive)
{
   char buf[10];

   sprintf(buf, "%c:\\", Drive);
   GotoPath(pCtx, buf, FALSE);
}

/* for any keystroke in the Files listbox, update path edit item */
static void LbFilesKey(FSCTX *pCtx, WORD Key)
{
   char *s;

   (void)Key;
   s = LbGetString(pCtx->pDlg, LBFILES, LbGetPos(pCtx->pDlg, LBFILES));
   SetDlgItem(pCtx->pDlg, EDFILE, EDSI_TXT, s);
}

static char *GetExt(FSCTX *pCtx, int NeedWild)
{
   static char buf[80];
   char *s = buf;
   char *p;

   GetDlgItem(pCtx->pDlg, EDFILE, EDGI_TXT, s);
   s = strrchr(buf, '\\'); /* search for last backslash */
   if(!s) s = strchr(buf, ':'); /* if not found, try a ':' */
   if(!s) s = buf; /* if not found, try entire buffer */
   else ++s;
   if(NeedWild && !strchr(s, '*') && !strchr(s, '?')) *s = '\0';

   return *s ? s : pCtx->DefPat;
}

static void LbDirsKey(FSCTX *pCtx, WORD Key)
{
   char *s;
   char buf[80];

   s = LbGetString(pCtx->pDlg, LBDIRS, LbGetPos(pCtx->pDlg, LBDIRS));
   switch(Key) {
      case ENTER_KEY:
         if(*s == '[') GotoDrive(pCtx, s[2]);
         else          GotoPath(pCtx, s, FALSE);
         UpdateFiles(pCtx, GetExt(pCtx, TRUE));
         break;
      case NO_KEY:
         break;
      default:
         if(*s == '[') sprintf(buf, "%c:\\", s[2]);
         else          sprintf(buf, "%s\\", s);
         strcat(buf, GetExt(pCtx, TRUE));
         SetDlgItem(pCtx->pDlg, EDFILE, EDSI_TXT, buf);
         break;
   }
}

static int HandleOk(FSCTX *pCtx, char *Dest)
{
   char buf[80];
   char *s;
   int n;

   GetDlgItem(pCtx->pDlg, EDFILE, EDSI_TXT, buf);
   s = strrchr(buf, '\\');
   if(!s) s = strchr(buf, ':');
   if(s) {
      if(*s == ':' || s-buf < 3) ++s;
      *s = '\0';
   }
   if(*buf) GotoPath(pCtx, buf, FALSE);
   else     return FALSE;
   UpdateFiles(pCtx, s = GetExt(pCtx, FALSE));
   SetFocus(pCtx->pDlg, EDFILE);
   if(!strchr(s, '*') && !strchr(s, '?')) {
      strcpy(Dest, pCtx->CurDir);
      if(strlen(Dest) > 3) strcat(Dest, "\\");
      strcat(Dest, s);
      return TRUE;
   }
   return FALSE;
}

int FileSelect(char *Title, char *Path, char *Ext, char *Dest, DIALOG *pDlg)
{
   FSCTX *pCtx;
   WORD Key;
   int Done = FALSE;
   int ret;

   pCtx = malloc(sizeof(*pCtx));
   pCtx->pDlg = pDlg ? pDlg : &FselDlg;

   strcpy(pCtx->DefPat, Ext);

   InitDialog(pCtx->pDlg);
   GotoPath(pCtx, Path, TRUE);
   ShowDialog(pCtx->pDlg, 40, 15, Title);
   UpdateFiles(pCtx, Ext);
   do {
      ret = HandleDialog(pCtx->pDlg, (int *)&Key);
      if(ret == BNCANCEL || ret == DN_CANCEL) {
         ret = DN_CANCEL;
         Done = TRUE;
      }
      if(ret == BNOK || ret == DN_OK || ret == LBFILES) {
         ret = DN_OK;
         Done = HandleOk(pCtx, Dest);
      }
      if(ret > BNCANCEL) Done = TRUE;
      if(Key & 0xff) Key &= 0xff;

      switch(GetFocus(pCtx->pDlg)) {
         case LBFILES: LbFilesKey(pCtx, Key); break;
         case LBDIRS:  LbDirsKey(pCtx, Key);  break;
      }
   } while(!Done);
   CloseDialog(pCtx->pDlg);
   GotoPath(pCtx, NULL, FALSE); /* restore original path and drive */
   free(pCtx);

   return ret;
}

#if TEST
void main(void)
{
   char buf[80];
   int ret;

   *buf = '\0';
   if(!PalInit(1)) FatalExit("Init failed - CGAGRAPH not loaded ?", 1);
   ret = FileSelect(" Open file ", ".", "*.*", buf, NULL);
   PalDeInit(1);
   printf("--> %d, Selected: %s\n", ret, buf);
}
#endif

