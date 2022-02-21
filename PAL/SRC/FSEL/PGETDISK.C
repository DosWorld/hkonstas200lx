/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  PGETDISK.C
   Author:  Harry Konstas / Gilles Kohl
   Started: Jan. 10 1996
   Subject: Get current disk
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                          standard includes
   -------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>

/* --------------------------------------------------------------------
                            msdos includes
   -------------------------------------------------------------------- */
#include <dos.h>

/* --------------------------------------------------------------------
                           local includes
   -------------------------------------------------------------------- */
#include "pal.h"
#include "palpriv.h"


int PalGetDisk(void)
{
   union REGS regs;

   regs.x.ax = 0x1900;
   int86(0x21, &regs, &regs);

   return regs.h.al;    /* returns drive, 0=A:, 1=B: */
}


