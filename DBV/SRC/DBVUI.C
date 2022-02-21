/* --------------------------------------------------------------------
   Project: DBV 2.0: General Database record viewer
   Module:  DBVUI.C
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

/* --------------------------------------------------------------------
                         local includes
   -------------------------------------------------------------------- */

#include "pal.h"
#include "dbv.h"
#include "dbvrs.h"

/* --------------------------------------------------------------------
                           Globals
   -------------------------------------------------------------------- */

extern DBVAR *pDBVar;

/* define the function key labels */
extern char *KeyLabels[10];
extern char *OKLabels[10];
extern char *RecKeys[10];
extern char *NoteLabels[10];

/* --------------------------------------------------------------------
                                 Help window
   -------------------------------------------------------------------- */

void Help(void)

{
    PALWIN *pWin;
    int x=40,y=8;

    pWin  = OpenWin(WS_HELP, 30, 10, 600, 190, "Help");
    SelectFont(MEDIUM_FONT);

    WinText(pWin, x,y,    "   HP200LX Database file viewer/calculator     ");
    WinText(pWin, x,y+10, "   ---------------------------------------     ");
    WinText(pWin, x,y+25, "ARROWS: Browse through database records.       ");
    WinText(pWin, x,y+35, "ENTER:  View record contents.                  ");
    WinText(pWin, x,y+45, " [F3]   View note in full screen.              ");
    WinText(pWin, x,y+55, " [F4]   Find a match for 1 and/or 2 strings.   ");
    WinText(pWin, x,y+65, " [F5]   Find the next match.                   ");
    WinText(pWin, x,y+80, " [F6]   Find Only (create find subset).        ");
    WinText(pWin, x,y+90, " [F8]   Dump a selected record to a file.      ");
    WinText(pWin, x,y+105," [F9]   Open another database.                 ");
    WinText(pWin, x,y+120," Command line arguments:                       ");
    WinText(pWin, x,y+130,"  DBV <dbfile> <optional search string>   ");

    GetKey();
    CloseWin(pWin);
}


/* --------------------------------------------------------------------
                           About window
   -------------------------------------------------------------------- */

void About(void)

{
    PALWIN *pWin;
    pWin  = OpenWin(WS_HP200, 140, 50, 470, 150, title);
    WinText(pWin, 35,8,   "   Freeware HP100/200LX ");
    WinText(pWin, 35,25,  " Database file viewer/calc ");
    WinText(pWin, 35,48 , "     by: Harry Konstas");
    WinText(pWin, 35,70,  "Portions (c)1996 PAL group");

    GetKey();
    CloseWin(pWin);
}


/* -------------------------------------------------------------------------
                             Find string
   ------------------------------------------------------------------------- */

