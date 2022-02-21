/* PAL-DIALOG RESOURCE FILE */

#include <stdlib.h>

#include "pal.h"
#include "dbvrs.h"

/*[PDE:Find]*/

IHRAINIT RAFIND3 = {
MEDIUM_FONT, 1
};

DLGITEM FindItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 10, 10,428, 14,   0L, "String 1:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 10, 34,428, 14,   0L, "&String 2:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 32, 80,  0,  0,    1, "String1 &OR String2", &RAFIND3,   IhRadio , sizeof(IHRADIO ), NULL },
{ 32, 96,  0,  0,    1, "String1 &AND String2", NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{286, 80, 15, 15,   0L, "&Inlude notes", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{141,119, 60, 25, IF_RET, "O.K.",      NULL,       IhButton, sizeof(IHBUTTON), NULL },
{241,119, 80, 25, IF_ESC, "Cancel",    NULL,       IhButton, sizeof(IHBUTTON), NULL },
{ 12, 58,426, 52,   0L, "Search Options", NULL,       IhGroup , sizeof(IHGROUP ), NULL }
};

DIALOG Find = {
458, 162,
0L,
0,
8,
FindItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:criteria]*/

IHLBINIT LBCRITERIA1 = {
MEDIUM_FONT
};

IHCBINIT LBCRITERIA2 = {
MEDIUM_FONT
};

DLGITEM criteriaItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 17,  8,179, 26,   0L, "     Field \x18\x19", &LBCRITERIA1,IhListBox, sizeof(IHLISTBOX), NULL },
{208,  8, 40, 26,   0L, "Op\x18\x19",  &LBCRITERIA2,IhListBox, sizeof(IHLISTBOX), NULL },
{264, 22,208, 14,   0L, "",          NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{301,  9,130, 10,   0L, "Criteria data", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 74, 49, 40, 25, IF_RET, "OK",        NULL,       IhButton, sizeof(IHBUTTON), NULL },
{344, 49, 80, 25, IF_ESC, "Cancel",    NULL,       IhButton, sizeof(IHBUTTON), NULL },
};

DIALOG criteria = {
490, 98,
0L,
0,
6,
criteriaItems,
DhStandard,
NULL,
NULL,
0
};

