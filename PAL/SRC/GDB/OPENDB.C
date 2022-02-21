/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  OPENDB.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Open database (low-level) routines
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                          standard includes
   -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

/* --------------------------------------------------------------------
                            local includes
   -------------------------------------------------------------------- */

#include "pal.h"


/* --------------------------------------------------------------------
                          Internal Utilities
   -------------------------------------------------------------------- */

/* remove ampersand (hotkey markers) from strings */

void RemoveHotkey(char *string)
{

   char *pS, *pD;

   if(!string) return;

   pS=string;
   pD=string;

   do {
      if(*pS != '&') *pD++ = *pS;
   } while(*pS++);

   *pD=0;

}

/* -------------------------------------------------------------------------
                     Get lookup info for specified record
   ------------------------------------------------------------------------- */

PLOOKUP *GetLookup(DBDESC *pDD, BYTE Type, WORD RecNum)
{

   WORD      indx;
   PLOOKUP   *pLook;

   indx = *(pDD->pFTAB + Type);
   if(indx >= pDD->pDBH->NumRecords) {
      pDD->ErrorMsg = RECORD_NOT_FOUND;
      return NULL;
   }

   pLook = pDD->pLOOK + indx + RecNum;

   if(pLook->Flags == LUFLAG_DELETED) {
      pDD->ErrorMsg = RECORD_DELETED;
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   return pLook;

}


/* -------------------------------------------------------------------------
                      Get record offset from lookup table
   ------------------------------------------------------------------------- */

DWORD GetRecOffset(DBDESC *pDD, BYTE Type, WORD RecNum)
{

   DWORD     Offset;
   PLOOKUP   *pLook;

   if(!(pLook = GetLookup(pDD, Type, RecNum))) return 0L;
   Offset = *((DWORD *)pLook->Offset) & 0x00ffffffL;

   pDD->ErrorMsg = OPERATION_OK;
   return Offset;

}


/* -------------------------------------------------------------------------
                       Get record size from lookup table
   ------------------------------------------------------------------------ */

WORD GetRecSize(DBDESC *pDD, BYTE Type, WORD RecNum)
{

   PLOOKUP   *pLook;

   if(!(pLook = GetLookup(pDD, Type, RecNum))) return 0L;

   pDD->ErrorMsg = OPERATION_OK;
   return pLook->Size;

}


/* -------------------------------------------------------------------------
              Get Record Header  (returns with SEEK pointing to data)
   ------------------------------------------------------------------------- */

RECHEADER *GetRecHeader(DBDESC *pDD, BYTE Type, WORD RecNum)
{
   RECHEADER *pRh;
   DWORD Offset;

   if(!(Offset = GetRecOffset(pDD, Type, RecNum))) return NULL;

   /* now get there */
   fseek(pDD->pDBF, Offset, SEEK_SET);

   /* allocate storage for record header */
   if(!(pRh = (RECHEADER*)malloc(sizeof(RECHEADER)))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* read record header */
   if(fread(pRh, 1, HEADER_SIZE, pDD->pDBF) != HEADER_SIZE) {
      free(pRh);
      pDD->ErrorMsg = FREAD_ERROR;
      return NULL;
   }

   if(pRh->Type != Type) {
      free(pRh);
      pDD->ErrorMsg = UNEXPECTED_REC_TYPE;
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   return pRh;
}


/* -------------------------------------------------------------------------
                         Read packed Lookup table
   ------------------------------------------------------------------------- */

PLOOKUP *ReadLookup(DBDESC *pDD)
{

   PLOOKUP *pLookup;
   RECHEADER *pRec;

   if(pDD->pDBH->LookupSeek == 0L) {
      pDD->ErrorMsg = NO_LOOKUP_TABLE;
      return NULL;
   }

   /* get to lookup table */
   fseek(pDD->pDBF, pDD->pDBH->LookupSeek, SEEK_SET);

   /* allocate storage for record header */
   if(!(pRec = malloc(sizeof(RECHEADER)))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* read lookup header */
   if(fread(pRec, 1, HEADER_SIZE, pDD->pDBF) != HEADER_SIZE) {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pRec);
      return NULL;
   }

   if(pRec->Type != TYPE_LOOKUPTABLE) {
      pDD->ErrorMsg = INVALID_LOOKUP;
      free(pRec);
      return NULL;
   }

   /* allocate storage for data */
   if(!(pLookup = malloc(pRec->Lenght))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pRec);
      return NULL;
   }

   /* read the data */
   if(fread(pLookup, 1, pRec->Lenght-HEADER_SIZE, pDD->pDBF) !=
            pRec->Lenght-HEADER_SIZE) {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pLookup);
      free(pRec);
      return NULL;
   }

   free(pRec);
   pDD->ErrorMsg = OPERATION_OK;
   return pLookup;
}


/* -------------------------------------------------------------------------
                           Read TypeFirst table
   ------------------------------------------------------------------------- */

WORD *ReadTypeFirst(DBDESC *pDD)
{
   WORD *pTfirst;
   RECHEADER *pRec;

   if(!(pTfirst = (WORD *)malloc(TYPEFIRST_SIZE+20)))  {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* first get to the lookup table */
   fseek(pDD->pDBF, pDD->pDBH->LookupSeek, SEEK_SET);

   /* allocate storage for record header */
   if(!(pRec = malloc(sizeof(RECHEADER)))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pTfirst);
      return NULL;
   }

   /* read lookup header */
   if(fread(pRec, 1, HEADER_SIZE, pDD->pDBF) != HEADER_SIZE) {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pRec);
      free(pTfirst);
      return NULL;
   }

   if(pRec->Type != TYPE_LOOKUPTABLE) {
      pDD->ErrorMsg = INVALID_LOOKUP;
      free(pRec);
      free(pTfirst);
      return NULL;
   }

   /* now get to the TYPEFIRST table */
   fseek(pDD->pDBF, pRec->Lenght-HEADER_SIZE, SEEK_CUR);

   /* read TypeFirst table */
   if(fread(pTfirst, 1, TYPEFIRST_SIZE, pDD->pDBF) != TYPEFIRST_SIZE) {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pRec);
      free(pTfirst);
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   free(pRec); /* no longer needed */
   return pTfirst;
}


