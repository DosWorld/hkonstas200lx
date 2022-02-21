/* --------------------------------------------------------------------
   Project: DBV 2.0: General Database record viewer
   Module:  DBVCO.C
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

/* define the function key labels */
extern char *KeyLabels[10];
extern char *OKLabels[10];
extern char *RecKeys[10];
extern char *NoteLabels[10];

extern DBVAR *pDBVar;

/* -------------------------------------------------------------------------
                               Error Handler
   ------------------------------------------------------------------------- */

void Error(char *message)
{
   SetLightSleep(SLEEP_ON);
   PalDeInit(1);
   fprintf(stderr, message);
   exit(1);

}

/* -------------------------------------------------------------------------
                   Read Config file if available
   ------------------------------------------------------------------------- */

void LoadConfig(DBVAR *pDBVar)
{

   char *buf;

   ReadMyConfig("DBV.EXE");
   buf = GetConfigString("paths","dbdir",NULL);
   if(buf==NULL) ReadConfig("c:\\_dat\\dbv.cfg          ");

   buf = GetConfigString("paths","dbdir","*.?db");
   strcpy(pDBVar->Path,buf);

   buf = GetConfigString("files","DumpFile","dbv.sav");
   strcpy(pDBVar->DumpFile,buf);

   pDBVar->BeepFlag = GetConfigInt("options","beep",1);
   pDBVar->IncNotes = GetConfigInt("options","SearchNotes",1);
   pDBVar->HorizFlag= GetConfigInt("options","HorizLines",0);
   pDBVar->Rjust    = GetConfigInt("options","RightJustify",0);
   DiscardConfig();

}


/* -------------------------------------------------------------------------
                            Select database
   ------------------------------------------------------------------------- */

char *SelectDatabase(char *dbpath)
{
   int stat;
   char buf[80];

   ShowFKeys(OKLabels);
   SetRule(FORCE_RULE);
   stat = FileSelect("Open database", dbpath,"*.?db", buf, NULL);
   ShowFKeys(KeyLabels);
   SetRule(FORCE_RULE);

   if(stat==DN_OK) return strdup(buf);
   else return NULL;

}


/* -------------------------------------------------------------------------
                           Create list item
   ------------------------------------------------------------------------- */

char *CreateItem(DBDESC *pDD, int RecNum)
{

   int x, k, pos, stat, size;
   void *pData;
   char item[200];
   char *pItem;
   unsigned char *pField;

   item[0] = 0; pos=0;
   if(RecNum > pDD->NumDataRec-1) return NULL;
   pData = ReadRecord(pDD, TYPE_DATA,RecNum);

   if(!pData) return NULL;  /* no data to process */
   /* read the column fields */
   for(x=0; x<20; x++) {
      size=pDD->pVDEF->ColumnInfo[x]>>8;
      if(size==255) break;
      pField = GetField(pDD, pDD->pVDEF->ColumnInfo[x]&0xff, pData);

      /* construct list item */
      if(pField) {

         stat = 0;
         /* right-justify numeric fields */
         if(pDBVar->Rjust) {
            if(size > strlen(pField)) {
               if((pDD->pFDEF[x]->FieldType>2) && (pDD->pFDEF[x]->FieldType<6)) {
                  stat = 1;
                  for(k=0;k < (size - strlen(pField)-1);k++)
                     item[pos++]=32;
                  k=0;
                  while(pField[k]) item[pos++] = pField[k++];
                  pos++;
               }
            }
         }

         if(!stat) {

            for(k=0; k< size; k++) {
               if(pField[k]>13) item[pos++]=pField[k];
               if(pField[k]==0) {
                  while(k++ < size)
                     item[pos++]=32;
               }
            }
            free(pField);
         }

      }
      item[pos]=0;
      if(strlen(item)>2) item[pos-1]=32;
   }

   free(pData);
   pItem = strdup(item);
   return pItem;

}


/* -------------------------------------------------------------------------
                           Display list titles
   ------------------------------------------------------------------------- */

