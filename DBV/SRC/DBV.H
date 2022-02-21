/* --------------------------------------------------------------------
   Project: DBV 2.0: General Database record viewer
   Module:  DBV.H
   Author:  Harry Konstas
   Started: 13 May 96
   Notes:   Compile in Large model -ml
   -------------------------------------------------------------------- */

#ifndef _DBV_H
#define _DBV_H


/* --------------------------------------------------------------------
                      constant definitions
   -------------------------------------------------------------------- */

#define title "DBView 2.0"

#define NORMAL 1
#define FULL_SCREEN 2
#define SEARCH 3
#define FIND_ALL 4
#define OP_OFFSET 14

#define REPAINT 1
#define REFRESH 2
#define SELECT 3
#define UNSELECT 4
#define LOCATE 5

/* --------------------------------------------------------------------
                           type definitions
   -------------------------------------------------------------------- */

typedef enum {
   CMD_ABOUT = 2, CMD_QUIT,

   SUM_OP=OP_OFFSET, AVG_OP, MIN_OP, MAX_OP, EXT_OP, CRT_OP, NO_OP

} MENUCOMMANDS;

typedef enum {
   NOP_CRIT, EQUAL_CRIT, ALMOST_CRIT, DIFF_CRIT, SMALL_CRIT, SMALLEQ_CRIT,
   BIG_CRIT, BIGEQ_CRIT, ON_CRIT, OFF_CRIT
}REL_OPERATOR;

typedef struct {
    char FileName[80];
    char Path[80];
    char DumpFile[80];
    char String1[60];
    char String2[60];
    char Found1;
    char Found2;
    char BeepFlag;
    char IncNotes;
    char HorizFlag;
    char Logic;
    char Rjust;
    int  CritOper;
    int  CritField;
    int  Fpos;
    char CritData[40];
} DBVAR;           /* DBV variables */



/* --------------------------------------------------------------------
                              prototypes
   -------------------------------------------------------------------- */

void    Help(void);
void    About(void);
void    LoadConfig(DBVAR *pDBVar);
void   *Initialize(void);
char   *SelectDatabase(char *dbpath);
void    Error(char *message);

DBDESC *Interact(DBDESC *pDD, DBVAR *pDBVar, int start);

int     FindString(DBDESC *pDD, DBVAR *pDBVar, int mode);
int     DumpRecord(DBDESC *pDD, DBVAR *pDBVar, int RecNum);
void    SelectViewPoint(DBDESC *pDD);
char   *CreateItem(DBDESC *pDD, int RecNum);
void    DisplayTitles(DBDESC *pDD);
void    RecordView(DBDESC *pDD, DBVAR *pDBVar, int index);
void    ListView(DBDESC *pDD, int mode, int index);
int     SpeedLocate(DBDESC *pDD, char key, int index, int select);
int     CutHalf(DBDESC *pDD, BYTE *keybuff, int pos);
int     SelectField(DBDESC *pDD);
void    Compute(DBDESC *pDD, int operation);
void    Criteria(DBDESC *pDD);

#endif

