/* --------------------------------------------------------------------
   Project: HP DOS Application Manager  Vr. 2.4
   Module:  HDM.C
   Author:  Harry Konstas
   Started: 6 April '95
   ATTENTION: Compile in large model only! -ml
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                            standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --------------------------------------------------------------------
                             local includes
   -------------------------------------------------------------------- */

#include "pal.h"      /* PAL Header file */
#include "hdm.h"


/* --------------------------------------------------------------------
                        global variables
   -------------------------------------------------------------------- */

char *TITLE = "HDM Vr.2.4 - HP200LX DOS Application Manager.";

IMGHDR *pCardImg;              /* Top Card pointer */

int   CurIcn   = 0;            /* currently selected icon    */
int   SrcIcn   = 0;            /* Source Icon for Order mode */
int   TopIcn   = 0;            /* very first icon on screen  */
int   RetFlag  = 0;            /* Return from APP flag       */
int   WaitFlag = 0;            /* Wait for a keypress flag   */
int   ShowFlag = 0;            /* Show TOPCARD flag          */
int   OrderFlag= 0;            /* Icon Order mode flag       */
int   SFflag   = 1;            /* Small font flag            */
int   HOTflag  = 0;
int   Vmode    = 3;            /* current video mode         */
int   StartFlag= 1;            /* startup flag               */

int   UserHot[26];             /* User hotkey definitions    */
char  OwnerName[60];
char  OwnerTitle[60];
char  Company[60];
char  icnpath[80];
char  datfile[80];

char  datdef[] = "c:\\_dat\\dosmgr.dat";

#if DESKTOP
char pcxdef[] = "c:\\_dat\\topcard.pcx";
char icnpathdef[] = "c:\\hp\\flash\\system\\icons\\";
#else
char pcxdef[] = "d:\\_sys\\topcard.pcx";
char icnpathdef[] = "a:\\system\\icons\\";
#endif

/* define the function key labels */
char *KeyLabels[10] = {
   "Help", "Add", "Edit", "Delete", "Open",
   "OpenArg", "Order", "TopCard", "PageUp", "PageDn"
};

char *OKCANLabels[10] = {
   NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, "Cancel", "O.K."
};

char *OKBROLabels[10] = {
   NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, "Browse", "Cancel", "O.K."
};


char pcxfile[80];     /* topcard filename /*

/* --------------------------------------------------------------------
                            Menu structures
   -------------------------------------------------------------------- */

/* the 'Application' pulldown: Items and Menu */
MENUITEM AppsItems[] = {
   { "&Add                 F2", MA_VAL,       MENUVAL(F2_KEY) },
   { "&Edit                F3", MA_VAL,       MENUVAL(F3_KEY) },
   { "&Delete              F4", MA_VAL,       MENUVAL(F4_KEY) },
   { "-----------------------", MT_SEPARATOR, MENUVAL(0)      },
   { "&Change icon order   F7", MA_VAL,       MENUVAL(F7_KEY) },
   { "Extract selected &icon ", MA_VAL,       MENUVAL(CMD_EXT)},
   { "&Sort icons by name    ", MA_VAL,       MENUVAL(CMD_SORT)},
   { "-----------------------", MT_SEPARATOR, MENUVAL(0)      },
   { "&Open                F5", MA_VAL,       MENUVAL(F5_KEY) },
   { "O&pen with arguments F6", MA_VAL,       MENUVAL(F6_KEY) },
   { "-----------------------", MT_SEPARATOR, MENUVAL(0)      },
   { "E&xit               ESC", MA_VAL,       MENUVAL(ESC_KEY)},
};
MENU AppsMenu = { NULL, MS_PULLDN, 0,0,12,12, AppsItems };