int FindString(DBDESC *pDD, DBVAR *pDBVar, int mode)
{
   PALWIN *pWin;
   WORD istat, flag;
   int r, f, i, hkey, *pVindex;

   InitDialog(&Find);

   if(strlen(pDBVar->String1)>1)
      SetDlgItem(&Find, FIND1, EDSI_TXT, pDBVar->String1);
   if(strlen(pDBVar->String2)>1)
      SetDlgItem(&Find, FIND2, EDSI_TXT, pDBVar->String2);

   if(pDBVar->IncNotes)
      SetDlgItem(&Find, FIND5, CKSI_STATE, (void *)1);

   if(mode==NORMAL) ShowDialog(&Find, 90, 20, "Find");
   else ShowDialog(&Find, 90, 20, "Find Only");
   ShowFKeys(OKLabels);

   for(;;) {
      istat=HandleDialog(&Find, &hkey);
      if(hkey & 0xff) hkey &= 0xff;

      if(hkey==F9_KEY || istat==DN_CANCEL || istat==FIND7) {
         CloseDialog(&Find);
         ShowFKeys(KeyLabels);
         return -1;
      }
      if(hkey==ENTER_KEY || hkey==F10_KEY || istat==FIND6) break;
   }


   GetDlgItem(&Find, FIND1, EDGI_TXT, pDBVar->String1);
   GetDlgItem(&Find, FIND2, EDGI_TXT, pDBVar->String2);

   GetDlgItem(&Find, FIND3, RAGI_STATE, &flag);
   if(flag) pDBVar->Logic= OR_LOGIC;
   GetDlgItem(&Find, FIND4, RAGI_STATE, &flag);
   if(flag) pDBVar->Logic= AND_LOGIC;

   GetDlgItem(&Find, FIND5, CKGI_STATE, &flag);
   if(flag) pDBVar->IncNotes=1;
   else pDBVar->IncNotes=0;

   CloseDialog(&Find);
   ShowFKeys(KeyLabels);

   if(strlen(pDBVar->String1)>1) strlwr(pDBVar->String1);
   else return -1;

   if(strlen(pDBVar->String2)>1) strlwr(pDBVar->String2);
   else pDBVar->String2[0]=0;


   if(mode==NORMAL) {
      pWin=OpenWin(WS_HP200, 140, 50, 470, 150, "Searching");
      WinText(pWin, 65,25,  "   Please wait...");
      WinText(pWin, 65,40,  "   <ESC aborts>");

      SetLightSleep(SLEEP_OFF);

      r=SearchDatabase(pDD, 0, pDD->NumDataRec, pDBVar->String1,
                  pDBVar->String2, pDBVar->Logic, pDBVar->IncNotes);

      CloseWin(pWin);
      SetLightSleep(SLEEP_ON);
   }


   if(mode==FIND_ALL) {
      /* allocate storage for find index table */
      if(!(pVindex = malloc(2000)))
         Error("\a\nFindALL: Out of memory.\n");

      pWin=OpenWin(WS_HP200, 140, 50, 470, 150, "Creating subset");
      WinText(pWin, 65,25,  "   Please wait...");
      WinText(pWin, 65,40,  "   <ESC aborts>");

      SetLightSleep(SLEEP_OFF);

      for(f=0, i=0; f<pDD->NumDataRec; f++) {

         r=SearchDatabase(pDD, f, pDD->NumDataRec, pDBVar->String1,
                  pDBVar->String2, pDBVar->Logic, pDBVar->IncNotes);

         if(inportb(0x60)==1) break;

         if(r!=-1 && i<1000) {
            pVindex[i++] = pDD->pVTAB[r];
            f = r;
         }
      }
      
      CloseWin(pWin);
      SetLightSleep(SLEEP_ON);

      if(i) {
         pDD->NumDataRec = i;
         free(pDD->pVTAB);
         pDD->pVTAB = pVindex;
         strcpy(pDD->pVDEF->Name, "Find-ONLY records");
         if(inportb(0x60)==1) GetKey();
         return 0;
      }

      free(pVindex);
      r=-1;
   }

   if(inportb(0x60)==1) GetKey();

   if(r==-1 && mode==NORMAL) {
      if(pDBVar->BeepFlag) printf("\a");
      MsgBox("ATTENTION!","No Match found.",NULL,"OK");
   }

   return r;
}




/* -------------------------------------------------------------------------
                             Select a viewpoint
   ------------------------------------------------------------------------- */

void SelectViewPoint(DBDESC *pDD)
{
   int n=0, r, CurrVpt;
   PALWIN *pWin;
   char vstring[1000];

   CurrVpt = pDD->pDBH->CurrentViewpt;
   vstring[0] = 0;

   for(r=0;r<pDD->NumViews;r++) {
      if(ChangeViewPoint(pDD,r)) {
         if(r>0)  strcat(vstring,"|");
         strcat(vstring, pDD->pVDEF->Name);
      }
   }

   strcat(vstring,"|                             ");

   pWin=OpenWin(WS_HP200, 140, 50, 474, 60, "Select a subset:");
   n= MenuSelect(vstring,140,60,0,5);
   CloseWin(pWin);

   if(n==-1) {
      ChangeViewPoint(pDD, CurrVpt);   /* get original viewpoint */
      return;
   }

   ChangeViewPoint(pDD,n);

}

