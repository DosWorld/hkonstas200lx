/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  SHOWREC.C
   Author:  Harry Konstas
   Started: 13 May 1996
   Subject: Show database record
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
                          Show Note in Window
   -------------------------------------------------------------------- */

int ShowNote(int x, int y, int w, int h, BYTE *text)
{
   BYTE *pSrc, *pDst;
   BYTE buffer[200];
   int  f, xp, yp, count;

   if(text==NULL) return -1;

   count = 0;
   pSrc = text;
   pDst = buffer;

   SetClip(x,y,x+w+5,y+h-1);

   /* clear screen area */
   SetColor(WHITE_COLOR);
   SetRule(FORCE_RULE);
   Rectangle(x,y,x+w+5,y+h-5,SOLID_FILL);
   SetColor(BLACK_COLOR);

   SelectFont(MEDIUM_FONT);

   for(yp=0; yp<h; yp+=10) {
      for(xp=0; xp<w; xp+=10) {

         if(*pSrc==0) {
            *pDst=0;
            WriteText(x, y + yp, buffer);
            SetClip(0,0,639,199);
            return -1;
         }

         if(*pSrc==9) {  /* TAB */
            pSrc++;
            for(f=0;f<8;f++) *pDst++ = 32;
         }

         if(*pSrc==10) {
            count++;
            pSrc++;
            break;
         }

         if(*pSrc >31) *pDst++ = *pSrc;

         count++;
         pSrc++;
      }

      *pDst = 0;
      SetRule(TXT_RULE);
      if(yp<h-10) WriteText(x, y + yp, buffer);
      pDst = buffer;
   }

   SetRule(FORCE_RULE);
   SetClip(0,0,639,199);
   return count;
}


/* -------------------------------------------------------------------------
                               Show record
   ------------------------------------------------------------------------- */

int ShowRecord(DBDESC *pDD, int page, void *pRecData)
{

   WORD f, offset, size, shift, mark, index, value, from, to, len;
   char buffer[100];

   BYTE *pNote, *pRec, *pData;
   IMGHDR *pImg;

   unsigned char RadioOff[] = {
   0x01, 0x00, 0x01, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x1F, 0x00,
   0x20, 0x80, 0x40, 0x40, 0x80, 0x20, 0x80, 0x20, 0x80, 0x20,
   0x80, 0x20, 0x80, 0x20, 0x40, 0x40, 0x20, 0x80, 0x1F, 0x00,
   0x00, 0x00,
   };

   unsigned char RadioOn[] = {
   0x01, 0x00, 0x01, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x1F, 0x00,
   0x20, 0x80, 0x4E, 0x40, 0x9F, 0x20, 0xBF, 0xA0, 0xBF, 0xA0,
   0xBF, 0xA0, 0x9F, 0x20, 0x4E, 0x40, 0x20, 0x80, 0x1F, 0x00,
   0x00, 0x00,
   };

   if(!pRecData) {
      pDD->ErrorMsg = NO_REC_DATA;
      return 0;   /* nothing to display */
   }

   pData = (BYTE*)pRecData;

   if(!pDD->NumFields) {
      pDD->ErrorMsg = MISSING_FIELDEFS;
      return 0;
   }

   from=0; to=pDD->NumFields;

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

   /* display info on all fields */
   for(f=from; f < to; f++) {

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

            pRec = &pData[offset];
            shift= 8 + (strlen(pDD->pFDEF[f]->Name) * 10);
            sprintf(buffer,"%s", pRec);
            len = (pDD->pCDEF[f]->w / 10);
            buffer[len+1]=0;
            SelectFont(MEDIUM_FONT);
            WriteText(pDD->pCDEF[f]->x + shift, pDD->pCDEF[f]->y + 3,buffer);
            break;

         case NOTE_FIELD:

            shift = 8 + (strlen(pDD->pFDEF[f]->Name) * 10);
            index = pData[offset+1]*256 + pData[offset];

            /* read note record - if any */
            if(!(pNote = ReadRecord(pDD, TYPE_NOTE, index))) break;
            size = GetRecSize(pDD, TYPE_NOTE, index);
            pNote[size-HEADER_SIZE] = 0;   /* place string terminator */

            ShowNote(pDD->pCDEF[f]->x + shift,
                     pDD->pCDEF[f]->y + 2,
                     pDD->pCDEF[f]->w - shift - 10,
                     pDD->pCDEF[f]->h, pNote);

            free(pNote);
            break;

         case GROUP_FIELD:  /* nothing to display */
            break;

         case TIME_FIELD:
            if((pData[offset] == 0) && (pData[offset+1] == 0x80))
               break;   /* no time */

            value = pData[offset+1]*256 + pData[offset];
            sprintf(buffer, "%02d:%02d",
                     value / 60, value % 60);

            shift= 8 + (strlen(pDD->pFDEF[f]->Name)*10);

            SelectFont(MEDIUM_FONT);
            WriteText(pDD->pCDEF[f]->x + shift, pDD->pCDEF[f]->y + 3,buffer);
            break;

         case DATE_FIELD:
            if((pData[offset] == 255) &&
               (pData[offset+1] == 255) &&
               (pData[offset+2] == 255)) break;   /* no date */

            shift= 8 + (strlen(pDD->pFDEF[f]->Name)*10);
            sprintf(buffer, "%04d/%02d/%02d", pData[offset]+1900,
                    pData[offset+1]+1, pData[offset+2]+1);
            SelectFont(MEDIUM_FONT);

            WriteText(pDD->pCDEF[f]->x + shift, pDD->pCDEF[f]->y + 3,buffer);
            break;


         case RADIO_FIELD:
            if(pData[offset] == pDD->pFDEF[f]->Reserved)
               pImg = (IMGHDR*)RadioOn;
            else  pImg = (IMGHDR*)RadioOff;

            PutImg(pDD->pCDEF[f]->x, pDD->pCDEF[f]->y, FORCE_RULE, pImg);;
            break;

         /* bit mask */
         case BYTEBOOL_FIELD:
         case WORDBOOL_FIELD:
            mark = pData[offset+1]*256 + pData[offset];
            mark &= pDD->pFDEF[f]->Reserved;

            SelectFont(MEDIUM_FONT);

            if(mark) WriteText(pDD->pCDEF[f]->x+2, pDD->pCDEF[f]->y + 2,"X");
            else     WriteText(pDD->pCDEF[f]->x+2, pDD->pCDEF[f]->y + 2," ");

            break;

      }
   }

   return f;    /* number of fields processed */
}


