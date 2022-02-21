/* --------------------------------------------------------------------
   Module:     GDBTOOL.H
   Author:     Gilles Kohl
   (Internet: gilles@spam.fido.de, FIDO 2:244/1106.3, CIS:100114,3146)
   Started:    23.06.1993   17:01:58
   Modified:   04.07.1993   18:29:46
   Subject:    Structure of HP100LX database files.

   Thanks to Andrew Gryc for the document that was the base of all this.

   I shamelessly included most of his comments, so that this file may
   serve as a quick reference of the GDB structure, too. Any errors in
   here are mine, not his. Andy's original text is available for download
   in the HPHAND forum on CompuServe, library 15 (Palmtop programmers)
   While this file may serve as a reference, you should read the text for
   a more 'linear' description of the GDB format, and an example dump.

   Note: Some records have variable length - these are indicated with an
   '(!)' in the last field comment. Take care when reading, writing, and
   mallocing such records, sizeof() should not be used unless you know
   what you're doing.
   -------------------------------------------------------------------- */
#pragma pack(1)

/* --------------------------------------------------------------------
                          Constant definitions
   -------------------------------------------------------------------- */

#define MAX_FIELDLEN        128

/* LHAPI window styles.  Most styles are not used. */
#define STYLE_NOSHADOW      0x00004000L
#define STYLE_COMBOEDIT     0x00008000L
#define STYLE_DATETIME      0x00010000L
#define STYLE_CHECKBOX      0x00020000L /*  Should be set for a check box */
#define STYLE_NO_PARENT_KEY 0x00040000L
#define STYLE_PUSHB_WIDTH   0x00080000L
#define STYLE_PUSHBUTTON    0x00100000L
#define STYLE_COMBOLIST     0x00200000L
#define STYLE_RADIO         0x00400000L /* Should be set for a radio button */
#define STYLE_GRAY          0x00800000L
#define STYLE_XYRELATIVE    0x01000000L
#define STYLE_WHCHAR        0x02000000L
#define STYLE_NOBORDER      0x04000000L
#define STYLE_LEAF          0x08000000L
#define STYLE_NOTIFY        0x10000000L
#define STYLE_NOFOCUS       0x20000000L
#define STYLE_SAVEUNDER     0x40000000L
#define STYLE_INVISIBLE     0x80000000L


/* Lookup table flags */
#define LUFLAG_DELETED      0x80

/* Field definition flags */
#define FIELDDEF_NODATA     0x80 /* no associated data             */
#define FIELDDEF_RESERVED   0x40 /* record is reserved (don't use) */
#define FIELDDEF_RELATIVE   0x20 /* the dataoffset does not point to the
                                    actual data, but points to a word.
                                    This word is the offset to the data. */
#define FIELDDEF_NULLTITLE  0x10 /* The window related to the fielddef
                                    has no title. */

/* SSL string tokens */
#define ERROR_TOKEN        0x00 /* (used only in parsing--not in SSL token strings) */
#define LPAREN_TOKEN       0x0c /* (used only in parsing--not in SSL token strings) */
#define RPAREN_TOKEN       0x0d /* (used only in parsing--not in SSL token strings) */

#define NOT_TOKEN          0x01 /* ( ! )  */
#define OR_TOKEN           0x02 /* ( | )  */
#define AND_TOKEN          0x03 /* ( & )  */
#define EQ_TOKEN           0x04 /* ( = )  */
#define LE_TOKEN           0x05 /* ( <= ) */
#define LT_TOKEN           0x06 /* ( < )  */
#define GE_TOKEN           0x07 /* ( >= ) */
#define GT_TOKEN           0x08 /* ( > )  */
#define NE_TOKEN           0x09 /* ( <> ) */
#define IN_TOKEN           0x0a /* ( -> ) */
#define CONTAINS_TOKEN     0x0b /* ( # )  */
#define SEPARATOR_TOKEN    0x0e /* ( , )  */
#define TO_TOKEN           0x0f /* ( .. ) */
#define NUMBER_TOKEN       0x10 /* Followed by number in ASCIIZ */
#define STRING_TOKEN       0x11 /* Followed by string in ASCIIZ */
#define NAME_TOKEN         0x12 /* Followed by Name index (int) */
#define BOOLNAME_TOKEN     0x13 /* Followed by BoolName index (int) */
#define TIME_TOKEN         0x14 /* Followed by time (int) */
#define DATE_TOKEN         0x15 /* Followed by date (3 chars) */
#define CATEGORY_TOKEN     0x16 /* Followed by Category in ASCIIZ */
#define USERTYPE_TOKEN     0x17 /* Followed by user type (char), size (int), and */
                                /* actual usertype data. */
