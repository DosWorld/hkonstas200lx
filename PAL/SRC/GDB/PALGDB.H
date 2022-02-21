/* --------------------------------------------------------------------
   Project: PAL General Database routines
   Module:  PALGDB.H
   Author:  Harry Konstas
   Started: 13 May 96
   -------------------------------------------------------------------- */

#ifndef _GDB_H
#define _GDB_H

/* --------------------------------------------------------------------
                       standard includes
   -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

/* --------------------------------------------------------------------
                          constant definitions
   -------------------------------------------------------------------- */

/* search modes */
#define OR_LOGIC 1
#define AND_LOGIC 2
#define INC_NOTES 1
#define SKIP_NOTES 0

/* GDB specifics */
#define MAXFIELDS 100
#define HEADER_SIZE 6
#define TYPEFIRST_SIZE 64
#define CARDEF_SIZE 20
#define LUFLAG_DELETED 0x80

/* --------------------------------------------------------------------
                           enum definitions
   -------------------------------------------------------------------- */

/* GDB error messages */
typedef enum {
   OPERATION_OK=0,        /* 00 all's well...                     */
   RECORD_DELETED,        /* 01 the record is marked as 'deleted' */
   OUT_OF_MEMORY,         /* 02 out of memory (malloc)            */
   RECORD_NOT_FOUND,      /* 03 specified record is not found     */
   FREAD_ERROR,           /* 04 file read operation error         */
   NO_LOOKUP_TABLE,       /* 05 the lookup table is missing       */
   INVALID_LOOKUP,        /* 06 the lookup table is invalid       */
   INVALID_DATABASE,      /* 07 not a valid database file         */
   WORLDTIME_DATABASE,    /* 08 this is a world-time DB file      */
   APPTBOOK_DATABASE,     /* 09 this is an appointment book file  */
   NO_VIEWPTABLE,         /* 10 can't find a viewpoint table      */
   NOT_MULTICARD,         /* 11 database has only 1 card page     */
   MISSING_FIELDEFS,      /* 12 missing field definition          */
   NO_REC_DATA,           /* 13 there's no record data to process */
   CANT_OPEN_FILE,        /* 14 can't open database file          */
   UNEXPECTED_REC_TYPE,   /* 15 unexpected record type            */
   INVALID_FIELD,         /* 16 field is invalid                  */
   INVALID_FIELD_TYPE,    /* 17 non-existent field type           */
   NO_NOTE,               /* 18 data record has no note attached  */
   EMPTY_FIELD,           /* 19 field is empty                    */
   STR_FOUND,             /* 20 search has found a match          */
   STR_NOT_FOUND,         /* 21 search did not find a match       */
   INVALID_VIEWPOINT,     /* 22 viewpoint is invalid              */
   RECNUM_OUT_OF_RANGE,   /* 23 record number out of range        */
   PASSWORD_PROTECTED,    /* 24 database is password protected    */
} GDBMSGS;


/* GDB record types */
typedef enum {
   TYPE_DBHEADER    = 0,  /* Header for database.  First record in db. */
   TYPE_PASSWORD    = 1,  /* Password information record.              */
   TYPE_CARDDEF     = 4,  /* Information on the card layout.           */
   TYPE_CATEGORY    = 5,  /* List of categories for the database.      */
   TYPE_FIELDDEF    = 6,  /* Definition record for each field          */
   TYPE_VIEWPTDEF   = 7,  /* Viewpoint (sort and subset) definition    */
   TYPE_NOTE        = 9,  /* Note record                               */
   TYPE_VIEWPTTABLE = 10, /* Table of viewpoint entries                */
   TYPE_DATA        = 11, /* Data record                               */
   TYPE_LINKDEF     = 12, /* Smart clip record                         */
   TYPE_CARDPAGEDEF = 13, /* Info struct for multiple card database    */
   TYPE_USER        = 14, /* up to 30: Defined by the application      */
   TYPE_LOOKUPTABLE = 31  /* Pointers to all other records in the db.  */
} RECTYPES;


