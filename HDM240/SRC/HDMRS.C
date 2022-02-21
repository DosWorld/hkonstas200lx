/* DIALOG RESOURCE FILE */

#include <stdlib.h>

#include "pal.h"
#include "hdmrs.h"

/*[PDE:ArgsDLG]*/
IHEDINIT EDARGSDLG1 = {
"",  120, MEDIUM_FONT
};

DLGITEM ArgsDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 19, 26,470, 14,   0L, "Enter &arguments:", &EDARGSDLG1,IhEdit  , sizeof(IHEDIT  ), NULL },
{ 89, 66,340, 10,   0L, "F8:Browse   F9:Cancel   F10:O.K.", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG ArgsDLG = {
514, 106,
0L,
0,
2,
ArgsDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:IconDLG]*/
IHEDINIT EDICONDLG1 = {
"",  78, MEDIUM_FONT
};

DLGITEM IconDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 19, 20,390, 14,   0L, "&Icons path:", &EDICONDLG1,IhEdit  , sizeof(IHEDIT  ), NULL },
{ 49, 60,340, 10,   0L, "Press F9 to cancel, F10 to accept.", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG IconDLG = {
434, 95,
0L,
0,
2,
IconDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:VolDLG]*/
DLGITEM VolDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{131, 33,  0,  0,    1, "&OFF",       NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{131, 53,  0,  0,    1, "&LOW",       NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{131, 73,  0,  0,    1, "&MEDIUM",    NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{131, 93,  0,  0,    1, "&HIGH",      NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{ 61, 13,218, 90,   0L, "Select level", NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{ 68,116,190, 10,   0L, "Use arrows to change.", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG VolDLG = {
338, 146,
0L,
3,
6,
VolDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:battDLG]*/

DLGITEM battDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 17, 12,222, 84,   0L, "Battery Voltage",   NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{257, 12,216, 54,   0L, "Battery type", NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{287, 32,  0,  0,    1, "&Alkaline",  NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{287, 52,  0,  0,    1, "&Nickel Cadmium", NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{ 27, 26,120, 10,   0L, "Main (AA's):", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 26, 44,120, 10,   0L, "Backup (Li):", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 41, 62,110, 10,   0L, "Card (a:\):", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{281, 78, 15, 15,   0L, "&Charging Enabled.", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{ 77,105,340, 10,   0L, "Press F9 to cancel, F10 to accept.", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 36, 79,110, 10,   0L, "AC adapter:", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG battDLG = {
490, 138,
0L,
7,
10,
battDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:InfoDLG]*/
DLGITEM InfoDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 91, 18,220, 10,   0L, "System Manager loaded:", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 91, 58,200, 10,   0L, "Drive A: free space:", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{141, 37,120, 10,   0L, "Free memory:", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{131, 95,238, 10,   0L, "Press any key to continue...", NULL,       IhStatic, sizeof(IHSTATIC), NULL },
{ 91, 75,200, 10,   0L, "Drive C: free space:", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG InfoDLG = {
498, 130,
0L,
0,
5,
InfoDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:OwnerDLG]*/

IHEDINIT EDOWNERDLG4 = {
"",  80, MEDIUM_FONT
};

DLGITEM OwnerDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 28, 10,464, 14,   0L, "&Name/company:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 35, 33,458, 14,   0L, "&Address:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 29, 57,464, 14,   0L, "&Phone numbers:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 99, 80,394, 14,   0L, "Topcard &file:", &EDOWNERDLG4,IhEdit  , sizeof(IHEDIT  ), NULL },
{199,103, 15, 15,   0L, "&Show Topcard", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{109,125,340, 10,   0L, "Press F9 to cancel, F10 to accept.", NULL,       IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG OwnerDLG = {
554, 154,
0L,
0,
6,
OwnerDLGItems,
DhStandard,
NULL,
NULL,
0
};

/*[PDE:AddDLG]*/

IHEDINIT EDADDDLG5 = {
"",  80, MEDIUM_FONT
};


DLGITEM AddDLGItems[] = {
/*X   Y   W   D   FLAGS     LABEL       INIT       TYPE           SIZE        PRIV */
{ 14,  6,342, 14,   0L, "Application &Name:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{420,  5, 15, 15,   0L, "&Enable Help", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{ 14, 25,570, 14,   0L, "Path + &filename:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 14, 44,570, 14,   0L, "&Command tail:", NULL,       IhEdit  , sizeof(IHEDIT  ), NULL },
{ 15, 63,570, 14,   0L, "&Help:",     &EDADDDLG5, IhEdit  , sizeof(IHEDIT  ), NULL },
{ 12, 85,112, 62,   0L, "Zoom mode", NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{137, 85,210, 62,   0L, "Display",   NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{359, 85,230, 62,   0L, "System",    NULL,       IhGroup , sizeof(IHGROUP ), NULL },
{ 33,103,  0,  0,    1, "40&x16",    NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{ 33,118,  0,  0,    1, "64x18",     NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{ 33,134,  0,  0,    1, "80x25",     NULL,       IhRadio , sizeof(IHRADIO ), NULL },
{148, 97, 15, 15,   0L, "&B/W (mono).", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{148,113, 15, 15,   0L, "&Invert (negative).", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{148,130, 15, 15,   0L, "&Dithered shading.", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{367, 98, 15, 15,   0L, "Enable &Serial port.", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{367,114, 15, 15,   0L, "&Pause upon return.", NULL,       IhCheck , sizeof(IHCHECK ), NULL },
{367,130, 15, 15,   0L, "&Open with arguments.", NULL,    IhCheck , sizeof(IHCHECK ), NULL },
{200,151,210, 10,   0L, "F9:Cancel    F10:O.K.", NULL,    IhStatic, sizeof(IHSTATIC), NULL }
};

DIALOG AddDLG = {
602, 175,
0L,
0,
18,
AddDLGItems,
DhStandard,
NULL,
NULL,
0
};