#define STAR_TOKEN         0x1a /* ( * ) */ 
#define PLACEHOLDER_TOKEN  0x19 /* ? */
#define EOL_TOKEN          0x18 /* END OF TOKEN STRING */

/* --------------------------------------------------------------------
                             Enum definitions
   -------------------------------------------------------------------- */

/*
 * The different types of records, as identified in the RECORDHEADER
 * 
 */
typedef enum {
   TYPE_DBHEADER    = 0,  /* Header for database.  First record in db. */
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
} RECORD_TYPE;

typedef enum { NORMAL = 0, GARBAGE = 1, MODIFIED = 2 } RECORD_STATUS;

typedef enum { 
   TYPE_GENERAL   = 'D', /* General database and phone book */
   TYPE_WORLDTIME = 'W',
   TYPE_NOTETAKER = 'N',
   TYPE_APPTBOOK  = '2'
} FILE_TYPE;

typedef enum {
   BYTEBOOL_FIELD  = 0, /* Checkbox: points to a bit in a byte */
   WORDBOOL_FIELD  = 1,  /* points to a bit in a word */
   STRING_FIELD    = 2, /* Points to ASCIIZ string             */
   PHONE_FIELD     = 3, /* Treated as a String by the database */

   NUMBER_FIELD    = 4, /* Numbers are stored as ASCIIZ strings,
                         but compare as numbers               */
   CURRENCY_FIELD  = 5, /* Treated as a number by the database */

   CATEGORY_FIELD  = 6, /* Stored as an ASCIIZ string with ';'
                          separating each selection           */

   TIME_FIELD      = 7, /* int: Minutes since midnight         */
   DATE_FIELD      = 8, /* points to char array[3];
                          array[0] is Year: 0 starts at 1900
                          array[1] is Month from 0..11
                          array[2] is Day from 0..30          */

   RADIO_FIELD     = 9, /* dataoffset points to actual offset of
                          radio button byte                   */

   NOTE_FIELD      = 10, /* The field is a note (which resides in a
                           separate record); dataoffset points to the
                           record number of the note.  If no note is
                           attached, the record number pointed to
                           should be -1. */

   GROUP_FIELD     = 11, /* Used for Groupboxes; no data is attached  */
   STATIC_FIELD    = 12, /* Used for Static Text; no data is attached */
   MULTILINE_FIELD = 13, /* Same as a string, but can have CR/LFs     */
   LIST_FIELD      = 14, /* Doubles as a STATIC_FIELD                 */
   COMBO_FIELD     = 15, /* Doubles as a STRING_FIELD                 */
   USER_FIELD      = 16, /* The applications can define their own
                           field types starting at USER_FIELD NOTE:
                           The FIELDDEF for user defined fields
                           MUST have the status bit set to
                           FIELDDEF_CALLBACK and a FieldCallback
                           function provided. */
} FIELD_TYPE;

/* --------------------------------------------------------------------
                          Structure definitions
   -------------------------------------------------------------------- */

/************************** RECORDHEADER ************************
 *
 * A RECORDHEADER structure precedes every record in the database
 * (including the DBHEADER, but not the signature).
 *
 */
typedef struct {
   char         Type;    /* see RECORD_TYPE   enum */
   char         Status;  /* see RECORD_STATUS enum */
   unsigned int Length;  /* Length of record *including* header
                            a zero byte record would have 6 as the
                            length). */
   int          Number;  /* Sequential record index.  Each
                            record type uses its own index
                            starting at 0. */
} RECORDHEADER;


/************************** DBTIMESTAMP *************************
 *
 * The timestamp structure is used in the database header for
 * reconciliation.
 * The date portion (bytes 0..2) and the time portion (bytes 3..4) are
 * used separately in subset strings.
 *
 */
