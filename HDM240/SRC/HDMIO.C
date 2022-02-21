/* --------------------------------------------------------------------
   Project: HP DOS Application Manager
   Module:  HDMIO.C
   Author:  Harry Konstas
   Started: 6 April '95
   Subject: I/O and control module
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

/* --------------------------------------------------------------------
                             local includes
   -------------------------------------------------------------------- */

#include "pal.h"      /* PAL Header file */
#include "hdm.h"
#include "hdmrs.h"


/* --------------------------------------------------------------------
                            global variables
   -------------------------------------------------------------------- */

char envfile[] = "c:\\_dat\\dosmgr.env";
char NewArgs[250];

extern SFflag;
extern char *TITLE;
extern int   UserHot[26];
extern char  datfile[80];
extern char *KeyLabels[10];
extern char *OKCANLabels[10];
extern char *OKBROLabels[10];
extern int   CurIcn;
extern int   SrcIcn;
extern int   TopIcn;
extern int   RetFlag;
extern int   WaitFlag;
extern int   ShowFlag;
extern int   Vmode;
extern int   OrderFlag;
extern int   StartFlag;
extern char  OwnerName[60];
extern char  OwnerTitle[60];
extern char  Company[60];
extern char  pcxfile[80];
extern char  icnpath[80];
extern char  deficon[200];
extern int   HOTflag;

/* --------------------------------------------------------------------
                          Find hotkey string lenght
   -------------------------------------------------------------------- */

int HotLen(char *string)
{

   int f=0, s=0;

   while(string[f]) {
      if(string[f]!='&') s++;
      f++;
   }

   return s;
}


/* --------------------------------------------------------------------
   LabelOut:
   Will return the width of a menu or dialog label given a string
   and a font. (Taken from PAL sources)
   -------------------------------------------------------------------- */
void LabelOut(int x, int y, int Style, int FontId, char *Lbl)
{
   int Pos = -1;
   int lx;
   int y2;
   char buf[80];
   char *s;

   y2 = y+FontHeight(FontId);
   strcpy(buf, Lbl);
   if(s = strchr(buf, '&')) {
      Pos = s - buf;
      *s = '\0'; lx = TextExt(FontId, buf); *s = '&';
      strcpy(buf+Pos, Lbl+Pos+1);
   }
   TextOut(x, y, Style, FontId, buf);
   if(Pos >= 0) {
      Line(x+lx, y2, x+lx+TextExt(FontId, "&")-1, y2);
   }
}

/* --------------------------------------------------------------------
                              Load Icon
   -------------------------------------------------------------------- */

void LoadIcon(APPBLK *pApp, int n)
{
   int f;
   char *pBits, *pIcn;

   pBits=&pApp->item[n]->Icon;

   /* erase icon area */
   for(f=0;f<200;f++) pApp->item[n]->Icon[f] = 0;
   pApp->item[n]->IcnFlag = 0;

   if(pIcn=BrowseIcons(pApp,n)) {
      pApp->item[n]->IcnFlag=1;
      memcpy(pBits, pIcn, 200);
      free(pIcn);
   }
}


/* --------------------------------------------------------------------
                             Extract Icon
   -------------------------------------------------------------------- */


void ExtractIcon(APPBLK *pApp, int n)
{
   FILE *pF;
   char buffer[40];

   strcpy(buffer, "c:\\");
   strcat(buffer, pApp->item[n]->AppName);
   buffer[11]=0;
   strcat(buffer, ".icn");

   if((pF = fopen(buffer,"w"))) {
      fwrite(pApp->item[n]->Icon, 200, 1, pF);
      fclose(pF);
      MsgBox("Attention", "Icon extracted to: c:\\", NULL, " &OK");
   }


}


/* --------------------------------------------------------------------
                                Show Icon
   -------------------------------------------------------------------- */

