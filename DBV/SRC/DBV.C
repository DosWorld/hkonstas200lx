/* --------------------------------------------------------------------
   Project: DBV 2.0: General Database record viewer
   Module:  DBV.C
   Author:  Harry Konstas
   Started: 13 May 1996
   NOTE:    Compile in Large model -ml !
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <ctype.h>

/* --------------------------------------------------------------------
                         local includes
   -------------------------------------------------------------------- */

#include "pal.h"
#include "dbv.h"
#include "dbvrs.h"

/* --------------------------------------------------------------------
                           Globals
   -------------------------------------------------------------------- */


/* define the function key labels */
char *KeyLabels[10] = {
   "Help", "View", "Note", "Find", "Next",
   "Only", "Subset", "Dump", "Open", NULL
};

char *RecKeys[10] = {
   "Help", "List", "Note", "Find", "Next",
   "\x11", "\x10", "Dump", NULL, NULL
};


char *OKLabels[10] = {
   NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, "Cancel", "O.K."
};

char *NoteLabels[10] = {
   NULL, NULL, "More", "Home", NULL,
   NULL, NULL, NULL, NULL, "O.K."
};



/* --------------------------------------------------------------------
                            Menu structures
   -------------------------------------------------------------------- */

MENUITEM AppsItems[] = {
   { "&Open database    F9", MA_VAL, MENUVAL(F9_KEY) },
   { "&Note             F3", MA_VAL, MENUVAL(F3_KEY) },
   { "&Subset           F7", MA_VAL, MENUVAL(F7_KEY) },
   { "---------------------", MT_SEPARATOR, MENUVAL(0)},
   { "&Dump record      F8", MA_VAL, MENUVAL(F8_KEY) },
   { "E&xtract field data ", MA_VAL, MENUVAL(EXT_OP)},
   { "--------------------", MT_SEPARATOR, MENUVAL(0)},
   { "&Exit               ", MA_VAL, MENUVAL(CMD_QUIT) },
};
MENU AppsMenu = { NULL, MS_PULLDN, 0,0,8,8, AppsItems };

MENUITEM SearchItems[] = {
   { "&Find             F4", MA_VAL, MENUVAL(F4_KEY) },
   { "Find Ne&xt        F5", MA_VAL, MENUVAL(F5_KEY) },
   { "Find On&ly        F6", MA_VAL, MENUVAL(F6_KEY) },
};
MENU SearchMenu = { NULL, MS_PULLDN, 0,0,3,3, SearchItems };

MENUITEM ComputeItems[] = {
   { "&Set criteria        ", MA_VAL, MENUVAL(CRT_OP) },
   { "---------------------", MT_SEPARATOR, MENUVAL(0)},
   { "&Total   (SUM)  value", MA_VAL, MENUVAL(SUM_OP) },
   { "&Average (MEAN) value", MA_VAL, MENUVAL(AVG_OP) },
   { "&Minimum (MIN)  value", MA_VAL, MENUVAL(MIN_OP) },
   { "Ma&ximum (MAX)  value", MA_VAL, MENUVAL(MAX_OP) },
};
MENU ComputeMenu = { NULL, MS_PULLDN, 0,0,6,6, ComputeItems };

MENUITEM AboutItems[] = {
   { "&Help  F1", MA_VAL, MENUVAL(F1_KEY)  },
   { "&About   ", MA_VAL, MENUVAL(CMD_ABOUT)},
};
MENU AboutMenu = { NULL, MS_PULLDN, 0,0,2,2, AboutItems };

/* the menu bar  */
MENUITEM BarItems[] = {
   { "&Database",     MA_MNU, &AppsMenu   },
   { "&Search",       MA_MNU, &SearchMenu },
   { "&Compute",      MA_MNU, &ComputeMenu},
   { "&Quit",         MA_VAL, MENUVAL(CMD_QUIT) },
   { "&About",        MA_MNU, &AboutMenu },
};
MENU BarMenu = { NULL, MS_HORIZ|MS_TOPLVL, 0,0,5,5, BarItems };

DBVAR  *pDBVar;  /* global variables structure */


/* --------------------------------------------------------------------
                                 Initialize
   -------------------------------------------------------------------- */

