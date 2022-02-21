/* --------------------------------------------------------------------
   Project: PAL: Palmtop Application Library
   Module:  IHDATE.C
   Author:  Andreas Garzotto
   Started: 21-JAN-96
   Subject: Extension to IhEdit so that dates can be handled
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* --------------------------------------------------------------------
                     local includes and defines
   -------------------------------------------------------------------- */

#include "pal.h"
#include "palkeys.h"
#include "palpriv.h"

#define LASTJULDATE 17520902L   /* last day to use Julian calendar */
#define LASTJULJDN  2361221L    /* jdn of same */

#define CAL_W 188
#define CAL_H 92

/* --------------------------------------------------------------------
                             global data
   -------------------------------------------------------------------- */

static char *month_names[] =
{ "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December"};


static int cal_x, cal_y;  /* location of calendar window */

IMGHDR *Fn4Bgnd = NULL;    /* old f4-key background */
IMGHDR *Fn6Bgnd = NULL;    /* old f6-key background */

/* --------------------------------------------------------------------
                           functions
   -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
                    convert date to julian date
   -------------------------------------------------------------------- */

long ymd_to_julian(int y, int m, int d)
 {
   long jdn;
   int julian = (((y * 100L) + m) * 100 + d  <=  LASTJULDATE);

   if (y < 0)              /* adjust BC year */
      y++;

   if (julian)
      jdn = 367L * y - 7 * (y + 5001L + (m - 9) / 7) / 4
            + 275 * m / 9 + d + 1729777L;
   else
      jdn = (long)(d - 32076)
            + 1461L * (y + 4800L + (m - 14) / 12) / 4
            + 367 * (m - 2 - (m - 14) / 12 * 12) / 12
            - 3 * ((y + 4900L + (m - 14) / 12) / 100) / 4
            + 1;            /* correction by rdg */

   return jdn;
}

/* --------------------------------------------------------------------
                    convert julian date to date
   -------------------------------------------------------------------- */

void julian_to_ymd(long jdn, int *yy, int *mm, int *dd)
{
   long x, z, m, d, y;
   long daysPer400Years = 146097L;
   long fudgedDaysPer4000Years = 1460970L + 31;
   int julian = (jdn <= LASTJULJDN);

   x = jdn + 68569L;
   if ( julian )
   {
      x += 38;
      daysPer400Years = 146100L;
      fudgedDaysPer4000Years = 1461000L + 1;
   }
   z = 4 * x / daysPer400Years;
   x = x - (daysPer400Years * z + 3) / 4;
   y = 4000 * (x + 1) / fudgedDaysPer4000Years;
   x = x - 1461 * y / 4 + 31;
   m = 80 * x / 2447;
   d = x - 2447 * m / 80;
   x = m / 11;
   m = m + 2 - 12 * x;
   y = 100 * (z - 49) + y + x;

   *yy = (int)y;
   *mm = (int)m;
   *dd = (int)d;

   if (*yy <= 0)                   /* adjust BC years */
       (*yy)--;
}

/* --------------------------------------------------------------------
                  Convert Julian date to string
   -------------------------------------------------------------------- */

void DateToString(long jul, char *date)
{
   struct tm t;

   julian_to_ymd(jul, &t.tm_year, &t.tm_mon, &t.tm_mday);
   t.tm_year -= 1900;
   t.tm_mon--;
   FormatDate(date, &t);
}

/* --------------------------------------------------------------------
                  Set date field to given julian date
   -------------------------------------------------------------------- */

static void SetDateField(DIALOG *pDlg, int num, long jul)
{
   DLGITEM *pItm = &(pDlg->Items[num]);
   IHDATE *ihd = pItm->Data;
   char buf[20];

   ihd->date = jul;
   DateToString(jul, buf);
   SetDlgItem(pDlg, num, EDSI_TXT, buf);
}

/* --------------------------------------------------------------------
                  Return Julian date for given date field
   -------------------------------------------------------------------- */

static long GetDateField(DIALOG *pDlg, int num)
{
   DLGITEM *pItm = &(pDlg->Items[num]);
   IHDATE *ihd = pItm->Data;

   return ihd->date;
}

/* --------------------------------------------------------------------
                   Display a window with a calendar
   -------------------------------------------------------------------- */

static PALWIN  *DispCalendar(long jul, long *start, long *first, long *last)
{
   PALWIN *w;
   char cap[30], day[3];
   int ye, m, d, h;
   int x = cal_x + 14;
   int y = cal_y + 30;

   julian_to_ymd(jul, &ye, &m, &d);
   sprintf(cap, "%s %d", month_names[m - 1], ye);
   w = OpenWin(WS_HP200, cal_x, cal_y, cal_x + CAL_W, cal_y + CAL_H, cap);
   TextOut(x, y - 12, TXT_RULE, SMALL_FONT, "Mo Tu We Th Fr Sa Su");
   *first = jul - (long)d + 1L;
   *start = *first - (*first % 7L);
   *last = *first;
   while (1)
   {
      julian_to_ymd(*last, &ye, &m, &d);
      if ((d == 1) && (*last != *first)) break;
      sprintf(day, "%2d", d);
      h = (int)(*last - *start);
      TextOut(x + 24 * (h % 7), y + 10 * (h / 7), TXT_RULE, SMALL_FONT, day);
      (*last)++;
   }
   (*last)--;
   return w;
}