int ShowIcon(APPBLK *pApp, int n)
{
   char dstring[80];
   int  fsize, soff;
   int r = (n-TopIcn)/ICNCOLS;
   int c = (n-TopIcn)%ICNCOLS;
   int x, y;

   SetColor(WHITE_COLOR);
   SetRule(FORCE_RULE);
   Rectangle(XOFF+BOXWIDTH*c,       YOFF+BOXDEPTH*r,
             XOFF+BOXWIDTH*(c+1)-1, YOFF+BOXDEPTH*(r+1)-1, SOLID_FILL);

   if(n>=pApp->AppNum) return 0;

   SetColor(BLACK_COLOR);
   x = XOFF+BOXWIDTH*c;
   y = YOFF+BOXDEPTH*r;

   if((pApp->item[n]->IcnFlag==0)||(pApp->item[n]->Icon==0)) { /* no icon */
      PutImg(x+ICNXOFF, y+ICNYOFF, FORCE_RULE, deficon);
   }

   else PutImg(x+ICNXOFF, y+ICNYOFF, FORCE_RULE, pApp->item[n]->Icon);

   SetColor(BLACK_COLOR);
   SelectFont(SMALL_FONT);
   SetRule(TXT_RULE);
   strcpy(dstring, pApp->item[n]->AppName);

   /* setup font size */
   if(SFflag) {
      fsize=SMALL_FONT;
      soff=5;
   }
   else {
      fsize=MEDIUM_FONT;
      soff=0;
   }

   if(HotLen(dstring)>7)  {
      if(!strstr(dstring,"&"))
         LabelOut(x+5+soff, y+ICNYOFF+44, TXT_RULE, fsize, dstring+7);
      else
         LabelOut(x+5+soff, y+ICNYOFF+44, TXT_RULE, fsize, dstring+8);
   }

   if(!strstr(dstring,"&")) dstring[7]=0;
   else dstring[8]=0;

   if(HotLen(dstring)>0)
      LabelOut(x+5+soff, y+ICNYOFF+33, TXT_RULE, fsize, dstring);

   SetRule(FORCE_RULE);
   SelectFont(MEDIUM_FONT);
   return TRUE;
}


/* --------------------------------------------------------------------
                            Show Icon screen
   -------------------------------------------------------------------- */

void ShowIcons(APPBLK *pApp)
{
   int r, c;

   for(r = 0; r < ICNROWS; r++) {
      for(c = 0; c < ICNCOLS; c++) ShowIcon(pApp, TopIcn + r*ICNCOLS+c);
   }
}


/* --------------------------------------------------------------------
                        Show current Icon
   -------------------------------------------------------------------- */

void ShowCurrent(void)
{
   int x1, y1, x2, y2;
   int r = (CurIcn-TopIcn)/ICNCOLS;
   int c = (CurIcn-TopIcn)%ICNCOLS;

   SetColor(BLACK_COLOR);
   SetRule(XOR_RULE);
   x1 = XOFF+BOXWIDTH*c;
   y1 = YOFF+BOXDEPTH*r;
   x2 = x1 + BOXWIDTH-1;
   y2 = y1 + BOXDEPTH-1;

   if(OrderFlag) RevBlock(x1,y1,x2,y2);
   else {
      Rectangle(x1, y1, x2, y2, OUTLINE_FILL);
      Rectangle(x1+1, y1+1, x2-1, y2-1, OUTLINE_FILL);
   }
}


/* --------------------------------------------------------------------
                             Save DOSAPP.DAT
   -------------------------------------------------------------------- */

int SaveDAT(APPBLK *pApp, int exclude)
{
   FILE *pDAT;
   PALWIN *pWin;
   METER *pMtr;

   int f, delflag=0;

   if(!(pDAT = fopen(datfile,"wb"))) return -1;

   pWin  = OpenWin(WS_HP200, 140, 50, 470, 120, "Please wait...");
   pMtr  = OpenMeter(PROG_METER, 145,80, 3, 0, pApp->AppNum-1, "Saving application list:");

   if(exclude!=ALL) delflag=1;
   pApp->AppNum -= delflag;
   if(pApp->AppNum<1) pApp->AppNum=0;
   fwrite(pApp, sizeof(APPBLK), 1, pDAT);

   for(f=0;f<pApp->AppNum + delflag;f++) {  /* write all except excluded item */
      if(f!=exclude) fwrite(pApp->item[f], sizeof(APPDATA), 1, pDAT);
      UpdateMeter(pMtr,f);
   }

   fclose(pDAT);
   CloseMeter(pMtr);
   CloseWin(pWin);
   return 0;
}