void DisplayTitles(DBDESC *pDD)
{

   int f, pos;
   char spaces[80];

   for(f=0;f<64;f++) spaces[f]=32;
   spaces[64]=0;

   SetRule(TXT_RULE);
   SelectFont(MEDIUM_FONT);
   SetLineType(0x5555);
   WriteText(0,12,spaces);

   for(f=0, pos=0; f<20; f++) {
      if((pDD->pVDEF->ColumnInfo[f]>>8)==255) break;
      if((pDD->pVDEF->ColumnInfo[f]&0xff)==255) break;


    SetRule(TXT_RULE);
      WriteText(pos,12,pDD->pFDEF[pDD->pVDEF->ColumnInfo[f]&0xff]->Name);

/*    BOLD
      SetRule(OR_RULE);
      WriteText(pos,12,pDD->pFDEF[pDD->pVDEF->ColumnInfo[f]&0xff]->Name);
*/
      if(pos>1) Line(pos,12,pos,180);
      pos += ((pDD->pVDEF->ColumnInfo[f]>>8) * 10);
   }

   SetLineType(0xffff);
   Line(1,22,638,22);
}


/* -------------------------------------------------------------------------
                          List view
   ------------------------------------------------------------------------- */

void ListView(DBDESC *pDD, int mode, int index)
{
   int f, pos = 24;
   char *pItem;
   char spaces[80];

   for(f=0;f<64;f++) spaces[f]=32;
   spaces[64]=0;

   if(mode==SELECT || mode==UNSELECT) {
      if(index<14) RevBlock(1, (index*12)+23, 638, (index*12)+34);
      return;
   }

   if(mode==REPAINT || mode==LOCATE) {
      SetColor(WHITE_COLOR);
      SetRule(FORCE_RULE);
      Rectangle(0,18,639,189, SOLID_FILL);
      SetColor(BLACK_COLOR);
   }

   SetRule(TXT_RULE);
   SelectFont(MEDIUM_FONT);
   SetLineType(0x5555);

   for(f=index; f<index+13; f++) {
      if(pItem = CreateItem(pDD, f)) {
         if(!pItem) pItem=strdup(spaces);
         WriteText(2, pos, pItem);
         if(pDBVar->HorizFlag) Line(0,pos+10,639,pos+10);
         pos+=12;
         free(pItem);
      }
   }

   SetLineType(0xffff);
   if(mode!=SELECT && mode!=UNSELECT && mode!=LOCATE) DisplayTitles(pDD);
}


/* --------------------------------------------------------------------
                            Speed locate
   -------------------------------------------------------------------- */

int SpeedLocate(DBDESC *pDD, char key, int index, int select)
{

   int s=0,r=0,oldr=0;
   void *pData;
   BYTE *pField;
   PALWIN *pWin;
   BYTE keybuf[80];
   
   pWin = OpenWin(WS_HP200, 5,1,635,17, NULL);

   SelectFont(MEDIUM_FONT);
   SetRule(TXT_RULE);

   ListView(pDD, UNSELECT, select);
   WinText(pWin, 2,2,"Locating:");

   /* speed locate loop */
   for(;;) {
      if(s>49) break;
      if((key<32||key>166)&&(key!=BACK_KEY)) break;

      if(key==BACK_KEY) {
         s--; if(s<0) s=0;
         keybuf[s++]=219;
         keybuf[s]=32;
         keybuf[s+1]=0;
         strlwr(keybuf);
         s--; if(s<0) s=0;
      }

      else {
         keybuf[s++] = key;
         keybuf[s] = 219;
         keybuf[s+1]=0;
         strlwr(keybuf);
      }

      r = CutHalf(pDD, keybuf, s);

      if(r!=-1) {
         oldr = r;
         ListView(pDD, LOCATE, r);
         ListView(pDD, SELECT, 0);
         SelectFont(MEDIUM_FONT);
         SetRule(TXT_RULE);
         WinText(pWin, 102, 2, keybuf);
      }

      else {
         r = oldr;
         printf("\a");
         s--; if(s<0) s=0;
         keybuf[s++]=219;
         keybuf[s]=32;
         keybuf[s+1]=0;
         strlwr(keybuf);
         s--; if(s<0) s=0;
      }

      key = GetKey();
   }

   CloseWin(pWin);
   return oldr;

}


/* --------------------------------------------------------------------
                         Convert ASCII -> alphanumeric
   -------------------------------------------------------------------- */