typedef struct {
   char    Year;   /*  0..199  = 1900..2099        */
   char    Month;  /*  0..11   = Jan..Dec          */
   char    Day;    /*  0..30   = 1st..31st         */
   int     Minute; /*  0..1439 = 12:00am..11:59pm  */
} DBTIMESTAMP;

/************************* PACKEDWINDOW ************************
 *
 */
typedef struct {
   long WinClass;    /* Window class handler.  Reconstructed
                        upon loading from fielddef information. */
   int  x;           /* X position in pixels (0..639)           */
   int  y;           /* Y position in pixels (0..199)           */
   int  w;           /* width in pixels                         */
   int  h;           /* height in pixels                        */
   int  LogicalSize; /* Size of buffer for edit-type fields (as
                        opposed to screen size).  Includes null
                        terminator.
                        For radio fields, an index of the radio type.
                        For check boxes, a bit mask for that check on. */
   long Style;       /* LHAPI window style.  Most styles are not used.
                        See STYLE_* defines */
   int  Parent;      /* Index of the parent window (all windows
                        within a group box give the index of the
                        group as their parent).  -1 if no parent
                        (top level window). */

} PACKEDWINDOW;


/************************** COLUMN INFO *************************
 *
 * Used for column arrangment in viewpoint definition
 *
 */
typedef struct {
   char Number;   /* Index of fielddef to use for this column */
   char Width;    /*  Width of column in characters.          */
} COLUMNINFO;


/************************* PACKED LOOKUP *************************
 *
 * Every record in the database has an entry in the lookup table.
 *
 */
typedef struct {
   int     Size;      /* Size of record (including record header) */
   int     Filters;   /* if bit 0x0001..0x8000 is set, indicates
                         that a switch to viewpoint 0..15 requires
                         this record to be sorted/filtered afresh.
                         The "dirty bit" for keeping track of
                         what records have been changed since last
                         visiting a viewpoint. */
   char    Flags;     /* 128 = Deleted */
   char    Offset[3]; /* 3 bytes of offset (0..16 Meg) Lowest..Highest
                         Gives disk seek address of record. */
} LOOKUP;


/************************** SIGNATURE ***************************
 *
 * The first 4 bytes in the file are the Database Signature.
 *
 */
typedef struct {
   char FileId[4];
} SIGNATURE;

#define HPSIGNATURE "hcD"

/**************************** DB HEADER *************************
 * Only one of these records is in a database.  It immediately
 * follows the signature.
 *
 */
typedef struct {
   RECORDHEADER RecHdr;     /* Although the format of a RECORDHEADER
                                  is above, specifically for this
                                  record it is:
                                  0x00 0x00 0x19 0x00 0x00 0x00       */
   int          ReleaseVer;    /* Database release ver (0x0102)       */
   char         FileType;      /* see FILE_TYPE enum                  */
   char         FileStatus;    /* Bits : 0x01=Database already open
                                         0x02=A record modified since
                                              last reconcile          */

   int          CurrentViewpt; /* Record num of current viewpoint    */
   int          NumRecords;    /* Count of all records in database   */
   long         LookupSeek;    /* File offset of lookup table.  If 0,
                                  it means that the lookup table is
                                  missing (i.e., the user has rebooted
                                  before the file was closed), and it
                                  must be reconstructed by walking
                                  through the database and reading 
                                  record headers.                    */
   DBTIMESTAMP  LastReconcile; /* When database was last reconciled
                                  or creation date if never reconciled). */

   int          ViewptHash;    /* "Magic" hash generated from the
                                   following system settings:
                                   Country, CodePage, Sort,
                                   Keyboard, Language, KEYBEZ active.
                                   Normally (on a US 100LX with no changes
                                   to these settings) this is 0x8437.
                                   Used to keep viewpt tables from 
                                   going out of date if system sort 
                                   order changes. If on DB_Open 
                                   this number mismatches, all 
                                   viewpoint tables are flagged as 
                                   invalid and are recreated as needed. */
} DBHEADER;


/************************** CARDDEF  ***************************
 *
 * Contains the positioning information for the windows in
 * the database card.  All records (even on multiple pages)
 * share the same carddef; the card page def record details
 * the spliting of fields into multiple pages.  There is only
 * one record of this type.
 *
 */
