/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  SHOWCARD.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Show database card
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
                           Show database card page
   ------------------------------------------------------------------------- */

int ShowCard(DBDESC *pDD, int page)
{
   int f, from, to, type, offset;
   IMGHDR *pImg;

   unsigned char RadioBtn[] = {
   0x01, 0x00, 0x01, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x1F, 0x00,
   0x20, 0x80, 0x40, 0x40, 0x80, 0x20, 0x80, 0x20, 0x80, 0x20,
   0x80, 0x20, 0x80, 0x20, 0x40, 0x40, 0x20, 0x80, 0x1F, 0x00,
   0x00, 0x00,
   };

   if(!pDD->NumFields) {
      pDD->ErrorMsg = MISSING_FIELDEFS;
      return 0;
   }

   pImg = (IMGHDR*)RadioBtn;
   from=0; to=pDD->NumFields;

   /* clear screen area */
   SetColor(WHITE_COLOR);
   SetRule(FORCE_RULE);
   Rectangle(0,12,639,188,SOLID_FILL);
   SetColor(BLACK_COLOR);

   /* check if multiple card database */
   if(pDD->pCPAG) {

      switch(page) {
         case 1:
            from=pDD->pCPAG->ps1;
            to=pDD->pCPAG->ps1 + pDD->pCPAG->pz1;
            break;

         case 2:
            from=pDD->pCPAG->ps2;
            to=pDD->pCPAG->ps2 + pDD->pCPAG->pz2;
            break;

         case 3:
            if(pDD->pCPAG->PageCount >= 3) {
               from=pDD->pCPAG->ps3;
               to=pDD->pCPAG->ps3 + pDD->pCPAG->pz3;
            }
            break;

         case 4:
            if(pDD->pCPAG->PageCount == 4) {
               from=pDD->pCPAG->ps4;
               to=pDD->pCPAG->ps4 + pDD->pCPAG->pz4;
            }
            break;

      }
   }

   SelectFont(MEDIUM_FONT);
   for(f=from;f<to;f++) {
      type = pDD->pFDEF[f]->FieldType;

      switch(type) {

         case STATIC_FIELD:
            WriteText(pDD->pCDEF[f]->x, pDD->pCDEF[f]->y,
                       pDD->pFDEF[f]->Name);
            break;

         case PHONE_FIELD:
         case CURRENCY_FIELD:
         case CATEGORY_FIELD:
         case NOTE_FIELD:
         case NUMBER_FIELD:
         case TIME_FIELD:
         case DATE_FIELD:
         case STRING_FIELD:
            offset=6 + (strlen(pDD->pFDEF[f]->Name) * 10);

            SetRule(TXT_RULE);
            WriteText(pDD->pCDEF[f]->x, pDD->pCDEF[f]->y + 3,
                      pDD->pFDEF[f]->Name);

            SetLineType(0x5555);
            Rectangle(pDD->pCDEF[f]->x + offset, pDD->pCDEF[f]->y,
                      pDD->pCDEF[f]->x + pDD->pCDEF[f]->w,
                      pDD->pCDEF[f]->y + pDD->pCDEF[f]->h,
                      OUTLINE_FILL);
            SetLineType(0xffff);
            break;

         case GROUP_FIELD:
            Rectangle(pDD->pCDEF[f]->x, pDD->pCDEF[f]->y +5,
                      pDD->pCDEF[f]->x + pDD->pCDEF[f]->w,
                      pDD->pCDEF[f]->y + pDD->pCDEF[f]->h,
                      OUTLINE_FILL);

            SetRule(TXT_RULE);
            WriteText(pDD->pCDEF[f]->x + 10, pDD->pCDEF[f]->y,
                      pDD->pFDEF[f]->Name);

            break;

         case WORDBOOL_FIELD:  /* checkbox */
            SetLineType(0xffff);
            Rectangle(pDD->pCDEF[f]->x,pDD->pCDEF[f]->y,
                      pDD->pCDEF[f]->x + 15,
                      pDD->pCDEF[f]->y + 13,
                      OUTLINE_FILL);

            SetRule(TXT_RULE);
            WriteText(pDD->pCDEF[f]->x + 20, pDD->pCDEF[f]->y+2,
                      pDD->pFDEF[f]->Name);

            break;

         case RADIO_FIELD:
            PutImg(pDD->pCDEF[f]->x, pDD->pCDEF[f]->y, FORCE_RULE, pImg);

            SetRule(TXT_RULE);
            WriteText(pDD->pCDEF[f]->x + 20, pDD->pCDEF[f]->y,
                      pDD->pFDEF[f]->Name);

            break;
      }
   }

   pDD->ErrorMsg = OPERATION_OK;
   return f;
}