/* --------------------------------------------------------------------
                  Mark a calendar entry if possible
   -------------------------------------------------------------------- */

static int MarkCalendar(long jul, long start, long first, long last)
{
   int x = cal_x + 14;
   int y = cal_y + 30;
   int h = (int)(jul - start);

   if ((jul < first) || (jul > last)) return -1;
   x += 24 * (h % 7);
   y += 10 * (h / 7);
   RevBlock(x - 2, y - 2, x + 17, y + 8);
   return 0;
}

/* --------------------------------------------------------------------
           Display a calendar and let the user select a date
   -------------------------------------------------------------------- */

static void Calendar(DIALOG *pDlg, int num)
{
   PALWIN *w;
   DLGITEM *pItm = &(pDlg->Items[num]);
   IHDATE *ihd = pItm->Data;
   long jul = ihd->date;
   long start, first, last, j = jul;
   int key;

   /* compute location of calendar window */

   cal_x = pDlg->pWin->x1 + pItm->PosX + ((IHEDIT *)(pItm->Data))->WinPos;
   cal_y = pDlg->pWin->y1 + pItm->PosY + pItm->Depth + 12;

   w = DispCalendar(jul, &start, &first, &last);
   MarkCalendar(jul, start, first, last);
   while (1)
   {
      key = GetKey();
      MarkCalendar(j, start, first, last);
      switch (key)
      {
      case 0x1c0d: /* ENTER */
                   CloseWin(w);
                   return;
      case 0x011b: /* ESC */
                   CloseWin(w);
                   SetDateField(pDlg, num, jul);
                   return;
      case 0x4d00: /* right */
                   j++;
                   break;
      case 0x4b00: /* left */
                   j--;
                   break;
      case 0x5000: /* down */
                   j += 7L;
                   break;
      case 0x4800: /* up */
                   j -= 7L;
                   break;
      default: break;
      }
      if (MarkCalendar(j, start, first, last))
      {
         CloseWin(w);
         w = DispCalendar(j, &start, &first, &last);
         MarkCalendar(j, start, first, last);
      }
      SetDateField(pDlg, num, j);
   }
}

/* --------------------------------------------------------------------
                  Set date field to today
   -------------------------------------------------------------------- */

