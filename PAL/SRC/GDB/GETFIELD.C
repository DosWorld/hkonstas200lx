/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  GETFIELD.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Get field data from a given record
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                          standard includes
   -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

/* --------------------------------------------------------------------
                            local includes
   -------------------------------------------------------------------- */

#include "pal.h"


/* --------------------------------------------------------------------
                          Get field data
   -------------------------------------------------------------------- */



char *GetField(DBDESC *pDD, int FieldNum, void *pRecData)
{

   WORD f, offset, size, mark, index, value;
   char buffer[160];

   BYTE *pData;
   char *pNote, *pField;

   if(!pRecData) {
      pDD->ErrorMsg = NO_REC_DATA;
      return NULL;   /* nothing to extract */
   }

   if(FieldNum > pDD->NumFields) {
      pDD->ErrorMsg = INVALID_FIELD;
      return NULL;
   }

   f = FieldNum;
   pData = (BYTE*)pRecData;
   pDD->ErrorMsg = OPERATION_OK;

   /* calculate relative/absolute data offset */
   offset = pDD->pFDEF[f]->DataOffset;
   if(pDD->pFDEF[f]->Flags & 0x20)      /* relative offset? */
      offset = pData[offset+1]*256 + pData[offset];

   switch(pDD->pFDEF[f]->FieldType) {

      /* strings */
      case STRING_FIELD:
      case PHONE_FIELD:
      case NUMBER_FIELD:
      case CURRENCY_FIELD:
      case CATEGORY_FIELD:
      case MULTILINE_FIELD:
      case COMBO_FIELD:

         pData = &pData[offset];
         pField= strdup(pData);
         return pField;


      case NOTE_FIELD:
         index = pData[offset+1]*256 + pData[offset];

         /* read note record - if any */
         if(!(pNote = ReadRecord(pDD, TYPE_NOTE, index))) {
            pDD->ErrorMsg = NO_NOTE;
            return NULL;
         }

         size = GetRecSize(pDD, TYPE_NOTE, index);
         pNote[size-HEADER_SIZE] = 0;   /* place string terminator */
         return pNote;


      case GROUP_FIELD:  /* nothing to display */
         pDD->ErrorMsg = EMPTY_FIELD;
         return NULL;


      case TIME_FIELD:
         if((pData[offset] == 0) && (pData[offset+1] == 0x80))
            break;   /* no time */

         value = pData[offset+1]*256 + pData[offset];
         sprintf(buffer, "%02d:%02d",
                  value / 60, value % 60);

         pField=strdup(buffer);
         return pField;


      case DATE_FIELD:
         if((pData[offset] == 255) &&
            (pData[offset+1] == 255) &&
            (pData[offset+2] == 255)) break;   /* no date */

         sprintf(buffer, "%04d/%02d/%02d", pData[offset]+1900,
                 pData[offset+1]+1, pData[offset+2]+1);
         pField=strdup(buffer);
         return pField;


      case RADIO_FIELD:
         if(pData[offset] == pDD->pFDEF[f]->Reserved)
            buffer[0]='X';
         else  buffer[0]=' ';
         buffer[1]=0;
         pField=strdup(buffer);
         return pField;


      /* bit mask */
      case BYTEBOOL_FIELD:
      case WORDBOOL_FIELD:
         mark = pData[offset+1]*256 + pData[offset];
         mark &= pDD->pFDEF[f]->Reserved;

         if(mark) buffer[0]='X';
         else     buffer[0]=' ';
         buffer[1]=0;
         pField=strdup(buffer);
         return pField;


      default:
         pDD->ErrorMsg = INVALID_FIELD_TYPE;
         return NULL;
   }

   return NULL;
}