typedef struct {
   RECORDHEADER RecHdr;  /*   See RECORDHEADER structure above.
                               0x04 0x?? 0x?? 0x?? 0x00 0x00     */

   PACKEDWINDOW Windows[1]; /*  ..n (!) Array of PACKED windows for each
                                field in the database.  The
                                relationship is one-to-one with
                                the FIELDDEF records.  The number
                                of PACKEDWINDOWS is computed by
                                the number of FIELDDEF records. */
} CARDDEF; /* NOTE: record of variable size, extends beyond definition */


/************************** FIELDDEF ***************************
 *
 * Defines each field in the database.  There is one FIELDDEF
 * record for every field (even non-data fields).  The
 * FIELDDEF records refer to the position of information in
 * DATA records in the file.  The database engine uses the
 * information in the FIELDDEFS to provide sort, subsetting,
 * and string retrieval functions.  Although in the general
 * databases, all information in the data records is pointed to
 * by FIELDDEFs, DATA records can contain additional
 * information.  This allows you to keep binary or non-user
 * accessible info in the records (the appointment book does
 * this with record links).
 *
 */
typedef struct {
   RECORDHEADER RecHdr; /* See RECORDHEADER structure above.
                              0x06 0x?? 0x?? 0x?? 0x00 0x??     */

   char      FieldType;    /* See FIELD_TYPE enum */

   char      FieldID;      /* Used to identify unique fields by
                              applications--not used. */

   int       DataOffset;   /* Offset of data into data record.  If info
                              is at fixed position in record, do not set
                              FIELDDEF_RELATIVE.  If in a variable position,
                              set FIELDEF_RELATIVE.  Then DataOffset points
                              at a fixed place in the record where the true
                              offset of the data (a int) can be found. */
   char      Flags;        /* See the FIELDDEF_* constants */
   int       Reserved;     /* Only used by some fields.
                              BYTEBOOL_FIELD and
                              WORDBOOL_FIELD : bitmask of bit to check
                              RADIO_FIELD : value to stuff into data byte
                              CATEGORY_FIELD : record number of associated
                              CATEGORY_TYPE rec */

    char     Name[20];     /* Space for 20 chars with trailing 0. */
} FIELDDEF;

 
/************************ CARDPAGEDEF **************************
 *
 * This record is only present if there is more than one card per
 * record in the database.  You cannot have this record if there is only
 * one card in the database.
 * 
 */
typedef struct {
   RECORDHEADER RecHdr;       /*  See RECORDHEADER structure above.
                                     0x0d 0x?? 0x28 0x00 0x00 0x00 */

   char         Reserved[20];    /* PWINDOW, whatever it is             */
   int          CurrPage;        /* Used by Card to track current page  */
   int          PageCount;       /* 2..4 for number of pages            */
   char         PageStart[4];    /* Index of control starting each page */
   char         PageSize[4];     /* Number of controls on each page     */
} CARDPAGEDEF;


/************************** CATEGORY ***************************
 *
 * Holds all the categories for the database.  A null terminated
 * string, with semicolons separating each category.  Maximum size
 * of the string is 256 bytes.
 * 
 */
typedef struct {
   RECORDHEADER RecHdr;       /* See RECORDHEADER structure above.
                                    0x05 0x?? 0x?? 0x?? 0x00 0x00 */
   char CategoryString[1];    /* (!) chars e.g. "Fred;Jolly" */
} CATEGORY;

/*************************** DATA ****************************
 *
 * A data record.  The data in the record is accessed by the fielddefs. The 
 * layout below is not required, but is what is output by the built-in 
 * applications (except the appointment book).  The first few bytes are the 
 * fixed fields.  They are in the same place in every record.  They are 
 * radio buttons, checkboxes, times, and dates.  Also in this group are 
 * relative fields--all the string fields generated by the native 100LX 
 * apps are relative.  The relative fields give the real offset of the
 * string in the record so that unused space can be packed out.
 * 
 */