void *Initialize(void)
{

   if(!PalInit(1)) FatalExit("Runs only on HP100/200LX...", 1);

   /* allocate storage for variables */
   if(!(pDBVar = malloc(sizeof(DBVAR))))
      Error("\a\nINIT: Out of memory.\n");

   pDBVar->Found1=0;
   pDBVar->Found2=0;
   pDBVar->String1[0] = 0;
   pDBVar->String2[0] = 0;
   pDBVar->CritOper = NOP_CRIT;
   pDBVar->Fpos = 0;
   pDBVar->CritData[0]=0;

   LoadConfig(pDBVar);

   ShowTopTime(title, TRUE);
   ShowFKeys(KeyLabels);

   SelectFont(MEDIUM_FONT);
   SetRule(FORCE_RULE);
   AnnuncPos(AN_LEFT);   /* place annunciators left */

}


/* --------------------------------------------------------------------
                               Interact
   -------------------------------------------------------------------- */

DBDESC *Interact(DBDESC *pDD, DBVAR *pDBVar, int start)
{
   PALWIN *pWin;
   WORD Key, key;
   int index, select = 0, page = 1, rem;
   int r, f, errcode;

   char buffer[80];
   char *pFname;
   void *pData;
   BYTE *pNotes;
   BYTE *pScan;

   index = start;

   do {

      if(pDD->pCPAG) {
      sprintf(buffer,"%-24s %20s (%d/%d) Page:%d", pDD->FileName,
              pDD->pVDEF->Name, index+1+select, pDD->NumDataRec, page);
      }

      else {
         sprintf(buffer,"%-24s %20s (%d/%d)", pDD->FileName,
              pDD->pVDEF->Name, index+1+select, pDD->NumDataRec);
      }

      ShowTopTime(buffer, TRUE);
      Key = GetKey();

      if(Key & 0xff) Key &= 0xff;

      if(Key == MENU_KEY || Key == AF10_KEY) {
         Key = HandleMenu(&BarMenu, 0, 10);
      }

      switch(Key) {

         case F2_KEY:
         case ENTER_KEY:
            RecordView(pDD, pDBVar, index+select);
            ListView(pDD, REPAINT, index);
            ListView(pDD, SELECT, select);
            break;

         case DOWN_KEY:
            ListView(pDD, UNSELECT, select);
            select++;

            if(select+index > pDD->NumDataRec-1) {
               select--;
               if(pDBVar->BeepFlag) printf("\a");
               ListView(pDD, SELECT, select);
               break;
            }

            if(select>12) {
               select=12;
               index++;

               if(index < pDD->NumDataRec-1) {
                  ListView(pDD, REFRESH, index);
               }

               else {
                  index--;
                  if(pDBVar->BeepFlag) printf("\a");
               }

               ListView(pDD, SELECT, select);
               break;
            }

            ListView(pDD, SELECT, select);
            break;


         case UP_KEY:
            ListView(pDD, UNSELECT, select);
            select--;

            if(select==-1) {
               select=0;
               index--;

               if(index==-1) {
                  index=0;
                  if(pDBVar->BeepFlag) printf("\a");
               }
               else ListView(pDD, REFRESH, index);

               ListView(pDD, SELECT, select);
               break;
            }

            ListView(pDD, SELECT, select);
            break;


         case HOME_KEY:
            ListView(pDD, UNSELECT, select);
            ListView(pDD, REFRESH, 0);
            ListView(pDD, SELECT, 0);
            index =0; select =0;
            break;


         case END_KEY:
            if(pDD->NumDataRec>13) {
               index = pDD->NumDataRec-13;
               select=12;
            }
            else select=pDD->NumDataRec-1;

            ListView(pDD, REPAINT, index);
            ListView(pDD, SELECT, select);
            break;


         case PGDN_KEY:
            ListView(pDD, UNSELECT, select);
            select+=12;

            if(select+index > pDD->NumDataRec-1) {
               select-=12;

               if(pDD->NumDataRec>13) {
                  index = pDD->NumDataRec-13;
                  select=12;
               }

               ListView(pDD, REFRESH, index);
               if(pDBVar->BeepFlag) printf("\a");
               ListView(pDD, SELECT, select);
               break;
            }

            if(select>12) {
               select=12;
               index+=12;

               if(index < pDD->NumDataRec-1) {
                  ListView(pDD, REFRESH, index);
               }
               else {
                  index-=12;
                  if(pDBVar->BeepFlag) printf("\a");
               }

               if(select+index > pDD->NumDataRec-1) {
                  select=pDD->NumDataRec-index-1;
                  ListView(pDD, REPAINT, index);
               }

               ListView(pDD, SELECT, select);
               break;
            }

            ListView(pDD, SELECT, select);
            break;


         case PGUP_KEY:
            ListView(pDD, UNSELECT, select);
            select-=12;

            if(select<0) {
               select=0;
               index-=12;

               if(index<0) {
                  index=0;
                  ListView(pDD, REFRESH, index);
                  if(pDBVar->BeepFlag) printf("\a");
               }
               else ListView(pDD, REFRESH, index);

               ListView(pDD, SELECT, select);
               break;
            }

            ListView(pDD, SELECT, select);
            break;

         case CMD_ABOUT:
            About();
            break;

         case CMD_QUIT:
            return pDD;             /* quit */

         /* computing operations */
         case SUM_OP:
            Compute(pDD,SUM_OP);
            break;

         case AVG_OP:
            Compute(pDD,AVG_OP);
            break;

         case MIN_OP:
            Compute(pDD,MIN_OP);
            break;

         case MAX_OP:
            Compute(pDD,MAX_OP);
            break;

         case EXT_OP:
            Compute(pDD,EXT_OP);
            break;

         case CRT_OP:
            Criteria(pDD);
            break;

         case F1_KEY:
            Help();
            break;

         case F3_KEY:               /* full note view */

            pData = ReadRecord(pDD, TYPE_DATA, index+select);

            /* read notes */
            pNotes = NULL;
            for(f=0;f<pDD->NumFields;f++) {
               if(pDD->pFDEF[f]->FieldType == NOTE_FIELD)
                  pNotes = GetField(pDD,f,pData);
            }

            if(!pNotes) break;   /* no notes */
            pScan = pNotes;

            ShowTopLine("Full screen note", NULL);
            ShowFKeys(NoteLabels);

            SetRule(FORCE_RULE);
            SetColor(WHITE_COLOR);
            Rectangle(0,10,638,188,SOLID_FILL);
            SetColor(BLACK_COLOR);

            rem=0;
            key=F3_KEY;

            do {
               if(key==HOME_KEY || key==F4_KEY) {
                  pScan = pNotes;
                  key=F3_KEY;
                  rem=0;
               }

               if((key==F3_KEY) && (rem!=-1))  {
                  pScan += rem;
                  rem = ShowNote(0,15,635,178, pScan);
               }

               key = GetKey();
               if(key & 0xff) key &= 0xff;
               if(key==ESC_KEY) break;
            } while(key!=F10_KEY);

            free(pNotes);

            ShowTopTime(title, TRUE);
            ShowFKeys(KeyLabels);
            SetRule(FORCE_RULE);

            ListView(pDD, REPAINT, index);
            ListView(pDD, SELECT, select);
            break;


         case F4_KEY:
            r=FindString(pDD, pDBVar, NORMAL);

            if(r!=-1) {
               index = r; select=0;
               /* RecordView(pDD, pDBVar, index); */
               ListView(pDD, REPAINT, index);
               ListView(pDD, SELECT, select);
            }
            break;


         case F5_KEY:
            if(strlen(pDBVar->String1)<2) {
               if(pDBVar->BeepFlag) printf("\a");
               break;
            }

            if(index==pDD->NumDataRec) break;

            pWin=OpenWin(WS_HP200, 140, 50, 470, 150, "Searching");
            WinText(pWin, 65,25,  "   Please wait...");
            WinText(pWin, 65,40,  "   <ESC aborts>");

            SetLightSleep(SLEEP_OFF);
            r=SearchDatabase(pDD, index+1, pDD->NumDataRec, pDBVar->String1,
                  pDBVar->String2, pDBVar->Logic, pDBVar->IncNotes);

            SetLightSleep(SLEEP_ON);
            CloseWin(pWin);

            if(r!=-1) {
               index = r;  select=0;
               /* RecordView(pDD,pDBVar, index); */
               ListView(pDD, REPAINT, index);
               ListView(pDD, SELECT, select);
            }

            else {
               if(pDBVar->BeepFlag) printf("\a");
               MsgBox("ATTENTION!","No more matches.",NULL,"OK");
            }

            break;


         case F6_KEY:
            r=FindString(pDD, pDBVar, FIND_ALL);

            if(r!=-1) {
               index = r; select=0;
               /* RecordView(pDD, pDBVar, index); */
               ListView(pDD, REPAINT, index);
               ListView(pDD, SELECT, select);
            }
            break;


         case F7_KEY:
            SelectViewPoint(pDD);
            index=0; page=1; select=0;
            ListView(pDD, REPAINT, index);
            ListView(pDD, SELECT, select);
            break;


         case F8_KEY:
            if(!DumpRecord(pDD, pDBVar, index+select)) {
               MsgWin("Message","Record saved in: %s",pDBVar->DumpFile);
               GetKey();
               MsgWin("Message",NULL);
            }
            else
               if(pDBVar->BeepFlag) printf("\a");

            break;


         case F9_KEY:
            if(pFname = SelectDatabase(pDBVar->Path)) {
               index=0; page=1;
               CloseDatabase(pDD); /* close previous database */

               /* open database */
               pDD=OpenDatabase(pFname,&errcode);   /* open database  */
               free(pFname);


               if(!pDD) {
                  PalDeInit(1);
                  printf("\a\nError #%d opening database file.\n",
                          errcode);
                  exit(1);
               }

               ListView(pDD, REPAINT, 0);
               ListView(pDD, SELECT, 0);
               index=0; select=0;
            }

      }


      /* speed locate */
      if(Key>31 && Key<167) {
         r = SpeedLocate(pDD, Key, index, select);
         if(r!=-1) {
            index=r; select=0;
         }
         else r=0;

         ListView(pDD, REPAINT, r);
         ListView(pDD, SELECT, 0);

      }

   } while(Key != ESC_KEY);

   return pDD;
}