/* -------------------------------------------------------------------------
                            Read Field definitions
   ------------------------------------------------------------------------- */

int ReadFieldefs(DBDESC *pDD)
{

   WORD f, count=0;
   RECHEADER *pRec;
   int fnum;

   /* calculate number of fielddef records */
   fnum = pDD->pFTAB[TYPE_FIELDDEF+1] - pDD->pFTAB[TYPE_FIELDDEF];

   for(f=0;f<fnum;f++) {
      /* read fieldefs, skipping deleted ones */
      pRec = GetRecHeader(pDD, TYPE_FIELDDEF, f);

      if(pDD->ErrorMsg==UNEXPECTED_REC_TYPE) break;
      if(pDD->ErrorMsg==RECORD_NOT_FOUND) break;

      if(pRec) {
         /* allocate storage for fieldef data */
         if(!(pDD->pFDEF[count] = (FIELDEF*)malloc(pRec->Lenght))) {
            pDD->ErrorMsg = OUT_OF_MEMORY;
            free(pRec);
            return 0;
         }

         /* read the data, seek is already set */
         if(fread(pDD->pFDEF[count], 1, pRec->Lenght-HEADER_SIZE,
               pDD->pDBF) != pRec->Lenght-HEADER_SIZE) {

            pDD->ErrorMsg = FREAD_ERROR;
            free(pRec);
            free(pDD->pFDEF[count]);
            return 0;
         }

         count++;
         free(pRec);
      }
   }

   /* register Number of Fields */
   pDD->NumFields = count;

   /* normalize all field names */
   for(f=0; f<pDD->NumFields; f++)
      RemoveHotkey(pDD->pFDEF[f]->Name);

   pDD->ErrorMsg = OPERATION_OK;

   return count;

}


/* -------------------------------------------------------------------------
                           Read card definitions
   ------------------------------------------------------------------------- */