typedef struct {
   RECORDHEADER RecHdr;       /* See RECORDHEADER structure above.
                                    0x0b 0x?? 0x?? 0x?? 0x?? 0x?? */
   int Fixed[1];                 /* Either fixed fields or pointers to
                                    variable fields. (!) Number of entries
                                    is determined by the fielddefs. */

/*
   n+1    Null placeholder   char    A 0 byte.  All null strings in the
                                     record are compressed to point to
                                     this single byte, instead of putting
                                     in multiple nulls.

   n+2..m Relative fields    char    Null terminated strings.  The number
                                     is determined by the fielddefs.      */
} DATARECORD;


/*************************** NOTE ****************************
 *
 * Record attached to a data record.  Each note field has its own record.
 * In the 100LX, only data records with notes have note records;
 * the Omnibook 300 has note records for every data record (even
 * if the note is empty).
 */
typedef struct {
   RECORDHEADER RecHdr;        /* See RECORDHEADER structure above.
                                     0x09 0x?? 0x?? 0x?? 0x?? 0x??  */
   char Note[1];                  /* (!) chars From 0..32767 characters.  
                                     The note cannot have characters 0x00 
                                     or 0xff. */
} NOTE;


/********************* SMART CLIP DEFINITION *******************
 * 
 * Smart-clip (also called link-def) definitions are used for
 * smart-clipping and printing. At least one must be present to print.
 * 
 */
typedef struct {
   RECORDHEADER RecHdr;     /* See RECORDHEADER structure above.
                                  0x0c 0x?? 0x?? 0x?? 0x?? 0x?? */
   int          Length;        /* Length of Link string (following name) */
   int          Flags;         /* Flags:
                                  0x02 = Linkdef can't be deleted by user */
   char         Name[32];      /*  Name */
   char         LinkString[1]; /* (!) Info representing the smart-clip. */

/* Link strings are byte sequences rather than zero-terminated strings.
   They contain SCII text and GLED special sequence strings. The ASCII 
   text is what the user enters between fields when defining a smart clip, 
   e.g. field names, blanks, newlines etc. Field descriptions 
   are enclosed in 0xff bytes - think of them as the triangles that the 
   user sees when viewing/editing a smart clip. 

   They have the following format:

  Byte#    Contents/meaning
  -----    ---------------------------------------------------------------
    0       FF : starting delimiter
    1       0A : length of the rest of the string
    2       40 : GLED field type, 0x40 means defined by app. calling GLED
    3       6F : identifies field as a smartclip
    4       00 : subfunction field (always 0)
    5       xx : width of the field in the smart clip display
    6       xx : field number (as described in FIELDDEF record)
    7       xx : optional width set by user. Zero if not set.
    8       xx : Flags:
                 Bits 0 and 1:   01 --> left-justify field
                                 10 --> center field
                                 11 --> right-justify field
   
                        Bit 2:   ???
                        Bit 3:   suppress line if empty
                        Bit 4:   Reverse name.

    9       09 : length of field minus one for backwards traversal
   10       FF : terminating delimiter.
   
   Many thanks to Everett Kaser for helping document this.

*/

} LINKDEF;


/******************** VIEW POINT DEFINITION ********************
 * Every viewpoint (that is, sort/filter/column arrangement combination)
 * has a viewpoint definition.  It describes the columns and their widths,
 * the sort fields, and the SSL string for filter matching.  Only
 * 16 viewpoints can be created.
 *
 */
typedef struct {
   RECORDHEADER RecHdr;     /* See RECORDHEADER structure above.
                                  0x07 0x?? 0x?? 0x?? 0x?? 0x00 */
   int          TokenLength;   /* Length of SSL Token representation */
   int          StringLength;  /* Length of SSL String representation */
   int          Flags;         /* Flags for the viewpoint:
       1   Viewpoint Unnamed       Viewpoint is the "Unnamed viewpoint"
       2   Viewpoint NoDelete      Viewpoint cannot be deleted by user
       4   Viewpoint General       Viewpoint was entered as SSL string, 
                                   not Card */
   char         Reserved;       /* ? */
   char         Name[32];       /* Name of viewpoint */
   int          SortField[3];   /* Index of fielddef to sort; -1 means no
                                    more fields. */
   int          Ascending[3];   /* 1=ascending,0=descending for above
                                   corresponding Sort Fields */
   COLUMNINFO   ColumnArrange[20]; /*  Array of column information;  
                                       -1 is last column */
   char Tokens[1];              /* (!) SSL Tokens in RPN-style --length is
                                        given by Token Length. */

   /* n+1..m SSL String             String form of SSL statement */
} VIEWPOINT;


