1412 DIALOGEX 0, 0, 160, 65
STYLE DS_FIXEDSYS | DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
CAPTION "Properties"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
FONT 8, "Tahoma"
{
   CONTROL "&Label:", 4512, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 7, 9, 24, 8 
   CONTROL "", 4096, "RichEdit20WPT", 0x50014080, 33, 7, 120, 12 
   CONTROL "Size:", 4513, STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE | WS_GROUP, 7, 25, 20, 8 
   CONTROL "", 4097, "RichEdit20WPT", 0x58014880, 33, 23, 120, 12 
   CONTROL "", 4514, STATIC, SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 7, 39, 146, 1 
   CONTROL "OK", 1, BUTTON, BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 26, 45, 50, 14 
   CONTROL "Cancel", 2, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 83, 45, 50, 14 
   CONTROL "", 4873, "NativeFontCtl", 0x50000030, 0, 0, 0, 0 
}
