/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  CLOSEDB.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Close database file
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
                           Close Database
   ------------------------------------------------------------------------- */

void CloseDatabase(DBDESC *pDD)
{

   int f;

   if(!pDD) return;  /* not open!! */

   /* close database file */
   fclose(pDD->pDBF);

   /* free lookup & type-first tables */
   if(pDD->pLOOK) free(pDD->pLOOK);
   if(pDD->pFTAB) free(pDD->pFTAB);

   /* free field & card defs */
   for(f=0;f<pDD->NumFields;f++) {
      if(pDD->pCDEF[f]) free(pDD->pCDEF[f]);
      if(pDD->pFDEF[f]) free(pDD->pFDEF[f]);
   }

   /* free cardpage def & category strings */
   if(pDD->pCPAG) free(pDD->pCPAG);
   if(pDD->pCATS) free(pDD->pCATS);

   /* free current viewpoint definition */
   if(pDD->pVDEF) {
      if(pDD->pVDEF->pSSLtokens) free(pDD->pVDEF->pSSLtokens);
      if(pDD->pVDEF->pSSLstring) free(pDD->pVDEF->pSSLstring);
      free(pDD->pVDEF);
   }

   /* free current linkdef */
   if(pDD->pLINK) {
      if(pDD->pLINK->pLinkString) free(pDD->pLINK->pLinkString);
      free(pDD->pLINK);
   }

   /* free current viewpoint table */
   if(pDD->pVTAB) free(pDD->pVTAB);

   /* free DB header */
   if(pDD->pDBH) free(pDD->pDBH);

   /* finally free DBDESC descriptor */
   free(pDD);

}


