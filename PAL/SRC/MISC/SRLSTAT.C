/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  SRLSTAT.C
   Author:  Harry Konstas
   Started: 2/20/1996
   Subject: Gets the serial port status (SERIAL_ON/SERIAL_OFF/INFARED)
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

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
   SerialPort:
   Get Serial Port status
   -------------------------------------------------------------------- */

int GetSerialStatus()
{
   unsigned char x,s;

   s=inportb(0x22);     /* save index */
   outportb(0x22,0x51);
   x=inportb(0x23);
   outportb(0x22,s);    /* restore index */

   if(x&0x20) return SERIAL_ON;
   if(x&1) return INFARED;
   return SERIAL_OFF;
}

