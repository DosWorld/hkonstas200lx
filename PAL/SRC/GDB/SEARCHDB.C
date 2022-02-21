/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  SEARCHDB.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Search database records
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
            Search through data records  (returns RecNum or -1)
   -------------------------------------------------------------------- */

int SearchDatabase(DBDESC *pDD, int start, int end,
                    char *string1, char *string2, int logic, int nflag)

{

   int r, f, found1, found2, sflag=0;
   void *pData;
   char *pField;

   char s1[100], s2[100];

   strcpy(s1,string1); strcpy(s2,string2);
   strlwr(s1); strlwr(s2);
   if(strlen(s2)>1) sflag=1;

   pDD->ErrorMsg = STR_FOUND;

   for(r=start; r<=end; r++) {
      if(inportb(0x60)==1) return -1;

      found1 = FALSE; found2 = FALSE;
      pData = ReadRecord(pDD, TYPE_DATA, r);

      if(pData) {

         /* search through fields */
         for(f=0; f<pDD->NumFields; f++) {

            switch(pDD->pFDEF[f]->FieldType) {

            case STRING_FIELD:
            case PHONE_FIELD:
            case NUMBER_FIELD:
            case CURRENCY_FIELD:
            case CATEGORY_FIELD:
            case MULTILINE_FIELD:
            case COMBO_FIELD:
               pField = GetField(pDD, f, pData);

               if(pField) {
                  strlwr(pField);
                  if(strstr(pField,s1)) found1 = TRUE;
                  if(sflag) if(strstr(pField,s2)) found2 = TRUE;
                  free(pField);
               }

               break;

            case NOTE_FIELD:
               if(!nflag) break;
               pField = GetField(pDD, f, pData);

               if(pField) {
                  strlwr(pField);
                  if(strstr(pField,s1)) found1 = TRUE;
                  if(sflag) if(strstr(pField,s2)) found2 = TRUE;
                  free(pField);
               }

               break;
            
            }
         }

         free(pData);
         if(logic==OR_LOGIC) if(found1 || found2)  return r;
         if(logic==AND_LOGIC) if(found1 && found2) return r;
        
      }

   }

   pDD->ErrorMsg = STR_NOT_FOUND;
   return -1;   /* not found! */
}