/* --------------------------------------------------------------------
                             Load DOSAPP.DAT
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

   if(strstr(pApp->Signature, "HDM240")) return 0;
   return TRUE;
}


/* --------------------------------------------------------------------
                             Change directory
   -------------------------------------------------------------------- */

void ChangeDir(char *path)
{

    char drive[MAXDRIVE],dir[MAXDIR],file[MAXFILE],ext[MAXEXT];

    fnsplit(path, drive, dir, file, ext);
    strupr(drive);

    if(*drive=='F') setdisk(5);
    if(*drive=='E') setdisk(4);
    if(*drive=='D') setdisk(3);
    if(*drive=='C') setdisk(2);
    if(*drive=='A') setdisk(0);

    strrev(dir);
    dir[0]=32;       /* remove trailing backslash */
    strrev(dir);
    chdir(dir);
}


/* --------------------------------------------------------------------
                             Set Zoom mode
   -------------------------------------------------------------------- */

void Zoom(int Mode)
{
   union REGS regs;
   struct SREGS segregs;

   regs.h.al = Mode;
   regs.h.ah = 0xd0;
   int86x(0x10, &regs, &regs, &segregs);
}


/* --------------------------------------------------------------------
                             Get video mode
   -------------------------------------------------------------------- */

int GetVideo(void)
{
   int   i=0;
   union REGS regs;
   struct SREGS segregs;

   int modetab[]={0,1,2,3,4,5,6,7,0x20,0x21,0x80,0x81,0x82,
                  0x83,0x84,0x85};

   regs.h.al = 0;
   regs.h.ah = 0xdf;

   int86x(0x10, &regs, &regs, &segregs);

   i = regs.h.bh;
   if(i>15||i<0) i=11;

   return modetab[i];
}


/* --------------------------------------------------------------------
                             Set MDA mode
   -------------------------------------------------------------------- */

void SetMDA(void)
{
   union REGS regs;
   struct SREGS segregs;

   regs.h.al = 2;
   regs.h.ah = 0;
   int86x(0x10, &regs, &regs, &segregs);
}


/* --------------------------------------------------------------------
                             Invert picture
   -------------------------------------------------------------------- */

void Invert(int mode)
{
    char v;

    outport(0x22, 0x2c);
    v=inport(0x23);
    if(mode==NEGATIVE) v=v|1;
    else v=v&0xfe;
    outport(0x23, v);
}


/* --------------------------------------------------------------------
                             Dithering
   -------------------------------------------------------------------- */

void Dither(int mode)
{
    char v;

    outport(0x22, 0x2c);
    v=inport(0x23);
    if(mode==DITHER) v=v&0xfd;
    else v=v|2;
    outport(0x23, v);
}


/* --------------------------------------------------------------------
                             Open Application
   -------------------------------------------------------------------- */