/* GDB Field types */
typedef enum {
   BYTEBOOL_FIELD  = 0,   /* Checkbox: points to a bit in a byte       */
   WORDBOOL_FIELD  = 1,   /* points to a bit in a word                 */
   STRING_FIELD    = 2,   /* Points to ASCIIZ string                   */
   PHONE_FIELD     = 3,   /* Treated as a String by the database       */
   NUMBER_FIELD    = 4,   /* Numbers are stored as ASCIIZ strings,     */
   CURRENCY_FIELD  = 5,   /* Treated as a number by the database       */
   CATEGORY_FIELD  = 6,   /* Stored as an ASCIIZ string with ';'
                             separating each selection                 */

   TIME_FIELD      = 7,   /* int: Minutes since midnight               */
   DATE_FIELD      = 8,   /* points to char array[3];
                             array[0] is Year: 0 starts at 1900
                             array[1] is Month from 0..11
                             array[2] is Day from 0..30                */

   RADIO_FIELD     = 9,   /* dataoffset points to actual offset of
                             radio button byte                         */

   NOTE_FIELD      = 10,  /* The field is a note (which resides in a
                             separate record); dataoffset points to the
                             record number of the note.  If no note is
                             attached, the record number pointed to
                             should be -1.                             */

   GROUP_FIELD     = 11,  /* Used for Groupboxes; no data is attached  */
   STATIC_FIELD    = 12,  /* Used for Static Text; no data is attached */
   MULTILINE_FIELD = 13,  /* Same as a string, but can have CR/LFs     */
   LIST_FIELD      = 14,  /* Doubles as a STATIC_FIELD                 */
   COMBO_FIELD     = 15,  /* Doubles as a STRING_FIELD                 */
   USER_FIELD      = 16,  /* The applications can define their own
                             field types starting at USER_FIELD        */
} FIELDTYPES;


/* --------------------------------------------------------------------
                           struct definitions
   -------------------------------------------------------------------- */


/* lookup table structure */
typedef struct {
   WORD    Size;          /* Size of record (including record header)  */
   WORD    Filters;       /* if bit 0x0001..0x8000 is set, indicates
                             that a switch to viewpoint 0..15 requires
                             this record to be sorted/filtered afresh.
                             The "dirty bit" for keeping track of
                             what records have been changed since last
                             visiting a viewpoint.                     */
   BYTE    Flags;         /* 128 = Deleted                             */
   BYTE    Offset[3];     /* 3 bytes of offset (0..16 Meg) Low..Highest
                             Gives disk seek address of record.        */
} PLOOKUP;


/* field definition structure */
typedef struct {
   BYTE FieldType;        /* Field type                           */
   BYTE FieldID;          /* (not used)                           */
   WORD DataOffset;       /* Offset of data into a data record    */
   BYTE Flags;            /* flags                                */
   int  Reserved;         /* Only used by some fields             */
   char Name[22];         /* field name                           */
} FIELDEF;               


/* card definition structure */
typedef struct {
   long int Class;        /* Window class handler                 */
   WORD x;                /* X position in pixels                 */
   WORD y;                /* Y position in pixels                 */
   WORD w;                /* width in pixels                      */
   WORD h;                /* height in pixels                     */
   WORD LogicalSize;      /* buffer size as opposed to screen size*/
   long int Style;        /* LHAPI window style                   */
   WORD Parent;           /* index of the parent window (top=-1   */
} CARDDEF;               


/* card page definition */
typedef struct {
   int   Reserved;
   WORD  CurrPage;        /* Number of current page      */
   WORD  PageCount;       /* Number of pages             */
   WORD  ps1;             /* field start index of page 1 */
   WORD  ps2;             /* field start index of page 2 */
   WORD  ps3;             /* field start index of page 3 */
   WORD  ps4;             /* field start index of page 4 */
   WORD  pz1;             /* number of fields in page 1  */
   WORD  pz2;             /* number of fields in page 2  */
   WORD  pz3;             /* number of fields in page 3  */
   WORD  pz4;             /* number of fields in page 4  */
} CARDPAGE;


/* smart clip (link) definition */
typedef struct {
   int  StringLenght;     /* lenght of string lenght     */
   int  Flags;            /* flags: 0x2=cannot delete    */
   char Name[32];         /* smart clip name             */
   char *pLinkString;     /* smart clip representation   */
} SCLINKDEF;


/* viewpoint definition */
typedef struct {
   int  TokenLenght;      /* lenght of SSL Token representation  */
   int  StringLenght;     /* lenght of SSL string representation */
   int  Flags;            /* Viewpoint flags                     */
   char Reserved;
   char Name[32];         /* Viewpoint name                      */
   int  SortField[3];     /* Index of fieldef to sort            */
   int  Ascending[3];     /* 1=ascending, 0=descending           */
   int  ColumnInfo[20];   /* MSB=Field# LSB=Width  (last=-1)     */
   char *pSSLtokens;      /* pointer to SSL tokens               */
   char *pSSLstring;      /* pointer to SSL string               */
} VIEWPTDEF;