int ReadCardDefs(DBDESC *pDD)
{

   WORD f, len;
   RECHEADER *pRec;

   if(!(pRec = GetRecHeader(pDD, TYPE_CARDDEF, 0))) return 0;
   len = pRec->Lenght/CARDEF_SIZE;

   for(f=0; f<len; f++) {
      if(!(pDD->pCDEF[f] = (CARDDEF*)malloc(sizeof(CARDDEF)))) {
         pDD->ErrorMsg = OUT_OF_MEMORY;
         free(pRec);
         return 0;
      }

      if(fread(pDD->pCDEF[f], 1, sizeof(CARDDEF), pDD->pDBF) !=
               sizeof(CARDDEF)) {
         pDD->ErrorMsg = FREAD_ERROR;
         free(pDD->pCDEF[f]);
         free(pRec);
         return 0;
      }

   }

   pDD->ErrorMsg = OPERATION_OK;
   free(pRec);
   return f;   /* must be same number as fieldefs */

}


/* -------------------------------------------------------------------------
                        Read card page definitions
   ------------------------------------------------------------------------- */

CARDPAGE *ReadCardPage(DBDESC *pDD)
{

   WORD indx;
   RECHEADER *pRec;

   indx = *(pDD->pFTAB + TYPE_CARDPAGEDEF);
   if(indx >= pDD->pDBH->NumRecords) {
      pDD->ErrorMsg = NOT_MULTICARD;
      return NULL;   /* not a multiple card DB */
   }

   if(!(pRec = GetRecHeader(pDD, TYPE_CARDPAGEDEF, 0))) return NULL;

   /* allocate storage for card page data */
   if(!(pDD->pCPAG = (CARDPAGE*)malloc(pRec->Lenght))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pRec);
      return NULL;
   }

   /* read cardpage at once, seek is already set */
   if(fread(pDD->pCPAG, 1, pRec->Lenght-HEADER_SIZE, pDD->pDBF) !=
            pRec->Lenght-HEADER_SIZE)  {

      pDD->ErrorMsg = FREAD_ERROR;
      free(pDD->pCPAG);
      free(pRec);
      return NULL;
   }

   free(pRec);
   pDD->ErrorMsg = OPERATION_OK;
   return pDD->pCPAG;

}


/* -------------------------------------------------------------------------
                        Read Viewpoint definition
   ------------------------------------------------------------------------- */

VIEWPTDEF *ReadViewDef(DBDESC *pDD, int ViewPoint)
{

   VIEWPTDEF *pVdef;
   RECHEADER *pRec;

   /* get viewpoint definition header */
   if(!(pRec = GetRecHeader(pDD, TYPE_VIEWPTDEF, ViewPoint))) return NULL;

   /* allocate storage for viewpoint-def structure */
   if(!(pVdef = (VIEWPTDEF*)malloc(sizeof(VIEWPTDEF)+10))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pRec);
      return NULL;
   }

   /* read only viewdef (no SSL), seek is already set */
   if(fread(pVdef, 1, 91, pDD->pDBF) != 91)  {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pVdef);
      free(pRec);
      return NULL;
   }

   pVdef->pSSLtokens = NULL;
   pVdef->pSSLstring = NULL;

   /* read SSL tokens, if any */
   if(pVdef->TokenLenght) {
      /* allocate storage for SSL tokens */
      if(!(pVdef->pSSLtokens = malloc(pVdef->TokenLenght))) {
         pDD->ErrorMsg = OUT_OF_MEMORY;
         free(pVdef);
         free(pRec);
         return NULL;
      }

      /* read SSL tokens, seek is already set */
      if(fread(pVdef->pSSLtokens, 1, pVdef->TokenLenght, pDD->pDBF) !=
            pVdef->TokenLenght)  {

         pDD->ErrorMsg = FREAD_ERROR;
         free(pVdef->pSSLtokens);
         free(pVdef->pSSLstring);
         free(pVdef);
         free(pRec);
         return NULL;
      }
   }

   /* read SSL string, if any, right after SSL tokens */
   if(pVdef->StringLenght) {
      /* allocate storage for SSL string */
      if(!(pVdef->pSSLstring = malloc(pVdef->StringLenght))) {
         pDD->ErrorMsg = OUT_OF_MEMORY;
         free(pVdef->pSSLtokens);
         free(pVdef);
         free(pRec);
         return NULL;
      }

      /* read SSL string, seek is already set */
      if(fread(pVdef->pSSLstring, 1, pVdef->StringLenght, pDD->pDBF) !=
            pVdef->StringLenght)  {

         pDD->ErrorMsg = FREAD_ERROR;
         free(pVdef->pSSLtokens);
         free(pVdef->pSSLstring);
         free(pVdef);
         free(pRec);
         return NULL;
      }
   }


   free(pRec);

   pDD->ErrorMsg = OPERATION_OK;
   return pVdef;

}