void ConvAlpha(unsigned char *string)
{

   int f=0;
   unsigned char c;

   while(string[f]) {
      c=string[f];

      if(c>32 && c<48 && c!=44) {
         string[f] += 90;
         break;
      }

      if(c>57 && c<65) {
         string[f] += 78;
         break;
      }

      if(c>90 && c<97) {
         string[f] += 54;
         break;
      }

      f++;
   }

}



/* --------------------------------------------------------------------
                         Cut in half search
   -------------------------------------------------------------------- */

int CutHalf(DBDESC *pDD, BYTE *keybuff, int pos)
{

   int pntr, dvder, count;
   BYTE buffer[80];
   void *pData;
   BYTE *pField;

   if(pos==0) return 0;
   strcpy(buffer,keybuff);
   buffer[pos]=0;
   ConvAlpha(buffer);

   pntr = pDD->NumDataRec >> 1;
   dvder = pntr >> 1;
   count=0;

   for(;;) {
      pData = ReadRecord(pDD, TYPE_DATA, pntr);

      pField = NULL;

      if(pData) {
         pField = GetField(pDD, pDD->pVDEF->ColumnInfo[0]&0xff, pData);
         free(pData);
      }

      if(pField) {
         strlwr(pField);
         ConvAlpha(pField);
         if(strlen(pField)>pos) pField[pos]=0;

         if(strcmp(buffer,pField)==0) {
            free(pField);
            return pntr;
         }

         if(strcmp(buffer,pField)>0) pntr += dvder;
         else if(strcmp(buffer,pField)<0) pntr -= dvder;
         else {
            free(pField);
            break;
         }

         free(pField);
      }

      dvder = dvder >> 1;

      if(dvder < 1) {
         dvder=1;
         if((count++)>10) return -1;
      }

      if(pntr > pDD->NumDataRec || pntr < 0) return -1;  /* not found */
   }

}



/* -------------------------------------------------------------------------
                             Dump record
   ------------------------------------------------------------------------- */


int DumpRecord(DBDESC *pDD, DBVAR *pDBVar, int RecNum)
{
   FILE *pDfile;
   int f;
   void *pData, *pField;


   /* read a record */
   pData = ReadRecord(pDD, TYPE_DATA, RecNum);

   /* dump the record */
   if(pData) {

      /* open file to append */
      if(!(pDfile = fopen(pDBVar->DumpFile,"a"))) return FALSE;

      fprintf(pDfile,"\n ---------- FILE: %s, Record #%d ----------\n",
              pDD->FileName, RecNum);

      /* dump all fields */
      for(f=0;f<pDD->NumFields;f++) {

         /* get field data */
         pField = GetField(pDD, f, pData);
         if(pField) {

            switch(pDD->pFDEF[f]->FieldType) {

               case STRING_FIELD:
               case PHONE_FIELD:
               case NUMBER_FIELD:
               case CURRENCY_FIELD:
               case CATEGORY_FIELD:
               case TIME_FIELD:
               case NOTE_FIELD:
               case COMBO_FIELD:
               case LIST_FIELD:

                  fprintf(pDfile," %s = %s\n",pDD->pFDEF[f]->Name, pField);
                  break;

               case RADIO_FIELD:
                  fprintf(pDfile,"  (%s) %s\n",pField, pDD->pFDEF[f]->Name);
                  break;

               case GROUP_FIELD:
                  fprintf(pDfile," %s:\n",pDD->pFDEF[f]->Name);
                  break;

               case WORDBOOL_FIELD:    /* checkbox */
                  fprintf(pDfile,"  [%s] %s\n",pField, pDD->pFDEF[f]->Name);
                  break;
            }
         }

         free(pField);
      }

      fprintf(pDfile,"\n\n");

      /* discard record */
      free(pData);
      fclose(pDfile);
   }

   return FALSE;

}


/* -------------------------------------------------------------------------
                             Compute field
   ------------------------------------------------------------------------- */