/* -------------------------------------------------------------------------
                             Select a field
   ------------------------------------------------------------------------- */

int SelectField(DBDESC *pDD)
{

   PALWIN *pWin;
   int f, n, i, type;
   char buffer[2000];
   char select[100];

   buffer[0]=0; i=0;

   /* read only valid fields */
   for(f=0;f<pDD->NumFields;f++) {
      type = pDD->pFDEF[f]->FieldType;

      switch(type) {
         case PHONE_FIELD:
         case CURRENCY_FIELD:
         case CATEGORY_FIELD:
         case NUMBER_FIELD:
         case TIME_FIELD:
         case DATE_FIELD:
         case STRING_FIELD:
         case NOTE_FIELD:
         case COMBO_FIELD:
            select[i++] = f;
            strcat(buffer,pDD->pFDEF[f]->Name);
            strcat(buffer,"|");
            break;
      }
   }
   strcat(buffer,"                             ");
   select[i++] = f;

   pWin=OpenWin(WS_HP200, 140, 50, 474, 60, "Select a Field:");
   n= MenuSelect(buffer,140,60,0,5);
   CloseWin(pWin);

   if(select[n] >= pDD->NumFields) return -1;
   return select[n];

}


/* -------------------------------------------------------------------------
                             Select criteria
   ------------------------------------------------------------------------- */

void Criteria(DBDESC *pDD)
{
   int f, type, istat, hkey, count=0;
   char index[100];

   InitDialog(&criteria);
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"NOP");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"=");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"~=");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"<>");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"<");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"<=");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,">");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,">=");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"ON");
   LbAddString(&criteria,CRITERIA2,LBPOS_LAST,"OFF");

   SetDlgItem(&criteria, CRITERIA3, EDSI_TXT, pDBVar->CritData);
   LbSetPos(&criteria, CRITERIA2, pDBVar->CritOper);

   /* read only valid fields */
   for(f=0;f<pDD->NumFields;f++) {
      type = pDD->pFDEF[f]->FieldType;

      switch(type) {
         case PHONE_FIELD:
         case CURRENCY_FIELD:
         case CATEGORY_FIELD:
         case NUMBER_FIELD:
         case TIME_FIELD:
         case DATE_FIELD:
         case STRING_FIELD:
         case WORDBOOL_FIELD:
         case BYTEBOOL_FIELD:
         case RADIO_FIELD:
         case COMBO_FIELD:
            LbAddString(&criteria,CRITERIA1,LBPOS_LAST,pDD->pFDEF[f]->Name);
            index[count++]=f;
            break;
      }
   }

   LbSetPos(&criteria, CRITERIA1, pDBVar->Fpos);
   ShowDialog(&criteria,80,40,"Set Criteria");

   for(;;) {
      istat=HandleDialog(&criteria, &hkey);
      if(hkey & 0xff) hkey &= 0xff;

      if(hkey==F9_KEY || istat==DN_CANCEL) {
         CloseDialog(&criteria);
         ShowFKeys(KeyLabels);
         pDBVar->CritOper = NOP_CRIT;  /* no criteria */
         return;
      }
      if(hkey==ENTER_KEY || hkey==F10_KEY) break;
   }

   f = LbGetPos(&criteria, CRITERIA1);
   pDBVar->Fpos = f;
   pDBVar->CritField = index[f];
   pDBVar->CritOper = LbGetPos(&criteria, CRITERIA2);
   GetDlgItem(&criteria, CRITERIA3, EDGI_TXT, pDBVar->CritData);

   CloseDialog(&criteria);
   return;

}


/* -------------------------------------------------------------------------
                             Record View
   ------------------------------------------------------------------------- */