/* -------------------------------------------------------------------------
                           Read Viewpoint table
   ------------------------------------------------------------------------- */

int *ReadViewTable(DBDESC *pDD, int ViewPoint)
{
   int *pTView;
   RECHEADER *pRec;

   /* get viewpointTable header (errors in ErrorMsg) */
   if(!(pRec = GetRecHeader(pDD, TYPE_VIEWPTTABLE, ViewPoint))) return NULL;

   if(!(pTView = (int *)malloc(pRec->Lenght))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pRec);
      return NULL;
   }

   /* read Viewpoint table */
   if(fread(pTView, 1, pRec->Lenght-HEADER_SIZE, pDD->pDBF) !=
             pRec->Lenght-HEADER_SIZE) {

      pDD->ErrorMsg = FREAD_ERROR;
      free(pRec);
      free(pTView);
      return NULL;
   }


   /* is it a valid viewpoint? */
   if(pTView[0]==-1) {
      pDD->ErrorMsg = INVALID_VIEWPOINT;
      free(pRec);
      free(pTView);
      return NULL;
   }

   /* register number of viewpoint entries */
   pDD->NumDataRec = (pRec->Lenght - 6) /2;

   free(pRec); /* no longer needed */
   pDD->ErrorMsg = OPERATION_OK;
   return pTView;

}


/* -------------------------------------------------------------------------
                       Read Categories string
   ------------------------------------------------------------------------- */