/* date stamp structure */
typedef struct {
   BYTE  Year;          /*  0..199  = 1900..2099               */
   BYTE  Month;         /*  0..11   = Jan..Dec                 */
   BYTE  Day;           /*  0..30   = 1st..31st                */
   WORD  Minute;        /*  0..1439 = 12:00am..11:59pm         */
} DBTIMESTAMP;


/* record header */
typedef struct {
   BYTE Type;           /* record type                         */
   BYTE Status;         /* record status                       */
   WORD Lenght;         /* record lenght including header      */
   WORD Index;          /* sequential record index             */
} RECHEADER;              


/* database file header */
typedef struct {
   char  Signature[4];  /* database file signature= 'hcD'      */
   RECHEADER h;         /* record header                       */
   WORD  ReleaseVer;    /* database release version            */
   BYTE  FileType;      /* File type (GDB/PDB/NDB or ADB)      */
   BYTE  FileStatus;    /* 0x1= Open   0x2= modified           */
   WORD  CurrentViewpt; /* index of current viewpoint          */
   WORD  NumRecords;    /* total number of records in db file  */
   DWORD LookupSeek;    /* file offset of the lookup table     */
   DBTIMESTAMP Date;    /* date stamp of last reconcilation    */
   WORD  ViewptHash;    /* generated by codepage, language, etc*/
} DBHEADER;             


/* database description structure */
typedef struct {
   int          ErrorMsg;            /* GDB last error register*/
   char         FileName[120];       /* DB filename            */
   WORD         NumFields;           /* number of fields in DB */
   WORD         NumDataRec;          /* number of data records */
   int          NumViews;            /* number of viewpoints   */
   int          NumLinks;            /* number of linkdefs     */
   FILE        *pDBF;                /* DB FILE ptr            */
   DBHEADER    *pDBH;                /* database header ptr    */
   FIELDEF     *pFDEF[MAXFIELDS];    /* field definition array */
   CARDDEF     *pCDEF[MAXFIELDS];    /* card definition array  */
   CARDPAGE    *pCPAG;               /* card page definition   */
   PLOOKUP     *pLOOK;               /* lookup table pointer   */
   WORD        *pFTAB;               /* Type first table ptr   */
   VIEWPTDEF   *pVDEF;               /* Current Viewpoint def. */
   int         *pVTAB;               /* Current Viewpoint table*/
   SCLINKDEF   *pLINK;               /* Current Link-definition*/
   char        *pCATS;               /* Category strings ptr   */
} DBDESC;               


/* --------------------------------------------------------------------
                              prototypes
   -------------------------------------------------------------------- */

/* low level functions */
DBHEADER  *ReadDBHeader(DBDESC *pDD);
PLOOKUP   *GetLookup(DBDESC *pDD, BYTE Type, WORD RecNum);
WORD       GetRecSize(DBDESC *pDD, BYTE Type, WORD RecNum);
DWORD      GetRecOffset(DBDESC *pDD, BYTE Type, WORD RecNum);
RECHEADER *GetRecHeader(DBDESC *pDD, BYTE Type, WORD RecNum);
PLOOKUP   *ReadLookup(DBDESC *pDD);
WORD      *ReadTypeFirst(DBDESC *pDD);
int        ReadFieldefs(DBDESC *pDD);
CARDPAGE  *ReadCardPage(DBDESC *pDD);
int        ReadCardDefs(DBDESC *pDD);
char      *ReadCategory(DBDESC *pDD);
VIEWPTDEF *ReadViewDef(DBDESC *pDD, int ViewPoint);
int       *ReadViewTable(DBDESC *pDD, int ViewPoint);
int        ShowNote(int x, int y, int w, int h, BYTE *text);
void       RemoveHotkey(char *string);

/* high level functions */
DBDESC    *OpenDatabase(char *filename, int *ErrorMsg);
void       CloseDatabase(DBDESC *pDD);
int        ChangeViewPoint(DBDESC *pDD, int ViewPoint);
int        ReadLinkDef(DBDESC *pDD, int LinkDef);
int        ShowCard(DBDESC *pDD, int page);
void      *ReadRecord(DBDESC *pDD, BYTE Type, WORD RecNum);
char      *GetField(DBDESC *pDD, int FieldNum, void *pRecData);
int        ShowRecord(DBDESC *pDD, int page, void *pRecData);
int        SearchDatabase(DBDESC *pDD, int start, int end,
                    char *string1, char *string2, int logic, int nflag);

#endif

