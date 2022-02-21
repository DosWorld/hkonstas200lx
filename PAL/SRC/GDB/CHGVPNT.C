/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  CHGVPNT.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Change the current viewpoint
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
                           Change current Viewpoint
   ------------------------------------------------------------------------- */

int ChangeViewPoint(DBDESC *pDD, int ViewPoint)
{
   int *pTable;
   VIEWPTDEF *pVdef;

   /* read viewpoint table */
   pTable = ReadViewTable(pDD, ViewPoint);

   if(!pTable) {
      pDD->ErrorMsg = INVALID_VIEWPOINT;
      return FALSE;
   }

   /* read corresponding viewpoint definition */
   pVdef = ReadViewDef(pDD, ViewPoint);

   if(!pVdef) {
      pDD->ErrorMsg = INVALID_VIEWPOINT;
      return FALSE;
   }

   /* free previous viewdef */
   if(pDD->pVDEF->pSSLtokens) free(pDD->pVDEF->pSSLtokens);
   if(pDD->pVDEF->pSSLstring) free(pDD->pVDEF->pSSLstring);
   free(pDD->pVDEF);
   free(pDD->pVTAB);

   pDD->pVTAB = pTable;
   pDD->pVDEF = pVdef;
   pDD->pVDEF->pSSLtokens = pVdef->pSSLtokens;
   pDD->pVDEF->pSSLstring = pVdef->pSSLstring;

   pDD->pDBH->CurrentViewpt = ViewPoint;
   pDD->ErrorMsg = OPERATION_OK;
   return TRUE;
}