void OpenApp(APPBLK *pApp, int mode)
{
   int f, istat, key;
   char buffer1[100], buffer2[100];
   
   if(pApp->AppNum==0) return;
   if(CurIcn>=pApp->AppNum) {
      CurIcn=TopIcn;
      return;
   }

   /* Display tip window */
   if(pApp->item[CurIcn]->HelpFlag==1 && strlen(pApp->item[CurIcn]->HelpMsg)>1) {
      buffer1[0]=0; buffer2[0]=0;
      strcpy(buffer1, pApp->item[CurIcn]->HelpMsg);

      if(strlen(buffer1)>40) {   /* split in two lines */
         for(f=0;f<40;f++)
            buffer2[f]=buffer1[f+40];
         buffer2[40]=0;
         buffer1[40]=0;
      }

      MsgBox("Tip", " | %s | %s |",
                 NULL, "  OK  ", buffer1, buffer2);
   }

   flushall();
   NewArgs[0]=0;
   buffer1[0]=0; buffer2[0]=0;

   if(pApp->item[CurIcn]->Sleep==68) mode=WITH_ARGS;

   /* Open with new arguments */
   if(mode==WITH_ARGS) {
      InitDialog(&ArgsDLG);
      ShowDialog(&ArgsDLG, 50,50, "Open with new arguments");
      ShowFKeys(OKBROLabels);


      for(;;) {
         istat = HandleDialog(&ArgsDLG, &key);

         if(istat==DN_CANCEL) {
            CloseDialog(&ArgsDLG);
            ShowFKeys(KeyLabels);
            return;
         }


         if(key==F8_KEY) {
            GetDlgItem(&ArgsDLG, ARGSDLG1, EDGI_TXT, NewArgs);
            FileSelect(" Select file ","c:\\","*.*", buffer1, NULL);
            if(strlen(NewArgs)>200) break;
            strcat(NewArgs, " ");
            strcat(NewArgs, buffer1);
            SetDlgItem(&ArgsDLG, ARGSDLG1, EDSI_TXT, NewArgs);
         }

         if(key==F10_KEY || key==0x1c0d) {
            GetDlgItem(&ArgsDLG, ARGSDLG1, EDGI_TXT, NewArgs);
            break;
         }

      }

      CloseDialog(&ArgsDLG);
   }

      AnnuncPos(AN_RIGHT);      /* place annunciators right*/
   PalDeInit(1);

#if(!DESKTOP)
    SetSerialPort(SERIAL_OFF);
    if(pApp->item[CurIcn]->Port==1)    /* Enable serial port */
        SetSerialPort(SERIAL_ON);

    if(pApp->item[CurIcn]->Color==66)
        SetMDA();
    if(pApp->item[CurIcn]->Zoom==77)    /* Zoom M */
        Zoom(ZOOM_M);
    if(pApp->item[CurIcn]->Zoom==83)    /* Zoom S */
        Zoom(ZOOM_S);
    if(pApp->item[CurIcn]->Zoom==76)    /* Zoom L */
        Zoom(ZOOM_L);

    if(pApp->item[CurIcn]->Shade==68)
        Dither(DITHER);
    else Dither(GRAY);

    if(pApp->item[CurIcn]->Negative==78)
        Invert(NEGATIVE);

/*  LIGHT SLEEP FEATURE IS REPLACED BY AUTO-ARGUMENT FEATURE
    SetLightSleep(SLEEP_ON);
    if(pApp->item[CurIcn]->Sleep==68)
        SetLightSleep(SLEEP_OFF);
*/


#endif

    WaitFlag =0;
    if(pApp->item[CurIcn]->Pause==87) WaitFlag=1;

    ChangeDir(pApp->item[CurIcn]->AppPath);
    if(NewArgs[0]==0) strcpy(NewArgs,pApp->item[CurIcn]->CmdTail);

    RetFlag=1;
    if(!HOTflag) CreateEnv();

#if(!DESKTOP)
    execlp("d:\\dos\\command.com","d:\\dos\\command.com","/c",
                 pApp->item[CurIcn]->AppPath, NewArgs, NULL);
    SetSerialPort(SERIAL_OFF);

#else
    execlp("command","command","/c",pApp->item[CurIcn]->AppPath,
                     NewArgs, NULL);
#endif

    FatalExit("\a\nCould not load COMMAND.COM, HDM quits.\n",99);
}


/* --------------------------------------------------------------------
                             Display Topcard
   -------------------------------------------------------------------- */

void DisplayCard(IMGHDR *pImg)
{
   if(!ShowFlag) return;

   if(RetFlag) {
      RetFlag=0;
      return;      /* don't display */
   }

   if(pImg) {
      PutImg(0,0, FORCE_RULE, pImg);
      SelectFont(MEDIUM_FONT);
      WriteText(152,75, OwnerName);
      WriteText(152,95, OwnerTitle);    /* changed for address */
      WriteText(165,115,Company);       /* changed for phone number */
      GetKey();
   }

   ClrBlock(0,0,639,199, WHITE_COLOR);
   ShowTopTime(TITLE, TRUE);
   ShowFKeys(KeyLabels);
}


/* --------------------------------------------------------------------
                             Read Environment
   -------------------------------------------------------------------- */