char *ReadCategory(DBDESC *pDD)
{

   RECHEADER *pRec;
   char String[300];  /* never exceeds 256 bytes */
   char *pCat;

   /* get category record header */
   if(!(pRec = GetRecHeader(pDD, TYPE_CATEGORY, 0))) return NULL;

    /* read category string */
   if(fread(String, 1, pRec->Lenght-HEADER_SIZE, pDD->pDBF) !=
             pRec->Lenght-HEADER_SIZE) {

      pDD->ErrorMsg = FREAD_ERROR;
      free(pRec);
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   free(pRec);
   pCat = strdup(String);
   return pCat;

}

/* -------------------------------------------------------------------------
                       Read database file header
   ------------------------------------------------------------------------- */

DBHEADER *ReadDBHeader(DBDESC *pDD)
{

   DBHEADER *pDBHeader;

   /* get to the beginning */
   fseek(pDD->pDBF, 0L, SEEK_SET);

   /* allocate storage for record header */
   if(!(pDBHeader = malloc(sizeof(DBHEADER)))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* read database header */
   if(fread(pDBHeader, 1, sizeof(DBHEADER), pDD->pDBF) != sizeof(DBHEADER)) {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pDBHeader);
      return NULL;
   }

   /* check database integrity */
   if(strcmp(pDBHeader->Signature, "hcD")) {
      pDD->ErrorMsg = INVALID_DATABASE;
      free(pDBHeader);
      return NULL;
   }

   if(pDBHeader->FileType == 'W') {
      pDD->ErrorMsg = WORLDTIME_DATABASE;
      free(pDBHeader);
      return NULL;
   }

   if(pDBHeader->FileType == '2') {
      pDD->ErrorMsg = APPTBOOK_DATABASE;
      free(pDBHeader);
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   return pDBHeader;
}


/* -------------------------------------------------------------------------
                             Open database
   ------------------------------------------------------------------------- */

DBDESC *OpenDatabase(char *filename, int *ErrorMsg)
{
   WORD  f,  ext=0;
   DBDESC *pDD;

   if(ErrorMsg==NULL) return NULL;

   if(!(pDD = (DBDESC*)malloc(sizeof(DBDESC)))) {
      *ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize all pointers */
   pDD->pDBH  =NULL;
   pDD->pDBF  =NULL;
   pDD->pCPAG =NULL;
   pDD->pCATS =NULL;
   pDD->pLOOK =NULL;
   pDD->pFTAB =NULL;
   pDD->pLINK =NULL;
   pDD->pVDEF =NULL;
   pDD->pVTAB =NULL;

   for(f=0;f<MAXFIELDS;f++) {
      pDD->pCDEF[f] =NULL;
      pDD->pFDEF[f] =NULL;
   }

   /* if no extension give it .GDB */
   for(f=0;filename[f];f++) if(filename[f]=='.') ext = 1;
   if(!ext) strcat(filename, ".gdb");
   strcpy(pDD->FileName, filename);

   /* open database file */
   if(!(pDD->pDBF = fopen(filename, "rb"))) {
      *ErrorMsg = CANT_OPEN_FILE;
      free(pDD);
      return NULL;
   }

   /* read database header */
   if(!(pDD->pDBH = ReadDBHeader(pDD))) {
      *ErrorMsg = pDD->ErrorMsg;
      fclose(pDD->pDBF);
      free(pDD);
      return NULL;
   }

   /* read lookup table */
   if(!(pDD->pLOOK = ReadLookup(pDD))) {
      *ErrorMsg = pDD->ErrorMsg;
      CloseDatabase(pDD);
      return NULL;
   }

   /* read type first table */
   if(!(pDD->pFTAB = ReadTypeFirst(pDD))) {
      *ErrorMsg = pDD->ErrorMsg;
      CloseDatabase(pDD);
      return NULL;
   }

   if(pDD->pFTAB[TYPE_PASSWORD+1] - pDD->pFTAB[TYPE_PASSWORD]) {
      *ErrorMsg = PASSWORD_PROTECTED;
      CloseDatabase(pDD);
      return NULL;
   }


   /* register number of viewpoints */
   pDD->NumViews = pDD->pFTAB[TYPE_VIEWPTDEF+1] - pDD->pFTAB[TYPE_VIEWPTDEF];

   /* load the first valid viewpoint definition */
   for(f=0;f<pDD->NumViews;f++) {

      /* read 'all database items' viewpoint table */
      if(pDD->pVTAB = ReadViewTable(pDD, f)) {

         /* read general viewpoint definition */
         if(!(pDD->pVDEF = ReadViewDef(pDD, f))) {
            *ErrorMsg = pDD->ErrorMsg;
            CloseDatabase(pDD);
            return NULL;
         }

         f=pDD->NumViews;
         break;
      }
   }

   /* viewpoint loaded? */
   if(!pDD->pVDEF) {
      *ErrorMsg = INVALID_VIEWPOINT;
      CloseDatabase(pDD);
      return NULL;
   }


   /* read field definitions */
   if(!(ReadFieldefs(pDD))) {
      *ErrorMsg = pDD->ErrorMsg;
      CloseDatabase(pDD);
      return NULL;
   }


   /* read card definitions */
   if(!(ReadCardDefs(pDD))) {
      *ErrorMsg = pDD->ErrorMsg;
      CloseDatabase(pDD);
      return NULL;
   }

   /* Read card page definitions - if any */
   pDD->pCPAG = ReadCardPage(pDD);

   /* Read categories string - if any */
   pDD->pCATS = ReadCategory(pDD);

   /* register number of link-defs */
   pDD->NumLinks = pDD->pFTAB[TYPE_LINKDEF+1] - pDD->pFTAB[TYPE_LINKDEF];
   pDD->pLINK = NULL; /* do not read any link-def (optional by user) */

   pDD->ErrorMsg = OPERATION_OK;
   *ErrorMsg = pDD->ErrorMsg;
   return pDD;

}

