#include <stdio.h>
#include <stdlib.h>

#include "pal.h"

void main(void)
{
   char *Key;
   char *Value;
   char *ptr;
   int x, def=9;

   ReadConfig("SETUP.CFG");

   if(Value = GetFirstEntry("Printer", &Key)) do {
      printf("\nThe key %s has the value '%s'", Key, Value);
   } while(Value = GetNextEntry(&Key));

   else printf("Section 'Printer' not found or empty\n");

   ptr=GetConfigString("Printer","test","default");
   printf("\nTest entry= %s",ptr);

   x=GetConfigInt("Printer","hello",def);
   printf("\nValue from Hello= %d",x);

   DiscardConfig();
}
