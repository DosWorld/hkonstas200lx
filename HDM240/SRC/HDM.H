/* --------------------------------------------------------------------
   Project: HP DOS Application Manager                      VERSION 2.4
   Module:  HDM.H
   Author:  Harry Konstas
   Started: 6 April '95
   Subject: HDM Header file

   NOTE: Compile in Large memory model only! Requires PAL Vr.1.06 or
         later.
   -------------------------------------------------------------------- */

#ifndef _HPM_H
#define _HPM_H

#define DESKTOP 0    /* 0 for palmtop version, 1 for desktop development */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

/* --------------------------------------------------------------------
                         local includes
   -------------------------------------------------------------------- */
#include "pal.h"


/* --------------------------------------------------------------------
                         constant definitions
   -------------------------------------------------------------------- */

#define NUM_OF_APPS 120
#define NEW_APP 1
#define EDIT_APP 2

#define NO_ARGS 1
#define WITH_ARGS 2

#define ZOOM_S 0x03
#define ZOOM_M 0x81
#define ZOOM_L 0x85
#define NEGATIVE 1
#define DITHER 1
#define GRAY 0
#define ALL 255

#define SOLID 0xffff
#define DASHED 0x5555

#define MAXICONS 200  /* maximum number of icons to load             */
#define XOFF     10   /* horizontal position of entire icon array    */
#define YOFF     11   /* vertical position of entire icon array      */
#define ICNXOFF  16   /* horizontal position of icon inside box      */
#define ICNYOFF   2   /* vertical position of icon inside box        */
#define BOXWIDTH 78   /* width of icon box, with text and frame      */
#define BOXDEPTH 59   /* depth of icon box, with text and frame      */
#define ICNCOLS   8   /* total number of horizontal icons to display */
#define ICNROWS   3   /* number of rows of icons to display          */

#define KEYENTER 0x1c0d
#define KEYESC 0x011b
#define ZOOMKEY 0xD000

/* --------------------------------------------------------------------
                           type definitions
   -------------------------------------------------------------------- */

typedef enum {
   CMD_ABOUT = 1, CMD_DEFDIAG, CMD_SAVE, CMD_EXIT, CMD_OWNER,
   CMD_VOLUME, CMD_BATTERY, CMD_QUIT, CMD_ICNDIR, CMD_TOPCARD,
   CMD_TITLE, CMD_TIMEOUT, CMD_INFO=14, CMD_SMLFNT, CMD_SORT,
   CMD_EXT
} MENUCOMMANDS;

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
} APPDATA;

typedef struct {
    char Signature[10];          /* file signature */
    int  AppNum;                 /* number of Apps */
    APPDATA *item[NUM_OF_APPS];
} APPBLK;


/* --------------------------------------------------------------------
                                prototypes
   -------------------------------------------------------------------- */

APPBLK *Initialize(void);
void About(void);
void Help(void);
void Interact(APPBLK *pApp);
void LoadIcon(APPBLK *pApp, int n);
int  ShowIcon(APPBLK *pApp, int n);
void ShowIcons(APPBLK *pApp);
void ShowCurrent(void);
void SetApp(APPBLK *pApp, int mode);
int  SaveDAT(APPBLK *pApp, int exclude);
int  LoadDAT(APPBLK *pApp);
void ChangeDir(char *path);
void OpenApp(APPBLK *pApp, int mode);
void Zoom(int Mode);
void Invert(int mode);
void Dither(int mode);
void DisplayCard(IMGHDR *pImg);
void DeleteApp(APPBLK *pApp, int appnum);
void SetOwner(void);
void ReadEnv(void);
void CreateEnv(void);
void IconPath(void);;
void NewOrder(APPBLK *pApp, int src, int dst);
void SetMDA(void);
void Battery(void);
void SetVolume(void);
void SysInfo(void);
int  HotLen(char *string);
char *BrowseIcons(APPBLK *pApp, int n);
void BuildHot(APPBLK *pApp);
void LabelOut(int x, int y, int Style, int FontId, char *Lbl);
void Bubble(APPBLK *pApp);
void Timeout(void);
void ExtractIcon(APPBLK *pApp, int n);
int  GetVideo(void);

#endif