static void SetToday(DIALOG *pDlg, int num)
{
   long ti, date;
   struct tm *t;

   ti = time(NULL);
   t = localtime(&ti);
   date = ymd_to_julian(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
   SetDateField(pDlg, num, date);
}

/* --------------------------------------------------------------------
                    handle key in date field
   -------------------------------------------------------------------- */

static int DateKey(int key, DIALOG *pDlg, int num)
{
   DLGITEM *pItm = &(pDlg->Items[num]);
   IHDATE *ihd = pItm->Data;

   switch (key)
   {
   case 0x0d2b: /* '+' key */
                SetDateField(pDlg, num, ++(ihd->date));
                IhEdit(IM_SHOW, 0, 0, pDlg, pItm);
                return IN_ACK;
   case 0x0c2d: /* '-' key */
                SetDateField(pDlg, num, --(ihd->date));
                IhEdit(IM_SHOW, 0, 0, pDlg, pItm);
                return IN_ACK;
   case F4_KEY:
                SetToday(pDlg, num);
                IhEdit(IM_SHOW, 0, 0, pDlg, pItm);
                return IN_ACK;
   case F6_KEY:
                Calendar(pDlg, num);
                return IN_ACK;
   default: return IhEdit(IM_KEY, key, 0, pDlg, pItm);;
   }
}

/* --------------------------------------------------------------------
                Interpret string as month name
   -------------------------------------------------------------------- */

static int get_month(char *p)
{
   switch (*p)
   {
   case 'j':
   case 'J': if ((p[1] == 'A') || (p[1] == 'a')) return 1;
             if ((p[2] == 'n') || (p[2] == 'N')) return 6;
             return 7;
   case 'f':
   case 'F': return 2;
   case 'm':
   case 'M': if ((p[2] == 'r') || (p[2] == 'R')) return 3;
             return 5;
   case 'a':
   case 'A': if ((p[1] == 'p') || (p[1] == 'P')) return 4;
             return 8;
   case 's':
   case 'S': return 9;
   case 'o':
   case 'O': return 10;
   case 'n':
   case 'N': return 11;
   case 'd':
   case 'D': return 12;
   default: return 0;
   }
}

/* --------------------------------------------------------------------
                Interpret string as year
   -------------------------------------------------------------------- */

static int get_year(char *p)
{
   long ti;
   struct tm *t;
   int y = atoi(p);

   ti = time(NULL);
   t = localtime(&ti);

   if (isdigit(*p))
   {
      if (y < 1900)
      {
          if (y < t->tm_year)
             y += 2000;
          else
             y += 1900;
      }
   }
   else
   {
      y = t->tm_year + 1900;
   }
   return y;
}

/* --------------------------------------------------------------------
                Interpret what user has entered as date
   -------------------------------------------------------------------- */

static void InterpretInput(DIALOG *pDlg, int num)
{
   DLGITEM *pItm = &(pDlg->Items[num]);
   IHDATE *ihd = pItm->Data;
   char buf[80], *p = buf;
   int y = 0, m = 0, d = 0, f;
   extern struct {BYTE DateFmt;} TimeSetup;
   extern void ReadSetup(void);

   ReadSetup();
   f = TimeSetup.DateFmt;

   GetDlgItem(pDlg, num, EDGI_TXT, buf);

   switch (f)
   {
   case 0:
   case 1:
   case 2:
   case 4:
   case 5:
   case 8:
   case 9: d = atoi(p);
           while (*p && isdigit(*p)) p++;
           while (*p && ispunct(*p)) p++;
           if (f <= 2)
           {
              m = get_month(p);
              while (*p && isalpha(*p)) p++;
              while (*p && ispunct(*p)) p++;
              y = get_year(p);
           }
           else
           {
              m = atoi(p);
              while (*p && isdigit(*p)) p++;
              while (*p && ispunct(*p)) p++;
              y = get_year(p);
           }
           break;
   case 3:
   case 7:
   case 10: m = atoi(p);
            while (*p && isdigit(*p)) p++;
            while (*p && ispunct(*p)) p++;
            d = atoi(p);
            while (*p && isdigit(*p)) p++;
            while (*p && ispunct(*p)) p++;
            y = get_year(p);
            break;
   case 6: y = get_year(p);
           while (*p && isdigit(*p)) p++;
           while (*p && ispunct(*p)) p++;
           m = atoi(p);
           while (*p && isdigit(*p)) p++;
           while (*p && ispunct(*p)) p++;
           d = atoi(p);
           break;
   default: break;
   }

   if (!y || !m || !d)
      MsgBox("ERROR", "Invalid date!", NULL, "Continue");
   else
      ihd->date = ymd_to_julian(y, m, d);
   SetDateField(pDlg, num, ihd->date);
}

/* --------------------------------------------------------------------
                    new handler for date fields
   -------------------------------------------------------------------- */

int IhDate(int Msg, int n, void *p, DIALOG *pDlg, DLGITEM *pItm)
{
   int res;
   struct tm *t;
   int num = (int)((char *)pItm - (char *)&(pDlg->Items[0])) / sizeof(DLGITEM);

   switch (Msg)
   {
      case IM_INIT:      res = IhEdit(Msg, n, p, pDlg, pItm);
                         SetToday(pDlg, num);
                         return res;
      case IM_SETDATA:   switch (n)
                         {
                         case DTSI_JUL:
                                  SetDateField(pDlg, num, *((long *)p));
                                  return IN_ACK;
                         case DTSI_TM:
                                  t = (struct tm *)p;
                                  SetDateField(pDlg, num,
                                     ymd_to_julian(t->tm_year + 1900,
                                     t->tm_mon + 1, t->tm_mday));
                                  return IN_ACK;
                         default: return IhEdit(Msg, n, p, pDlg, pItm);
                         }
      case IM_GETDATA:   switch (n)
                         {
                         case DTGI_JUL:
                                  *((long *)p) = GetDateField(pDlg, num);
                                  return IN_ACK;
                         case DTGI_TM:
                                  t = (struct tm *)p;
                                  julian_to_ymd(GetDateField(pDlg, num),
                                     &(t->tm_year), &(t->tm_mon),
                                     &(t->tm_mday));
                                  t->tm_year -= 1900;
                                  t->tm_mon -= 1;
                                  return IN_ACK;
                         default: return IhEdit(Msg, n, p, pDlg, pItm);
                         }
      case IM_SETFOCUS:
                         if (!Fn4Bgnd)
                         {
                            Fn4Bgnd = GetImg(211, 190, 395, 199, NULL);
                            Fn6Bgnd = GetImg(335, 190, 395, 199, NULL);
                            SetColor(BLACK_COLOR);
                            SetRule(FORCE_RULE);
                            Rectangle(211, 190, 259, 199, SOLID_FILL);
                            Rectangle(335, 190, 393, 199, SOLID_FILL);
                            SetRule(XOR_RULE);
                            SelectFont(SMALL_FONT);
                            WriteText(221, 191, "Today");
                            WriteText(337, 191, "Calendr");
                         }
                         return IhEdit(Msg, n, p, pDlg, pItm);
      case IM_LOSEFOCUS:
      case IM_KILL:
                         if (Fn4Bgnd)
                         {
                            PutImg(211, 190, FORCE_RULE, Fn4Bgnd);
                            PutImg(335, 190, FORCE_RULE, Fn6Bgnd);
                            free(Fn4Bgnd);
                            free(Fn6Bgnd);
                            Fn4Bgnd = Fn6Bgnd = NULL;
                         }
                         InterpretInput(pDlg, num);
                         return IhEdit(Msg, n, p, pDlg, pItm);

      case IM_KEY:       return DateKey(n, pDlg, num);
      default:           return IhEdit(Msg, n, p, pDlg, pItm);
   }
}

