/* --------------------------------------------------------------------
   Project: HP DOS Application Manager
   Module:  HDMUI.C
   Author:  Harry Konstas
   Started: 6 April '95
   Subject: User interface module
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                            standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <alloc.h>


/* --------------------------------------------------------------------
                             local includes
   -------------------------------------------------------------------- */

#include "pal.h"      /* PAL Header file */
#include "hdm.h"
#include "hdmrs.h"


/* --------------------------------------------------------------------
                            global variables
   -------------------------------------------------------------------- */

extern char *KeyLabels[10];
extern char *OKCANLabels[10];
extern int   CurIcn;              /* currently selected icon    */
extern int   TopIcn;
extern int   ShowFlag;
extern char  OwnerName[60];
extern char  OwnerTitle[60];
extern char  Company[60];
extern char  pcxfile[80];
extern char  icnpath[80];
extern char  datfile[80];
extern char  SFflag;

/* --------------------------------------------------------------------
                           Help window
   -------------------------------------------------------------------- */

void Help(void)

{
    PALWIN *pWin;
    int x=40,y=8;

    pWin  = OpenWin(WS_HELP, 30, 10, 600, 190, "Quick-Help");
    SelectFont(MEDIUM_FONT);

    WinText(pWin, x,y,    "   PREDEFINED  L A U N C H   H O T K E Y S     ");
    WinText(pWin, x,y+10, "       CTRL  F1-F8 = First row hotkeys.        ");
    WinText(pWin, x,y+20, "       SHIFT F1-F8 = Second row hotkeys.       ");
    WinText(pWin, x,y+30, "       ALT   F1-F8 = Third row hotkeys.        ");
    WinText(pWin, x,y+40, "                                               ");
    WinText(pWin, x,y+50, "         F U N C T I O N   K E Y S:            ");
    WinText(pWin, x,y+60, " F1 = This help screen  F6 = Run with arguments");
    WinText(pWin, x,y+70, " F2 = Add application   F7 = Move application. ");
    WinText(pWin, x,y+80, " F3 = Edit application  F8 = Display Topcard.  ");
    WinText(pWin, x,y+90, " F4 = DEL  application  F9 = Page up [PGUP].   ");
    WinText(pWin, x,y+100," F5 = Run app. [ENTER]  F10= Page down [PGDN]. ");
    WinText(pWin, x,y+110,"                                               ");
    WinText(pWin, x,y+120,"        ARROWS = Select application.           ");
    WinText(pWin, x,y+133,"   ESC = Quits HDM.  [SPACE] Info. screen.");
    GetKey();
    CloseWin(pWin);
}


/* --------------------------------------------------------------------
                           About window
   -------------------------------------------------------------------- */

void About(void)

{
    PALWIN *pWin;
    pWin  = OpenWin(WS_HP200, 140, 50, 470, 160, "About HDM");
    WinText(pWin, 35,4,   "         HDM 2.4         ");
    WinText(pWin, 35,18,  " DOS Application Manager ");
    WinText(pWin, 35,32 , " Harry Konstas (freeware)");
    WinText(pWin, 35,50,  " email:hkonstas@total.net");
    WinText(pWin, 35,65 , "   Portions Copyright:   ");
    WinText(pWin, 35,80, "  The PAL Group. (c)1996  ");

    GetKey();
    CloseWin(pWin);
}


/* --------------------------------------------------------------------
                            Add/Edit Application
   -------------------------------------------------------------------- */