/* -------------------------------------------------------------------------
                                 M A I N
   ------------------------------------------------------------------------- */

int main (int argc, char * argv[])
{

   PALWIN *pWin;
   DBDESC *pDD;

   char *FileName[80];
   char *pFname;
   int   r,errcode;

   Initialize();

   /* get a database filename */
   if(argc>1) strcpy(FileName, argv[1]);
   else {
      if(!(pFname = SelectDatabase(pDBVar->Path))) {
         PalDeInit(1);
         exit(0);
      }

      strcpy(FileName, pFname);
      free(pFname);

   }

   /* open database */
   MsgWin("Please wait...","Loading database");
   pDD=OpenDatabase(FileName, &errcode);   /* open database  */
   MsgWin("Attention:",NULL);

   if(!pDD) {
      PalDeInit(1);
      printf("\a\nError #%d opening database file.\n",
             errcode);

      if(errcode==NO_LOOKUP_TABLE)
         printf("Database file not closed?\n");
      exit(-1);
   }

   /* process search string - if any. */
   r=0;
   if(argc>2) {
      strcpy(pDBVar->String1, argv[2]);
      pWin=OpenWin(WS_HP200, 140, 50, 470, 150, "Searching");
      WinText(pWin, 65,25,  "   Please wait...");
      WinText(pWin, 65,40,  "   <ESC aborts>");

      pDBVar->Logic = OR_LOGIC;
      r=SearchDatabase(pDD, 0, pDD->NumDataRec, pDBVar->String1,
                  NULL, OR_LOGIC, pDBVar->IncNotes);

      CloseWin(pWin);
      if(r!=-1) RecordView(pDD,pDBVar, r);
      else r=0;
   }

   ListView(pDD, REPAINT, r);
   ListView(pDD, SELECT, 0);

   pDD = Interact(pDD, pDBVar, r);

   CloseDatabase(pDD);  /* Close database */
   SetLightSleep(SLEEP_ON);  /* just in case */
   PalDeInit(1);        /* quit program   */

   printf("DBV: Portions Copyright 1996, The PAL Group.\n");
   return 0;

}


