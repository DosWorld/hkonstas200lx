/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  PSETDISK.C
   Author:  Harry Konstas
   Started: Jan. 10 1996
   Subject: Set current disk
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


void PalSetDisk(int Disk)
{
   union REGS regs;

   regs.x.ax = 0x0e00;
   regs.x.dx = Disk & 0xff;
   int86(0x21, &regs, &regs);
}