/************************ LOOKUP TABLE ***************************
 * This table gives the disk address and type of every record in the 
 * database.  If it is absent (and the LookupSeek in the header is 0), then 
 * the database engine will recreate on opening by stepping through the 
 * file.  It is therefore not crucial that it is present.
 * 
 */
typedef struct {
   RECORDHEADER RecHdr;     /* See RECORDHEADER structure above.
                                   0x1f 0x?? 0x?? 0x?? 0x00 0x00 */

   LOOKUP       Lookup[1];      /* (!) entries for disk records */
} LOOKUPTABLE;

/* the LOOKUPTABLE is always the last record in the database (if present).
 * If it is there, it is followed by a small table called the TypeFirst
 * table that has no record header.  This table gives the beginning
 * lookup entries for each group of record types.  The correct lookup entry
 * to use for Record Type X, Record Number Y is given by looking at
 * Lookup Entry [TypeFirst[X] + Y].
 *
 */
typedef int TYPEFIRST[32]; /* First lookup entry for record types 0..31. */


/*************************** GDB descriptor **********************
 * The GDB descriptor is not part of the Database format, rather
 * it is used to hold information on a currently open DB. (gk)
 * 
 */
typedef struct {
   int          f;          /* the file handle                       */
   DBHEADER     Hdr;        /* the database header                   */
   LOOKUPTABLE *pLuTab;     /* lookup table (includes RecHdr)        */
   LOOKUP      *pIdx;       /* the actual lookup table               */
   TYPEFIRST   Tf;          /* the typefirst table                   */
   FIELDDEF    *pFld;       /* the field definition array            */
   int          FldCnt;     /* the number of fields in the DB        */
   LINKDEF    **pLnk;       /* the linkdef array                     */
   int          LnkCnt;     /* the number of clips in the DB         */
   int         *pViewPtTab; /* Pointer to currently loaded viewpt tb */
   int          VptCnt;     /* Number of records in current viewpt   */
   VIEWPOINT   *pViewPoint; /* Pointer to currently loaded viewpoint */
} GDB;


/* --------------------------------------------------------------------
                             Prototypes
   -------------------------------------------------------------------- */

void   Fatal(char *Fmt, ...);
void   NormalizeName(char *d, char *s);
int    FieldNameCmp(char *f1, char *f2);
void   CheckSignature(GDB *pDb);
void   ReadRecHdr(GDB *pDb, RECORDHEADER *pHdr);
void  *ReadRec(GDB *pDb, void *pDest, unsigned Size);
void   Expect(void *pRec, RECORD_TYPE Type);
void   GetDbHeader(GDB *pDb);
void   Seek2Record(GDB *pDb, RECORD_TYPE RecType, int Nbr, int *pSize);
void  *GetRecord(GDB *pDb, void *pDest, RECORD_TYPE RecType, int Nbr);
void  *GetDataRecord(GDB *pDb, void *pDest, int Nbr);
void   GetIndex(GDB *pDb);
void   GetFields(GDB *pDb);
void   GetLinks(GDB *pDb);
void   GetViewPtTab(GDB *pDb, int n);
int    GetDataField(char *buf, GDB *pDb, DATARECORD *pDat, int n);
void   GetColumns(char *buf, GDB *pDb, DATARECORD *pDat);
void   GetColumnTitles(char *buf, GDB *pDb);
int    CompareDataFields(GDB *pDb, DATARECORD *pDat, int n, char *Val, int l);
void   ShowDataFields(GDB *pDb, DATARECORD *pDat, FILE *File, int LinkNbr);
GDB   *OpenGdb(char *DbName);
void   CloseGdb(GDB *pDb);

/* --------------------------------------------------------------------
                             Externals
   -------------------------------------------------------------------- */
extern void (*GdbFatalMsg)(char *);
extern char  *TimeFormat;   /* Time format from CFG file */
extern char  *DateFormat;   /* Time format from CFG file */