void ReadEnv(void)
{
  FILE *env;
  char buffer[110];

   if(env=fopen(envfile,"r")) {

      fgets(buffer,100, env);
      sscanf(buffer, "%*s %[^\n]", &OwnerName);
      fgets(buffer,100, env);
      sscanf(buffer, "%*s %[^\n]", &OwnerTitle);
      fgets(buffer,100, env);
      sscanf(buffer, "%*s %[^\n]", &Company);

      fgets(buffer,100, env);
      sscanf(buffer, "%*s %[^\n]", &icnpath);
      fgets(buffer,100, env);
      sscanf(buffer, "%*s %[^\n]", &pcxfile);

      fscanf(env, "%d\n", &CurIcn);
      fscanf(env, "%d\n", &TopIcn);
      fscanf(env, "%d\n", &RetFlag);
      fscanf(env, "%d\n", &WaitFlag);
      fscanf(env, "%d\n", &ShowFlag);
      fscanf(env, "%d\n", &SFflag);
      fscanf(env, "%d\n", &Vmode);
      fscanf(env, "%d\n", &StartFlag);

      fclose(env);
   }
}

/* --------------------------------------------------------------------
                             Create Environment
   -------------------------------------------------------------------- */

void CreateEnv(void)
{
  FILE *env;

    env=fopen(envfile,"w");

    fprintf(env, "OWNER: %s\n", OwnerName);
    fprintf(env, "TITLE: %s\n", OwnerTitle);
    fprintf(env, "COMPANY: %s\n", Company);

    fprintf(env, "ICONS: %s\n", icnpath);
    fprintf(env, "TOPCARD: %s\n", pcxfile);

    fprintf(env, "%u\n", CurIcn);
    fprintf(env, "%u\n", TopIcn);
    fprintf(env, "%u\n", RetFlag);
    fprintf(env, "%u\n", WaitFlag);
    fprintf(env, "%u\n", ShowFlag);
    fprintf(env, "%u\n", SFflag);
    fprintf(env, "%u\n", Vmode);
    fprintf(env, "%u\n", StartFlag);

    fclose(env);
}


/* --------------------------------------------------------------------
                             New Order
   -------------------------------------------------------------------- */

void NewOrder(APPBLK *pApp, int src, int dst)
{
   char *hold;

   hold=(char *)pApp->item[src];
   pApp->item[src] = pApp->item[dst];
   pApp->item[dst]=hold;

   SaveDAT(pApp, ALL);
   BuildHot(pApp);
}


/* --------------------------------------------------------------------
                             Icon Browser
   -------------------------------------------------------------------- */

char *BrowseIcons(APPBLK *pApp, int n)
{

   char path[MAXPATH];
   char drive[MAXDRIVE],dir[MAXDIR],file[MAXFILE],ext[MAXEXT];

   char *IcnNames[MAXICONS];  /* array of filenames */
   int i=0, key, IcnCount=0;
   struct ffblk ffb;
   char fname[100];
   char Dir[64];
   FILE *f;
   PALWIN *pWin;
   char *BroBuff;

   if(!(BroBuff=malloc(210))) FatalExit("\a\nBROWSER: Out of memory.\n",99);

   /* search on current path */
   strcpy(fname,pApp->item[n]->AppPath);
   fnsplit(fname, drive, dir, file, ext);
   strcpy(ext,".icn");
   fnmerge(path, drive, dir, file, ext);

   if(f = fopen(path, "rb")) {
      fclose(f);
      IcnNames[IcnCount++] = strdup(path);
   }

   /* search on icon path */
   strcpy(Dir, icnpath);
   if(Dir[strlen(Dir)-1] != '\\') strcat(Dir, "\\");
   strcat(Dir, "*.ICN");

   /* scan all *.ICN in this directory */
   if(findfirst(Dir, &ffb, 0) == 0) do {
      char Name[256];

      if(IcnCount >= MAXICONS) continue; /* ignore if too many icons */
      if(*ffb.ff_name == '.') continue; /* skip . and .. */
      strcpy(Name, Dir);
      strlwr(ffb.ff_name+1);
      strcpy(strrchr(Name, '\\')+1, ffb.ff_name);
      IcnNames[IcnCount++] = strdup(Name);
   } while(findnext(&ffb) == 0);


   /* Browse Icons */
   if(!IcnCount) return 0;  /* no icons found */

   pWin=OpenWin(WS_HP200, 200,40,400,150,"Select icon \x18 \x19");
   WriteText(210,120,"Press F9 to cancel");
   WriteText(212,132," or F10 to accept.");

   for(;;) {

      if((f = fopen(IcnNames[i], "rb"))) {
         if((fread(BroBuff, 200, 1, f) == 1)) {
         PutImg(275, 65, FORCE_RULE, BroBuff);
         fclose(f);
         }
      }
      
      key=GetKey();
      switch(key) {

         case END_KEY:
            i=IcnCount-1;
            break;

         case HOME_KEY:
            i=0;
            break;

         case UP_KEY:
            i--;
            if(i<0) i=IcnCount-1;
            break;

         case DOWN_KEY:
            i++;
            if(i>=IcnCount) i=0;
            break;

         case KEYENTER:
         case F10_KEY:
            for(i=0;i<IcnCount;i++) free(IcnNames[i]);
            CloseWin(pWin);
            return BroBuff;

         case KEYESC:
         case F9_KEY:
            for(i=0;i<IcnCount;i++) free(IcnNames[i]);
            CloseWin(pWin);
            free(BroBuff);
            return 0;
      }
   }
}