void RecordView(DBDESC *pDD, DBVAR *pDBVar, int index)
{
   PALWIN *pWin;
   WORD Key, key;
   int page = 1, rem;
   int f, r;

   char buffer[80];
   void *pData;
   BYTE *pNotes;
   BYTE *pScan;

   ShowCard(pDD, 1);
   pData = ReadRecord(pDD, TYPE_DATA, index);
   ShowRecord(pDD, 1, pData);
   free(pData);
   ShowFKeys(RecKeys);

   do {

      if(pDD->pCPAG) {
      sprintf(buffer,"%-24s %20s (%d/%d) Page:%d", pDD->FileName,
              pDD->pVDEF->Name, index+1, pDD->NumDataRec, page);
      }

      else {
         sprintf(buffer,"%-24s %20s (%d/%d)", pDD->FileName,
              pDD->pVDEF->Name, index+1, pDD->NumDataRec);
      }

      ShowTopTime(buffer, TRUE);
      Key = GetKey();
      if(Key & 0xff) Key &= 0xff;

      switch(Key) {

         case F1_KEY:
            Help();
            break;

         case F2_KEY:
            ShowFKeys(KeyLabels);
            return;

         case F3_KEY:               /* full note view */

            pData = ReadRecord(pDD, TYPE_DATA, index);

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

               if((key==F3_KEY)&&(rem!=-1)) {
                  pScan += rem;
                  rem = ShowNote(0,15,635,178, pScan);
               }

               key = GetKey();
               if(key & 0xff) key &= 0xff;
               if(key==ESC_KEY) break;
            } while(key!=F10_KEY);

            free(pNotes);

            ShowTopTime(title, TRUE);
            ShowFKeys(RecKeys);
            SetRule(FORCE_RULE);

            ShowCard(pDD, page);
            ShowRecord(pDD, page, pData);

            free(pData);

            break;

         case PGDN_KEY:
            if(pDD->pCPAG) {
               page++;
               if(page>pDD->pCPAG->PageCount) page--;
               pData = ReadRecord(pDD, TYPE_DATA, index);
               ShowCard(pDD, page);
               ShowRecord(pDD, page, pData);
               free(pData);
            }
            break;

         case PGUP_KEY:
            if(pDD->pCPAG) {
               page--;
               if(page ==0) page=1;
               pData = ReadRecord(pDD, TYPE_DATA, index);
               ShowCard(pDD, page);
               ShowRecord(pDD, page, pData);
               free(pData);
            }
            break;


         case F4_KEY:
            r=FindString(pDD, pDBVar, NORMAL);

            ShowFKeys(RecKeys);

            if(r!=-1) {
               index = r;
               pData = ReadRecord(pDD, TYPE_DATA, index);
               ShowCard(pDD, page);
               ShowRecord(pDD, page, pData);
               free(pData);
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
               index = r;
               pData = ReadRecord(pDD, TYPE_DATA, index);
               ShowCard(pDD, page);
               ShowRecord(pDD, page, pData);
               free(pData);
            }

            else {
               if(pDBVar->BeepFlag) printf("\a");
               MsgBox("ATTENTION!","No more matches.",NULL,"OK");
            }

            ShowFKeys(RecKeys);

            break;

         case F8_KEY:
            if(!DumpRecord(pDD, pDBVar, index)) {
               MsgWin("Message","Record saved in: %s",pDBVar->DumpFile);
               GetKey();
               MsgWin("Message",NULL);
            }
            else
               if(pDBVar->BeepFlag) printf("\a");

            break;


         case F7_KEY:
         case DOWN_KEY:
         case RIGHT_KEY:
            if(index < pDD->NumDataRec-1) {
               ShowCard(pDD, page);
               pData = ReadRecord(pDD, TYPE_DATA, ++index);
               ShowRecord(pDD, page, pData);
               free(pData);
            }
            break;

         case F6_KEY:
         case UP_KEY:
         case LEFT_KEY:
            if(index!=0) {
               ShowCard(pDD, page);
               pData = ReadRecord(pDD, TYPE_DATA, --index);
               ShowRecord(pDD, page, pData);
               free(pData);
            }
            break;

      }

   } while(Key != ESC_KEY);

   ShowFKeys(KeyLabels);

}