/* the 'System' pulldown: Items and Menu */
MENUITEM SystemItems[] = {
   { "&Battery    ", MA_VAL, MENUVAL(CMD_BATTERY) },
   { "&Volume     ", MA_VAL, MENUVAL(CMD_VOLUME)  },
   { "&Information", MA_VAL, MENUVAL(CMD_INFO)    },
   { "&Timeout    ", MA_VAL, MENUVAL(CMD_TIMEOUT) },
};
MENU SystemMenu = { NULL, MS_PULLDN, 0,0,4,4, SystemItems };


/* the 'Setup' pulldown: Items and Menu */
MENUITEM SetupItems[] = {
   { "&Owner                 ", MA_VAL,  MENUVAL(CMD_OWNER)    },
   { "&Icon Directory        ", MA_VAL,  MENUVAL(CMD_ICNDIR)   },
   { "&Change font size  ZOOM", MA_VAL,  MENUVAL(CMD_SMLFNT)   },
};
MENU SetupMenu = { NULL, MS_PULLDN, 0,0,3,3, SetupItems };


/* the 'About' pulldown: Items and Menu */
MENUITEM AboutItems[] = {
   { "&Help  F1", MA_VAL, MENUVAL(F1_KEY)  },
   { "&About   ", MA_VAL, MENUVAL(CMD_ABOUT)},
};
MENU AboutMenu = { NULL, MS_PULLDN, 0,0,2,2, AboutItems };


/* the menu bar  */
MENUITEM BarItems[] = {
   { "&Application",  MA_MNU, &AppsMenu  },
   { "&System",       MA_MNU, &SystemMenu},
   { "Set&up",        MA_MNU, &SetupMenu },
   { "&Quit",         MA_VAL, MENUVAL(ESC_KEY)},
   { "A&bout",        MA_MNU, &AboutMenu },
};
MENU BarMenu = { NULL, MS_HORIZ|MS_TOPLVL, 0,0,5,5, BarItems };


/* --------------------------------------------------------------------
                                 Initialize
   -------------------------------------------------------------------- */

APPBLK *Initialize(void)
{
   APPBLK *pApp;
   int f;

   /* set defaults */
   OrderFlag=0;
   OwnerName[0]=0;
   OwnerTitle[0]=0;
   Company[0]=0;
   strcpy(pcxfile, pcxdef);
   strcpy(icnpath, icnpathdef);

   ReadEnv();                /* read environment */
   if(StartFlag) {
      Vmode = GetVideo();       /* get current video mode */
      StartFlag = 0;
      CreateEnv();              /* save it in .ENV        */
   }

   if(!WaitFlag) {
      printf("\n%s\n", TITLE);
      printf("Loading...");
   }

   else {
      printf("\nPress any key to return to HDM...");
      GetKey();
   }

   if(ShowFlag) {
      pCardImg=LoadPcx(pcxfile,1);               /* load topcard pcx */
      if(!pCardImg) pCardImg=LoadPcx(pcxdef,1);  /* try the default */
   }

   if(!PalInit(1)) FatalExit("Runs only on HP100/200LX...", 99);
   DisplayCard(pCardImg);

   ShowTopTime(TITLE, TRUE);
   ShowFKeys(KeyLabels);
   AnnuncPos(AN_LEFT);   /* place annunciators left */

   pApp = malloc(sizeof(APPBLK));
   if(!pApp) FatalExit("\a\nINIT: Failed to initialize HDM.\n",99);
   pApp->AppNum = 0;

   for(f=0;f<NUM_OF_APPS;f++)
      pApp->item[f]=NULL;

   if(LoadDAT(pApp)) pApp->AppNum=0;  /* DAT not found */

   strcpy(pApp->Signature, "HDM240");
   SetLightSleep(SLEEP_ON);
   SetSerialPort(SERIAL_OFF);
   Invert(0);         /* screen normal */

   if(CurIcn >= pApp->AppNum) {
      TopIcn=0;
      CurIcn=0;
   }

   BuildHot(pApp);    /* built hotkey table */
   ShowIcons(pApp);
   return pApp;
}