/* --------------------------------------------------------------------
                             HotKey builder
   -------------------------------------------------------------------- */

void BuildHot(APPBLK *pApp)
{
   int key, i;
   char *p;

   for(i=0;i<26;i++) UserHot[i] = -1;

   for(i=0;i<pApp->AppNum;i++) {
      key=0;   /* get hotkey, if any (scan to &) */
      if(p=strchr(pApp->item[i]->AppName, 38))  {
         key=*(p+1);
         if(key>90) key -= 32; /* convert UCASE  */
         if((key>64) && (key<91)) UserHot[key-65]=i;  /* assign key */
      }
   }
}


/* --------------------------------------------------------------------
                            Bubble Sort app-list
   -------------------------------------------------------------------- */

void Bubble(APPBLK *pApp)
{
   char *pSwap;
   char string1[40];
   char string2[40];
   int  swap, limit, f, s, d;
   PALWIN *pWin;
   METER *pMtr;

   printf("\a");
   if(!AskYesNo("Sort application list?")) return;

   pWin  = OpenWin(WS_HP200, 140, 50, 470, 120, "Please wait...");
   pMtr  = OpenMeter(LEVEL_METER, 145,80, 3, 0, pApp->AppNum-1, "Sorting application list:");

   limit=pApp->AppNum-1;
   do {
      swap=0;

      for(f=0;f<limit;f++) {
         UpdateMeter(pMtr,f);

         s=0; d=0;
         /* skip white space of 1st string */
         while(pApp->item[f]->AppName[s++]==32);

         /* remove hotkey character */
         s--;
         while(pApp->item[f]->AppName[s]) {
           if(pApp->item[f]->AppName[s++]!=38)
              string1[d++]=pApp->item[f]->AppName[s-1];
         }
         string1[d]=0;

         s=0; d=0;
         /* skip white space of 2st string */
         while(pApp->item[f+1]->AppName[s++]==32);

         /* remove hotkey character */
         s--;
         while(pApp->item[f+1]->AppName[s]) {
           if(pApp->item[f+1]->AppName[s++]!=38)
              string2[d++]=pApp->item[f+1]->AppName[s-1];
         }
         string2[d]=0;

         strupr(string1); strupr(string2);
         if(strcmp(string1,string2)>0) {

            /* swap pointers */
            pSwap=(char *)pApp->item[f];
            pApp->item[f]=pApp->item[f+1];
            pApp->item[f+1]=(void *)pSwap;
            swap=f;
         }
      }
      limit=swap;
   }while (swap);

   SaveDAT(pApp,ALL);
   BuildHot(pApp);  /* update hotkey list */
   CloseMeter(pMtr);
   CloseWin(pWin);
   ShowIcons(pApp);
}

