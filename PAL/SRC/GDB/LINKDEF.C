/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  LINKDEF.C
   Author:  Harry Konstas
   Started: 20 June 1996
   Subject: Read link (smart-clip) definition
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


/* -------------------------------------------------------------------------
                    Read a link (smart clip) definition
   ------------------------------------------------------------------------- */

int ReadLinkDef(DBDESC *pDD, int LinkDef)
{

   SCLINKDEF *pLdef;
   RECHEADER *pRec;

   /* get linkdef definition header */
   if(!(pRec = GetRecHeader(pDD, TYPE_LINKDEF, LinkDef))) return NULL;

   /* allocate storage for link-def structure */
   if(!(pLdef = (SCLINKDEF*)malloc(sizeof(SCLINKDEF)+10))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      free(pRec);
      return FALSE;
   }

   /* read only linkdef (no link string), seek is already set */
   if(fread(pLdef, 1, 36, pDD->pDBF) != 36)  {
      pDD->ErrorMsg = FREAD_ERROR;
      free(pLdef);
      free(pRec);
      return FALSE;
   }

   pLdef->pLinkString = NULL;

   /* read link string, if any */
   if(pLdef->StringLenght) {
      /* allocate storage for link string */
      if(!(pLdef->pLinkString = malloc(pLdef->StringLenght))) {
         pDD->ErrorMsg = OUT_OF_MEMORY;
         free(pLdef);
         free(pRec);
         return FALSE;
      }

      /* read link string, seek is already set */
      if(fread(pLdef->pLinkString, 1, pLdef->StringLenght, pDD->pDBF) !=
            pLdef->StringLenght)  {

         pDD->ErrorMsg = FREAD_ERROR;
         free(pLdef->pLinkString);
         free(pLdef);
         free(pRec);
         return FALSE;
      }
   }

   /* free previous linkdef */
   if(pDD->pLINK) {
      if(pDD->pLINK->pLinkString) free(pDD->pLINK->pLinkString);
      free(pDD->pLINK);
   }

   pDD->pLINK = pLdef;
   pDD->pLINK->pLinkString = pLdef->pLinkString;

   free(pRec);
   pDD->ErrorMsg = OPERATION_OK;
   return TRUE;

}
