/* --------------------------------------------------------------------
   Module:  HDMCNV.C
   Author:  Harry Konstas
   Started: 16 May '95
   Subject: HDM Converter
   Note:    COMPILE IN LARGE MODEL ONLY !!!!!    (-ml)
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>


/* --------------------------------------------------------------------
                           type definitions
   -------------------------------------------------------------------- */

#define NUM_OF_APPS 120
#define TRUE 1

typedef unsigned char BYTE;


/* New DOSAPP structure */

typedef struct {
   char AppName[20];
   char AppPath[40];
   char CmdTail[40];
   char HelpMsg[80];
   char Reserved[20];
   BYTE HelpFlag;
   BYTE Zoom;
   BYTE Color;
   BYTE Shade;
   BYTE Negative;
   BYTE Pause;
   BYTE Sleep;
   BYTE Port;
   BYTE IcnFlag;
   BYTE Icon[200];
} APPDATA2;

typedef struct {
    char Signature2[10];
    int  AppNum2;
    APPDATA2 *item2[NUM_OF_APPS];
} APPBLK2;


/* Old DOSAPP structure */

typedef struct {
   char AppName[20];
   char AppPath[40];
   char CmdTail[40];
   BYTE Zoom;
   BYTE Color;
   BYTE Shade;
   BYTE Negative;
   BYTE Pause;
   BYTE Sleep;
   BYTE Port;
   BYTE IcnFlag;
   BYTE Icon[200];
} APPDATA;

typedef struct {
    char Signature[10];          /* file signature */
    int  AppNum;                 /* number of Apps */
    APPDATA *item[NUM_OF_APPS];
} APPBLK;


char  datfile[100];


/* --------------------------------------------------------------------
                               Utilities
   -------------------------------------------------------------------- */

void FatalExit(char *Msg, int ExitCode)
{
   fprintf(stderr, "Fatal error: %s\n", Msg);
   exit(ExitCode);
}


/* --------------------------------------------------------------------
                             Load old DOSAPP.DAT
   -------------------------------------------------------------------- */

int LoadDAT(APPBLK *pApp)
{
   FILE *pDAT;
   int f;

   if(!(pDAT = fopen(datfile,"rb"))) return -1;
   fread(pApp, sizeof(APPBLK), 1, pDAT);

   for(f=0;f<pApp->AppNum;f++) {
      pApp->item[f]=malloc(sizeof(APPDATA));
      if(!pApp) FatalExit("\a\nLoadDAT: Out of memory.\n",99);
      fread(pApp->item[f], sizeof(APPDATA), 1, pDAT);
   }
   fclose(pDAT);

   if(strstr(pApp->Signature, "HDM.DATA")) return 0;
   return TRUE;
}


/* --------------------------------------------------------------------
                             Save new DOSAPP.DAT
   -------------------------------------------------------------------- */

int SaveDAT(APPBLK2 *pApp2)
{
   FILE *pDAT;

   int f;
   char Dest[100];

   strcpy(Dest, datfile);
   for(f=0;f<strlen(datfile);f++) {
      if(Dest[f]==46) break;
   }

   Dest[f+1]=98;
   Dest[f+2]=97;
   Dest[f+3]=107;

   rename(datfile,Dest);
   if(!(pDAT = fopen(datfile,"wb"))) return -1;

   fwrite(pApp2, sizeof(APPBLK2), 1, pDAT);

   for(f=0;f<pApp2->AppNum2;f++)
      fwrite(pApp2->item2[f], sizeof(APPDATA2), 1, pDAT);

   fclose(pDAT);
   return 0;
}


/* --------------------------------------------------------------------
                                M A I N
   -------------------------------------------------------------------- */

void main(int argc, char *argv[])

{
   int f, i;

   APPBLK *pApp;
   APPBLK2 *pApp2;

   strcpy(datfile, "c:\\_dat\\dosmgr.dat");
   if(argc>1) strcpy(datfile, argv[1]);


   pApp2 = malloc(sizeof(APPBLK2));

   fprintf(stderr, "\nHDM Vr.2.4 DOSMGR.DAT file converter.\n");
   fprintf(stderr, "Usage: hdmcnv <dat-file>\n");
   fprintf(stderr, "If no dat-file is given, c:\\_dat\\dosmgr.dat will be converted.\n\n");

   pApp = malloc(sizeof(APPBLK));
   if(!pApp) FatalExit("\a\nINIT: Failed to initialize HDMCNV.\n",99);
   pApp->AppNum = 0;

   for(f=0;f<NUM_OF_APPS;f++)
      pApp->item[f]=NULL;

   if(LoadDAT(pApp)) {
      fprintf(stderr,"\aVr.2.2 DOSMGR.DAT file not found or already converted.\n");
      exit(-2);
   }

   fprintf(stderr, "\nConverting %s ...", datfile);

   strcpy(pApp2->Signature2, "HDM240");
   pApp2->AppNum2 = pApp->AppNum;

   for(f=0;f<pApp2->AppNum2;f++) {

      pApp2->item2[f]=malloc(sizeof(APPDATA2));
      if(!pApp2) FatalExit("\a\nHDMCNV: Out of memory.\n",99);

      fprintf(stderr,".");

      strcpy(pApp2->item2[f]->AppName, pApp->item[f]->AppName);
      strcpy(pApp2->item2[f]->AppPath, pApp->item[f]->AppPath);
      strcpy(pApp2->item2[f]->CmdTail, pApp->item[f]->CmdTail);
      strcpy(pApp2->item2[f]->HelpMsg, "");
      pApp2->item2[f]->HelpFlag = 0;

      pApp2->item2[f]->Zoom = pApp->item[f]->Zoom;
      pApp2->item2[f]->Color = pApp->item[f]->Color;
      pApp2->item2[f]->Shade = pApp->item[f]->Shade;
      pApp2->item2[f]->Negative = pApp->item[f]->Negative;
      pApp2->item2[f]->Pause = pApp->item[f]->Pause;
      pApp2->item2[f]->Sleep = pApp->item[f]->Sleep;
      pApp2->item2[f]->Port = pApp->item[f]->Port;
      pApp2->item2[f]->IcnFlag = pApp->item[f]->IcnFlag;

      for(i=0;i<200;i++)
         pApp2->item2[f]->Icon[i] = pApp->item[f]->Icon[i];

   }


   SaveDAT(pApp2);
   fprintf(stderr, "\n\nConversion completed.\n\n");

}