/* --------------------------------------------------------------------
                                 Interact
   -------------------------------------------------------------------- */

void Interact(APPBLK *pApp)
{
   WORD Key;
   int Hot;

   do {
      ShowCurrent();
      Key = GetKey();
      ShowTopTime(TITLE, TRUE);
      ShowCurrent();
      if(Key & 0xff) Key &= 0xff;


      if(Key == MENU_KEY || Key == AF10_KEY)
         Key = HandleMenu(&BarMenu, 0, 10);


/* ------------------ ORDER KEY PROCESS ------------------- */
      if(OrderFlag) {   /* Order mode */
         switch(Key) {

            case ESC_KEY:
            case F9_KEY:
               ShowFKeys(KeyLabels);
               OrderFlag=0;
               Key=0;
               break;

            case ENTER_KEY:
            case F10_KEY:
               NewOrder(pApp, SrcIcn,CurIcn);
               ShowFKeys(KeyLabels);
               OrderFlag=0;
               Key=0;
               ShowIcons(pApp);
               break;
         }
      }


/* ------------- OPERATION KEY PROCESS ---------------- */

      switch(Key) {

         case CMD_EXT:
            ExtractIcon(pApp,CurIcn);
            break;

         case CMD_TIMEOUT:
            Timeout();
            break;

         case CMD_SORT:
            Bubble(pApp);
            break;

         case CMD_OWNER:
            SetOwner();
            break;

         case CMD_ICNDIR:
            IconPath();
            break;

         case CMD_BATTERY:
            Battery();
            break;

         case CMD_VOLUME:
            SetVolume();
            break;

         case SPACE_KEY:
         case CMD_INFO:
            SysInfo();
            break;

         case CMD_SMLFNT:
         case ZOOMKEY:
             SFflag=!SFflag;
             ShowIcons(pApp);
             CreateEnv();
             break;

         case DOWN_KEY:
            if(CurIcn+ICNCOLS < pApp->AppNum) CurIcn += ICNCOLS;
            break;
         case UP_KEY:
            if(CurIcn-ICNCOLS >= 0) CurIcn -= ICNCOLS;
            break;
         case LEFT_KEY:
            if(CurIcn > 0) --CurIcn;
            break;
         case RIGHT_KEY:
            if(CurIcn+1 < pApp->AppNum) ++CurIcn;
            break;

         case PGUP_KEY:
         case F9_KEY:       /* PGUP */
            if(TopIcn>=24) {
                CurIcn -=24;
                TopIcn-=24;
                ShowIcons(pApp);
            }
            else {
                CurIcn=0;
                TopIcn=0;
                ShowIcons(pApp);
            }
            break;

         case HOME_KEY:
            CurIcn=0;
            TopIcn=0;
            ShowIcons(pApp);
            break;

         case PGDN_KEY:
         case F10_KEY:       /* PGDN */
            if(TopIcn+24 < pApp->AppNum) {
                CurIcn +=24;
                TopIcn +=24;
                if(CurIcn > pApp->AppNum) CurIcn=pApp->AppNum-1;
                ShowIcons(pApp);
            }
            break;
      }


      /* Second switch Group */
      if(OrderFlag) Key=0;  /* ignore rest in order mode */
      switch(Key) {

         case F2_KEY:       /* new application */
            SetApp(pApp, NEW_APP);
            break;

         case F3_KEY:       /* edit application */
            SetApp(pApp, EDIT_APP);
            break;

         case DEL_KEY:
         case F4_KEY:       /* delete application */
            if(pApp->AppNum>0) DeleteApp(pApp, CurIcn);
            break;

         case F5_KEY:
         case ENTER_KEY:    /* launch application */
            OpenApp(pApp, NO_ARGS);
            break;

         case F6_KEY:
            OpenApp(pApp, WITH_ARGS);
            break;

         case F7_KEY:
            if(pApp->AppNum <2) break;
            OrderFlag=1;
            SrcIcn=CurIcn;
            ShowFKeys(OKCANLabels);
            break;

         case F8_KEY:
            DisplayCard(pCardImg);
            ShowIcons(pApp);
            break;

         case CMD_ABOUT:
            About();  break;

         case F1_KEY:
            Help();   break;


/* --------------- PRE-DEFINED HOTKEY PROCESS -------------- */

      /* CTRL-Fn HOTKEYS */

        case 0x5e00:
            CurIcn=TopIcn+0;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5f00:
            CurIcn=TopIcn+1;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6000:
            CurIcn=TopIcn+2;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6100:
            CurIcn=TopIcn+3;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6200:
            CurIcn=TopIcn+4;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6300:
            CurIcn=TopIcn+5;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6400:
            CurIcn=TopIcn+6;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6500:
            CurIcn=TopIcn+7;
            OpenApp(pApp, NO_ARGS);
            break;



      /* SHIFT-Fn HOTKEYS */

        case 0x5400:
            CurIcn=TopIcn+8;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5500:
            CurIcn=TopIcn+9;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5600:
            CurIcn=TopIcn+10;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5700:
            CurIcn=TopIcn+11;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5800:
            CurIcn=TopIcn+12;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5900:
            CurIcn=TopIcn+13;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5a00:
            CurIcn=TopIcn+14;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x5b00:
            CurIcn=TopIcn+15;
            OpenApp(pApp, NO_ARGS);
            break;


      /* ALT-Fn HOTKEYS */

        case 0x6800:
            CurIcn=TopIcn+16;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6900:
            CurIcn=TopIcn+17;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6a00:
            CurIcn=TopIcn+18;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6b00:
            CurIcn=TopIcn+19;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6c00:
            CurIcn=TopIcn+20;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6d00:
            CurIcn=TopIcn+21;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6e00:
            CurIcn=TopIcn+22;
            OpenApp(pApp, NO_ARGS);
            break;

        case 0x6f00:
            CurIcn=TopIcn+23;
            OpenApp(pApp, NO_ARGS);
            break;

      }

/* ----------------- USER DEFINED HOTKEY PROCESS ----------------- */

      Hot = (Key & 0xff);    /* get ASCII code */

      if(Hot>90) Hot -= 32;  /* convert UCASE  */
      if((Hot>64) && (Hot<91)) {
         Hot = UserHot[Hot-65]; /* get hotkey app. ID */
         if(Hot!=-1) {
            CurIcn = Hot;
            HOTflag=1;
            OpenApp(pApp, NO_ARGS);
         }
      }

/* ------------------ DISPLAY PROCESS ----------------- */

      /* Show icons */

      if(CurIcn >= TopIcn+ICNROWS*ICNCOLS) {
         TopIcn += ICNCOLS;
         ShowIcons(pApp);
      }
      if(CurIcn < TopIcn) {
         TopIcn -= ICNCOLS;
         ShowIcons(pApp);
      }
   } while(Key!=ESC_KEY);
}


/* --------------------------------------------------------------------
                                 MAIN
   -------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
   APPBLK *pApp;

   if(*argv[1]!=126) {
      printf("\n!HDM: Must run DM.EXE to start HDM...\n");
      return 0;
   }

   strcpy(datfile,datdef);
   if(argc==3) strcpy(datfile, argv[2]);

   pApp = Initialize();      /* Initialize */
   Interact(pApp);           /* handle user interaction */

   AnnuncPos(AN_RIGHT);      /* place annunciators right*/
   PalDeInit(1);             /*      quit program       */

   RetFlag=FALSE;
   WaitFlag=FALSE;
   StartFlag=1;
   CreateEnv();              /* recreate environment */

   Zoom(Vmode);
   printf("\nHDM: Portions Copyright 1994, The PAL Group.\n\n");
   return 99;                /* Quit errlevel code */
}

