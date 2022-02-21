/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  READREC.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Read database record
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
                              Read GDB record data
   ------------------------------------------------------------------------- */

void *ReadRecord(DBDESC *pDD, BYTE Type, WORD RecNum)
{
   WORD      size;
   void     *pData;
   DWORD     Offset;

   /* if TYPE_DATA read index from viewpoint table */
   if(Type==TYPE_DATA) {
      if(RecNum > pDD->NumDataRec) {
         pDD->ErrorMsg = RECNUM_OUT_OF_RANGE;
         return NULL;
      }
      RecNum = pDD->pVTAB[RecNum];
   }

   if(!(Offset = GetRecOffset(pDD, Type, RecNum))) return NULL;
   if(!(size = GetRecSize(pDD, Type, RecNum))) return NULL;

   /* now get to the data */
   fseek(pDD->pDBF, Offset + HEADER_SIZE, SEEK_SET);

   /* allocate storage for data */
   if(!(pData = malloc(size))) {
      pDD->ErrorMsg = OUT_OF_MEMORY;
      return NULL;
   }

   /* read the data */
   if(fread(pData, 1, size-HEADER_SIZE, pDD->pDBF) !=
            size-HEADER_SIZE) {

      pDD->ErrorMsg = FREAD_ERROR;
      free(pData);
      return NULL;
   }

   pDD->ErrorMsg = OPERATION_OK;
   return pData;
}