void Compute(DBDESC *pDD, int operation)
{

   int n, r, count, TimeFlag=0, oper;
   FILE *pEfile;
   METER *pMtr;
   void *pData, *pField;
   BYTE *pCField;
   char *pScan;

   unsigned long time;
   float result, val, val2, val3;
   WORD hour,minutes;

   char *OpeName[] = {"SUM", "AVG","MIN","MAX","EXT"};

   oper = operation;
   n = SelectField(pDD);     /* select field to compute */
   if(n==-1) return;         /* user aborted */

   val=0; val2=0; val3=0; result=0; count=0;
   pCField=NULL; pField=NULL;

   if(operation==EXT_OP) {
      /* open file to append */
      if(!(pEfile = fopen(pDBVar->DumpFile,"w"))) return;
      pMtr = OpenMeter(PROG_METER, 120,80,4,0, pDD->NumDataRec,
                        "Extracting field data:");
   }

   else {
      pMtr = OpenMeter(PROG_METER, 120,80,4,0, pDD->NumDataRec,
                     "Computing on current subset:");
   }


   /* scan records in current subset */
   for(r=0; r<pDD->NumDataRec; r++) {
      UpdateMeter(pMtr, r);

      if(inp(0x60) == 1) {
         CloseMeter(pMtr);
         return;                 /* user aborted */
      }

      /* read a record */
      pData = ReadRecord(pDD, TYPE_DATA, r);

      if(pData) {
         /* get field data */
         pField = GetField(pDD, n, pData);
         if(!pField) break;

         /* convert time/data to float */
         if(strstr(pField,":")) {
            TimeFlag=1;
            pScan=strchr(pField,58);     /* scan for ':' */
            *pScan=0;                    /* split in half */
            val = atof(pField) * 60;     /* get hours */
            val = val + atof(pScan+1);   /* get minutes */
         }
         else val = atof(pField);

         operation = oper;

         /* process criteria */
         if(pDBVar->CritOper != NOP_CRIT) {  /* any criteria? */
            /* read criteria field */
            pCField = GetField(pDD, pDBVar->CritField, pData);

            if(!pCField) break;
            val2 = atof(pCField);
            val3 = atof(pDBVar->CritData);

            switch(pDBVar->CritOper) {
               case EQUAL_CRIT:
                  if(strcmp(pCField, pDBVar->CritData)==0) break;
                  operation=NO_OP;
                  break;

               case ALMOST_CRIT:
                  if(strstr(pCField, pDBVar->CritData)) break;
                  operation=NO_OP;
                  break;

               case DIFF_CRIT:
                  if(strcmp(pCField, pDBVar->CritData)!=0) break;
                  operation=NO_OP;
                  break;

               case SMALL_CRIT:
                  if(val2<val3) break;
                  operation=NO_OP;
                  break;

               case SMALLEQ_CRIT:
                  if(val2<=val3) break;
                  operation=NO_OP;
                  break;

               case BIG_CRIT:
                  if(val2>val3) break;
                  operation=NO_OP;
                  break;

               case BIGEQ_CRIT:
                  if(val2>=val3) break;
                  operation=NO_OP;
                  break;

               case ON_CRIT:
                  if(pCField[0]=='X') break;
                  operation=NO_OP;
                  break;

               case OFF_CRIT:
                  if(pCField[0]==' ') break;
                  operation=NO_OP;
                  break;

            }

            free(pCField);
         }

         free(pField);

         /* process calulation */
         switch(operation) {

            case NO_OP:  /* no operation */
               break;

            case SUM_OP:
            case AVG_OP:
               result += val;
               count++;
               break;

            case MIN_OP:
               if(!result) result = val;
               if(val < result) result = val;
               break;

            case MAX_OP:
               if(val > result) result = val;
               break;

            case EXT_OP:
               fprintf(pEfile, "%s\n", pField);
               break;

         }
      }

      if(pData) free(pData);

   }

   CloseMeter(pMtr);

   if(operation==EXT_OP) {
      fclose(pEfile);
      return;
   }

   /* display result */
   if(operation==AVG_OP) result = result / count;

   if(TimeFlag) {
      TimeFlag=0;
      time=result;
      hour=time/60;
      minutes=time%60;

      MsgBox("Calculation Result",
             "DATABASE : %s |FIELD: %s |OPERATION: %s = %d:%02d",
              NULL, " OK ", pDD->FileName, pDD->pFDEF[n]->Name,
              OpeName[oper-OP_OFFSET], hour, minutes);
   }

   else {
      MsgBox("Calculation Result",
             "DATABASE : %s |FIELD: %s |%s = %.2f",
              NULL, " OK ", pDD->FileName, pDD->pFDEF[n]->Name,
              OpeName[oper-OP_OFFSET], result);
   }

}