void SetApp(APPBLK *pApp, int mode)
{
   PALWIN *pWin;
   int indx, istat, flag=0, f;

   if((pApp->AppNum==0) && (mode==EDIT_APP)) return;
   if((pApp->AppNum==NUM_OF_APPS)&&(mode==NEW_APP)) {
      pWin  = OpenWin(WS_HP200, 140, 50, 470, 160, "ATTENTION!");
      WinText(pWin, 35,32 , "    No more room for ");
      WinText(pWin, 35,50,  "   a new application!");

      printf("\a");
      GetKey();
      CloseWin(pWin);
      return; /* no more space */
   }

   InitDialog(&AddDLG);

   if(mode==NEW_APP) {
      SetDlgItem(&AddDLG, ADDDLG11, RASI_STATE, (void *)1); /* Zoom S */
      indx = pApp->AppNum;
      ShowDialog(&AddDLG, 10, 18, "New Application");
   }

   /* Set edit values */
   if(mode==EDIT_APP) {
      indx = CurIcn;
      SetDlgItem(&AddDLG, ADDDLG1, EDSI_TXT, pApp->item[indx]->AppName);

      if(pApp->item[indx]->HelpFlag == 1)
         SetDlgItem(&AddDLG, ADDDLG2, CKSI_STATE, (void *)1);

      SetDlgItem(&AddDLG, ADDDLG3, EDSI_TXT, pApp->item[indx]->AppPath);
      SetDlgItem(&AddDLG, ADDDLG4, EDSI_TXT, pApp->item[indx]->CmdTail);
      SetDlgItem(&AddDLG, ADDDLG5, EDSI_TXT, pApp->item[indx]->HelpMsg);

      /* Zoom mode */
      if(pApp->item[indx]->Zoom == 76)
         SetDlgItem(&AddDLG, ADDDLG9, RASI_STATE, (void *)1);
      if(pApp->item[indx]->Zoom == 77)
         SetDlgItem(&AddDLG, ADDDLG10, RASI_STATE, (void *)1);
      if(pApp->item[indx]->Zoom == 83)
         SetDlgItem(&AddDLG, ADDDLG11, RASI_STATE, (void *)1);

      /* Color */
      if(pApp->item[indx]->Color == 66)
         SetDlgItem(&AddDLG, ADDDLG12, CKSI_STATE, (void *)1);

      /* neg/pos */
      if(pApp->item[indx]->Negative == 78)
         SetDlgItem(&AddDLG, ADDDLG13, CKSI_STATE, (void *)1);

      /* Shading */
      if(pApp->item[indx]->Shade == 68)
         SetDlgItem(&AddDLG, ADDDLG14, CKSI_STATE, (void *)1);

      /* serial port */
      if(pApp->item[indx]->Port == 1)
         SetDlgItem(&AddDLG, ADDDLG15, CKSI_STATE, (void *)1);

      /* pause */
      if(pApp->item[indx]->Pause == 87)
         SetDlgItem(&AddDLG, ADDDLG16, CKSI_STATE, (void *)1);

      /* Get sleep */
      if(pApp->item[indx]->Sleep == 68)
         SetDlgItem(&AddDLG, ADDDLG17, CKSI_STATE, (void *)1);

      ShowDialog(&AddDLG, 10, 10, "Edit Application");
   }


   ShowFKeys(OKCANLabels);

   istat = HandleDialog(&AddDLG, NULL);

   if(istat==DN_CANCEL) {
      CloseDialog(&AddDLG);
      ShowFKeys(KeyLabels);
      return;
   }


   /* allocate storage */
   if(mode==NEW_APP) {
      if(!(pApp->item[indx]=malloc(sizeof(APPDATA))))
         FatalExit("\a\nSETAPP: Out of memory.\n",99);
      pApp->AppNum++;
   }

   GetDlgItem(&AddDLG, ADDDLG1, EDGI_TXT, pApp->item[indx]->AppName);

   GetDlgItem(&AddDLG, ADDDLG2, RAGI_STATE, &flag);
   if(flag) pApp->item[indx]->HelpFlag = 1;
   else pApp->item[indx]->HelpFlag = 0;

   GetDlgItem(&AddDLG, ADDDLG3, EDGI_TXT, pApp->item[indx]->AppPath);
   GetDlgItem(&AddDLG, ADDDLG4, EDGI_TXT, pApp->item[indx]->CmdTail);
   GetDlgItem(&AddDLG, ADDDLG5, EDGI_TXT, pApp->item[indx]->HelpMsg);

   if((strlen(pApp->item[indx]->AppName)<1) ||
      (strlen(pApp->item[indx]->AppPath)<1)) {

      if(mode==NEW_APP) {
         free(pApp->item[indx]);
         pApp->AppNum--;
      }
      CloseDialog(&AddDLG);
      ShowFKeys(KeyLabels);
      return;
   }

   /* Get Zoom mode */
   GetDlgItem(&AddDLG, ADDDLG9, RAGI_STATE, &flag);
   if(flag) pApp->item[indx]->Zoom = 76; /* L */
   GetDlgItem(&AddDLG, ADDDLG10, RAGI_STATE, &flag);
   if(flag) pApp->item[indx]->Zoom = 77; /* M */
   GetDlgItem(&AddDLG, ADDDLG11, RAGI_STATE, &flag);
   if(flag) pApp->item[indx]->Zoom = 83; /* S */

   /* Get color */
   GetDlgItem(&AddDLG, ADDDLG12, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Color = 66; /* B */
   else pApp->item[indx]->Color = 67;     /* C */

   /* Get neg/pos */
   GetDlgItem(&AddDLG, ADDDLG13, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Negative = 78; /* N */
   else pApp->item[indx]->Negative = 80;     /* P */

   /* Get shading */
   GetDlgItem(&AddDLG, ADDDLG14, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Shade = 68; /* D */
   else pApp->item[indx]->Shade = 70;     /* F */

   /* Get serial port */
   GetDlgItem(&AddDLG, ADDDLG15, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Port = 1; /* 1 */
   else pApp->item[indx]->Port = 0;     /* 0 */

   /* Get pause */
   GetDlgItem(&AddDLG, ADDDLG16, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Pause = 87; /* W */
   else pApp->item[indx]->Pause = 82;      /* R */

   /* Get sleep */
   GetDlgItem(&AddDLG, ADDDLG17, CKGI_STATE, &flag);
   if(flag) pApp->item[indx]->Sleep = 68; /* D */
   else pApp->item[indx]->Sleep = 69;     /* E */

   CloseDialog(&AddDLG);

   if(mode==NEW_APP) LoadIcon(pApp,indx);
   if(mode==EDIT_APP) {
      printf("\a");
      istat=MsgBox("Change Icon?", " ", NULL, " &No [F9]| &Yes [F10]");
         if(istat) LoadIcon(pApp,indx);
   }

   SaveDAT(pApp, ALL);          /* save DAT file */

   for(f=0;f<pApp->AppNum;f++)
     free(pApp->item[f]);

   LoadDAT(pApp);
   BuildHot(pApp);  /* update hotkey list */
   ShowFKeys(KeyLabels);
   ShowIcons(pApp);
}


/* --------------------------------------------------------------------
                            Delete Application
   -------------------------------------------------------------------- */

void DeleteApp(APPBLK *pApp, int appnum)
{
   int f;

   printf("\a");
   if(AskYesNo("ATTENTION! Delete Selected Application?")) {

      if(pApp->AppNum>1)  {
         SaveDAT(pApp, appnum);
         for(f=0;f<pApp->AppNum+1;f++)   /* remove from memory */
         free(pApp->item[f]);
      }

      else {
         free(pApp->item[0]);
         remove(datfile);
      }

      if(LoadDAT(pApp)) {
         pApp->AppNum=0;
         CurIcn=0;
         TopIcn=0;
      }

      else
         if(CurIcn >= pApp->AppNum) CurIcn = pApp->AppNum-1;

      BuildHot(pApp);  /* update hotkey list */
      ShowIcons(pApp);
   }
}

/* --------------------------------------------------------------------
                            Owner info screen
   -------------------------------------------------------------------- */

void SetOwner(void)
{
   int istat, flag;

   InitDialog(&OwnerDLG);

   ShowFKeys(OKCANLabels);
   SetDlgItem(&OwnerDLG, OWNERDLG1, EDSI_TXT, &OwnerName);
   SetDlgItem(&OwnerDLG, OWNERDLG2, EDSI_TXT, &OwnerTitle);
   SetDlgItem(&OwnerDLG, OWNERDLG3, EDSI_TXT, &Company);
   SetDlgItem(&OwnerDLG, OWNERDLG4, EDSI_TXT, &pcxfile);

   if(ShowFlag)
      SetDlgItem(&OwnerDLG, OWNERDLG5, RASI_STATE, (void *)1);

   ShowDialog(&OwnerDLG, 30, 20, "Owner");

   istat = HandleDialog(&OwnerDLG, NULL);
   if(istat==DN_CANCEL) {
      CloseDialog(&OwnerDLG);
      ShowFKeys(KeyLabels);
      return;
   }

   GetDlgItem(&OwnerDLG, OWNERDLG1, EDGI_TXT, &OwnerName);
   GetDlgItem(&OwnerDLG, OWNERDLG2, EDGI_TXT, &OwnerTitle);
   GetDlgItem(&OwnerDLG, OWNERDLG3, EDGI_TXT, &Company);
   GetDlgItem(&OwnerDLG, OWNERDLG4, EDGI_TXT, &pcxfile);
   GetDlgItem(&OwnerDLG, OWNERDLG5, CKGI_STATE, &flag);

   if(flag) ShowFlag=1;
   else ShowFlag=0;

   CloseDialog(&OwnerDLG);
   CreateEnv();
   ShowFKeys(KeyLabels);
}


/* --------------------------------------------------------------------
                            Icon Path Screen
   -------------------------------------------------------------------- */

void IconPath(void)
{
   int istat;

   InitDialog(&IconDLG);
   ShowFKeys(OKCANLabels);

   SetDlgItem(&IconDLG, ICONDLG1, EDSI_TXT, &icnpath);

   ShowDialog(&IconDLG, 110, 40, "Set Icon directory");

   istat = HandleDialog(&IconDLG, NULL);
   if(istat==DN_CANCEL) {
      CloseDialog(&IconDLG);
      ShowFKeys(KeyLabels);
      return;
   }

   GetDlgItem(&IconDLG, ICONDLG1, EDGI_TXT, &icnpath);
   CloseDialog(&IconDLG);

   strrev(icnpath);
   if(icnpath[0]!='\\') {
      strrev(icnpath);
      strcat(icnpath,"\\");
   }
   else strrev(icnpath);

   CreateEnv();
   ShowFKeys(KeyLabels);

}


/* --------------------------------------------------------------------
                            Battery setup
   -------------------------------------------------------------------- */

void Battery(void)
{
   int istat, v, flag;
   char buffer[50];
   char buffer2[50];

   InitDialog(&battDLG);
   ShowFKeys(OKCANLabels);

   if(ChargeMode(CHG_STAT))
      SetDlgItem(&battDLG, BATTDLG8, CKSI_STATE, (void *)1);

   if(BattType(BATT_STAT)==ALK_BATT)
      SetDlgItem(&battDLG, BATTDLG3, RASI_STATE, (void *)1);

   if(BattType(BATT_STAT)==NICD_BATT)
      SetDlgItem(&battDLG, BATTDLG4, RASI_STATE, (void *)1);

   ShowDialog(&battDLG, 70, 30, "Battery management");

   SelectFont(MEDIUM_FONT);
   v=BattVoltage(MAIN_BATT);
   sprintf(buffer, "%1u.", v/100);
   sprintf(buffer2, "%2u V", v-((v/100)*100));
   strcat(buffer,buffer2);
   WriteText(230,68,buffer);

   v=BattVoltage(BACK_BATT);
   sprintf(buffer, "%1u.", v/100);
   sprintf(buffer2, "%2u V", v-((v/100)*100));
   strcat(buffer,buffer2);
   WriteText(230,86,buffer);

   if(BattStatus(CARD_BATT)) WriteText(230,105,"LOW!");
   else WriteText(230,105,"OK");

   if(DCStatus()) WriteText(230,122,"YES");
   else WriteText(230,122,"NO");

   istat = HandleDialog(&battDLG, NULL);
   if(istat==DN_CANCEL) {
      CloseDialog(&battDLG);
      ShowFKeys(KeyLabels);
      return;
   }

   GetDlgItem(&battDLG, BATTDLG3, RAGI_STATE, &flag);
   if(flag) BattType(ALK_BATT);
   GetDlgItem(&battDLG, BATTDLG4, RAGI_STATE, &flag);
   if(flag) BattType(NICD_BATT);

   GetDlgItem(&battDLG, BATTDLG8, CKGI_STATE, &flag);
   if(flag) ChargeMode(CHG_ENA);
   else ChargeMode(CHG_DIS);

   CloseDialog(&battDLG);
   ShowFKeys(KeyLabels);
}


/* --------------------------------------------------------------------
                            Volume setting
   -------------------------------------------------------------------- */

void SetVolume(void)
{
   int istat, flag;

   InitDialog(&VolDLG);
   ShowFKeys(OKCANLabels);

#if !DESKTOP
   if(GetVolume()==VOL_OFF)
      SetDlgItem(&VolDLG, VOLDLG1, RASI_STATE, (void *)1);
   if(GetVolume()==VOL_LOW)
      SetDlgItem(&VolDLG, VOLDLG2, RASI_STATE, (void *)1);
   if(GetVolume()==VOL_MEDIUM)
      SetDlgItem(&VolDLG, VOLDLG3, RASI_STATE, (void *)1);
   if(GetVolume()==VOL_HI)
      SetDlgItem(&VolDLG, VOLDLG4, RASI_STATE, (void *)1);
#endif

   ShowDialog(&VolDLG, 140, 30, "Volume level");

   istat = HandleDialog(&VolDLG, NULL);
   if(istat==DN_CANCEL) {
      CloseDialog(&VolDLG);
      ShowFKeys(KeyLabels);
      return;
   }

#if !DESKTOP
   GetDlgItem(&VolDLG, VOLDLG1, RAGI_STATE, &flag);
   if(flag) Volume(VOL_OFF);
   GetDlgItem(&VolDLG, VOLDLG2, RAGI_STATE, &flag);
   if(flag) Volume(VOL_LOW);
   GetDlgItem(&VolDLG, VOLDLG3, RAGI_STATE, &flag);
   if(flag) Volume(VOL_MEDIUM);
   GetDlgItem(&VolDLG, VOLDLG4, RAGI_STATE, &flag);
   if(flag) Volume(VOL_HI);
#endif

   CloseDialog(&VolDLG);
   ShowFKeys(KeyLabels);
}

/* --------------------------------------------------------------------
                             System Info
   -------------------------------------------------------------------- */

void SysInfo(void)
{
   int dummy;
   char buffer[200];
   struct dfree data;

   InitDialog(&InfoDLG);
   ShowDialog(&InfoDLG, 70, 32, "System Information");

   SelectFont(MEDIUM_FONT);

   if(CheckSysMgr()) WriteText(390,62,"YES");
   else WriteText(390,62,"NO");

   sprintf(buffer, "%luK", farcoreleft()/1000);
   WriteText(350,81,buffer);

   getdfree(3,&data);   /* drive c: space */
   sprintf(buffer, "%ld bytes",
      (long) data.df_avail * data.df_sclus * data.df_bsec);
   WriteText(365,120,buffer);
 
   SetErrorHandler(NOE_ERRH);
   dummy=getdisk();
   if(setdisk(0)) getdfree(1,&data); /* drive a: space */
   else data.df_avail=0;

   sprintf(buffer, "%ld bytes",
   (long) data.df_avail * data.df_sclus * data.df_bsec);
   WriteText(365,103,buffer);
   setdisk(dummy);
   SetErrorHandler(DEF_ERRH);

   HandleDialog(&InfoDLG, &dummy);
   CloseDialog(&InfoDLG);

}

/* --------------------------------------------------------------------
                             Set timeout
   -------------------------------------------------------------------- */

void Timeout(void)
{
   WORD r;
   char buffer[82];

   r=MsgBox("Auto shut-off timeout", "Enter timeout value (minutes):|3",
             buffer, " OK |! Cancel");
   if(r != DN_CANCEL) {
      r=atoi(buffer) * 1092;
      if(r<1092) r=1092;
#if !DESKTOP
      SetTimeOut(r);
#endif
   }

}

