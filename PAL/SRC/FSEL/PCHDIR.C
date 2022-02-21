/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  PCHDIR.C
   Author:  Harry Konstas / Gilles Kohl
   Started: Jan. 10 1996
   Subject: Change to new directory/path
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

int PalChDir(char *Path)
{
   union REGS regs;
   struct SREGS segregs;

   regs.x.ax = 0x3b00;

   regs.x.dx  = FP_OFF(Path);
   segregs.ds = FP_SEG(Path);

   int86x(0x21, &regs, &regs, &segregs);

   return regs.x.ax != 3;
}


