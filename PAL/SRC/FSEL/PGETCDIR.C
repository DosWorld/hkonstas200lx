/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  PGETCDIR.C
   Author:  Harry Konstas / Gilles Kohl
   Started: Jan. 10 1996
   Subject: Get current directory
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

/* --------------------------------------------------------------------
                              Functions
   -------------------------------------------------------------------- */

/* drive: -1=current, 0=A:, 1=B:, 2=C: ... */
int PalGetCurDir(int Drive, char *Dest)
{
   union REGS regs;
   struct SREGS segregs;

   if(Drive < 0) Drive = PalGetDisk()+1;
   *Dest++ = 'A'+Drive-1;
   *Dest++ = ':';
   *Dest++ = '\\';

   regs.x.ax = 0x4700;
   regs.x.dx = Drive & 0xff;

   regs.x.si  = FP_OFF(Dest);
   segregs.ds = FP_SEG(Dest);

   int86x(0x21, &regs, &regs, &segregs);

   return regs.x.ax != 15;
}


