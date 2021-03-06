1800 DIALOGEX 32, 32, 288, 228
STYLE DS_MODALFRAME | DS_CONTEXTHELP | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
CAPTION "Page Setup"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
FONT 8, "MS Shell Dlg"
{
   CONTROL "Printer", 1075, BUTTON, BS_GROUPBOX | WS_CHILD | WS_VISIBLE | WS_GROUP, 8, 4, 272, 84 
   CONTROL "&Name:", 1093, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 20, 36, 8 
   CONTROL "", 1136, COMBOBOX, CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_GROUP | WS_TABSTOP, 52, 18, 152, 152 
   CONTROL "&Properties", 1025, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 212, 17, 60, 14 
   CONTROL "Status:", 1095, STATIC, SS_LEFT | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 36, 36, 10 
   CONTROL "", 1099, STATIC, SS_LEFTNOWORDWRAP | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 52, 36, 224, 10 
   CONTROL "Type:", 1094, STATIC, SS_LEFT | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 48, 36, 10 
   CONTROL "", 1098, STATIC, SS_LEFTNOWORDWRAP | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 52, 48, 224, 10 
   CONTROL "Where:", 1097, STATIC, SS_LEFT | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 60, 36, 10 
   CONTROL "", 1101, STATIC, SS_LEFTNOWORDWRAP | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 52, 60, 224, 10 
   CONTROL "Comment:", 1096, STATIC, SS_LEFT | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 72, 36, 10 
   CONTROL "", 1100, STATIC, SS_LEFTNOWORDWRAP | SS_NOPREFIX | WS_CHILD | WS_VISIBLE | WS_GROUP, 52, 72, 224, 10 
   CONTROL "Paper", 1073, BUTTON, BS_GROUPBOX | WS_CHILD | WS_VISIBLE | WS_GROUP, 8, 92, 164, 56 
   CONTROL "Si&ze:", 1089, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 108, 36, 8 
   CONTROL "", 1137, COMBOBOX, CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_GROUP | WS_TABSTOP, 52, 106, 112, 112 
   CONTROL "&Source:", 1090, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 16, 128, 36, 8 
   CONTROL "", 1138, COMBOBOX, CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_GROUP | WS_TABSTOP, 52, 126, 112, 112 
   CONTROL "Orientation", 1072, BUTTON, BS_GROUPBOX | WS_CHILD | WS_VISIBLE | WS_GROUP, 180, 92, 100, 56 
   CONTROL "", 1084, STATIC, SS_ICON | WS_CHILD | WS_VISIBLE | WS_GROUP, 190, 112, 20, 20 
   CONTROL "P&ortrait", 1056, BUTTON, BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 224, 106, 52, 12 
   CONTROL "L&andscape", 1057, BUTTON, BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE, 224, 126, 52, 12 
   CONTROL "Print Scale", 2000, BUTTON, BS_GROUPBOX | WS_CHILD | WS_VISIBLE | WS_GROUP, 8, 152, 164, 45 
   CONTROL "", 1803, STATIC, SS_ICON | WS_CHILD | WS_VISIBLE | WS_GROUP, 22, 169, 16, 16 
   CONTROL "", 1801, EDIT, ES_LEFT | ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, 52, 171, 40, 12 
   CONTROL "Spin1", 1802, "msctls_updown32", UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_CHILD | WS_VISIBLE, 159, 170, 11, 12 , 0x00000020
   CONTROL "%  (10 - 400)", -1, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 95, 173, 57, 8 
   CONTROL "OK", 1, BUTTON, BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 180, 206, 48, 14 
   CONTROL "Cancel", 2, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 232, 206, 48, 14 
   CONTROL "&Help", 1038, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 8, 206, 48, 14 
}

