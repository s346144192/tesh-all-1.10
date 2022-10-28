
#include <Options.h>
#include <CtrlFunc.h>
#include <CodeEdit.h>
#include <FuncList.h>
#include <commctrl.h>

#define STYLE_TYPE_BOLD 0x1
#define STYLE_TYPE_ITALIC 0x2
#define STYLE_TYPE_UNDERLINE 0x4
#define STYLE_TYPE_FORE 0x8
#define STYLE_TYPE_BACK 0x10


std::map<int, std::string> style_option_keywords = {
{SCE_JASS_WORD,"Block"},
{SCE_JASS_WORD2,"Keyword"},
{SCE_JASS_WORD3,"Value"},
{SCE_JASS_WORD4,"Type"},
{SCE_JASS_WORD5,"Constant"},
{SCE_JASS_WORD6,"Native"},
{SCE_JASS_WORD7,"BJ"},
{SCE_JASS_OPERATOR,"Operator"},
{SCE_JASS_NUMBER,"Number"},
{SCE_JASS_STRING,"String"},
{SCE_JASS_CHARACTER,"CharNumber"},
{SCE_JASS_RUNTEXTMACRO,"Runtextmacro"},
{SCE_JASS_COMMENT,"Comment"},
{SCE_JASS_COMMENTLINE,"Comment"},
{SCE_JASS_COMMENTFLAG,"Comment"},
{SCE_JASS_PREPROCESSOR,"Preprocessor Comment"},
{SCE_LUA_WORD,"Lua Keyword"},
{SCE_LUA_WORD5,"Lua JassConstant"},
{SCE_LUA_WORD6,"Lua JassNative"},
{SCE_LUA_WORD7,"Lua JassBJ"},
{SCE_LUA_OPERATOR,"Lua Operator"},
{SCE_LUA_IMPORT_FALG,"Lua Operator"},
{SCE_LUA_NUMBER,"Lua Number"},
{SCE_LUA_STRING,"Lua String"},
{SCE_LUA_CHARACTER,"Lua String2"},
{SCE_LUA_LITERALSTRING,"Lua LiteralString"},
{SCE_LUA_COMMENT,"Lua Comment"},
{SCE_LUA_COMMENTLINE,"Lua Comment"},
{SCE_LUA_COMMENTDOC,"Lua Comment"},
{STYLE_LINENUMBER,"Line Numbers"},
};

int GetKeywordStyle(const char* name) {
	if(!name)
	{
		return 0;
	}
	for (auto it = style_option_keywords.begin(); it != style_option_keywords.end();++it) {
		if (strcmp(name,(*it).second.data())==0) {
			return (*it).first;
		}
	}
	return 0;
}

CHOOSEFONTA* Options::cur_fontinfo;
BindList<HWND, Options*> Options::classlist;
BindList<HWND, StyleOptions*> StyleOptions::classlist;

int StyleOptions::color_list[25] = {
	-1,
	-1,
	0xFFFFFF,
	0x000000,
	0xFF0000,
	0x00FF00,
	0x0000FF,
	0xFFFF00,
	0x00FFFF,
	0xFF00FF,
	0xFF9900,
	0x808080,
	0xBFBFBF,
	0x3F3F3F,
	0x996633,
	0xCCFFCC,
	0x009900,
	0x8099FF,
	0x000099,
	0xFF00FF,
	0xFFCCCC,
	0x990000,
	0xFFCC00,
	0x998066,
	0,
};

LRESULT CALLBACK StyleOptions::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int id = StyleOptions::classlist.getid(hwnd);
	if (id < 0) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	StyleOptions* style_options = StyleOptions::classlist.at(id).second;
	switch (message)
	{
	case WM_SIZE:
		break;
	case WM_NOTIFY: 
		break;
	case WM_CTLCOLORSTATIC: {
		if ((HWND)lParam == style_options->_FontPreview) {
			SetTextColor((HDC)wParam, TO_RGB(style_options->GetForeSelColor()));
			SetBkColor((HDC)wParam, TO_RGB(style_options->GetBackSelColor()));
			return (INT_PTR)GetSysColorBrush(COLOR_MENU); //背景画刷
		}
		break;
	}
	case WM_DRAWITEM:{
		style_options->drawitem((LPDRAWITEMSTRUCT)lParam);
		return TRUE;
	}
	case WM_COMMAND:
	{
		WORD ctrl_id = LOWORD(wParam);
		WORD event_id = HIWORD(wParam);
		// 分析菜单选择:
		switch (ctrl_id)
		{
		case IDR_STYLEOPTIONS_LIST:
			if (event_id == LBN_SELCHANGE || event_id == LBN_SELCANCEL) {
				style_options->LoadListCurSel(false);
			}
			break;
		case IDR_STYLEOPTIONS_BOLD:
			if (event_id == BN_CLICKED) {
				style_options->SetFontBold(GetCheck(style_options->_Bold));
			}
			break;
		case IDR_STYLEOPTIONS_ITALIC:
			if (event_id == BN_CLICKED) {
				style_options->SetFontItalic(GetCheck(style_options->_Italic));
			}
			break;
		case IDR_STYLEOPTIONS_UNDERLINE:
			if (event_id == BN_CLICKED) {
				style_options->SetFontUnderline(GetCheck(style_options->_Underline));
			}
			break;
		case IDR_STYLEOPTIONS_FOREGROUND_COMBOX:
			if (event_id == CBN_SELCHANGE) {
				int idx = style_options->GetForeSelIndex();
				//自定义
				if (idx == 0) {
					style_options->SetForeColor((int)style_options->default_fore);
				}
				else if (idx==1) {
					COLORREF color= TO_BGR(style_options->custom_fore);
					if (ShowColorDialog(style_options->_hwnd
						, CC_RGBINIT| CC_ANYCOLOR
						, &color, NULL,0) == IDOK) {
						style_options->SetForeColor(TO_RGB((int)color));
					}
				}
				else if (idx>=2 && idx<sizeof(color_list)) {
					style_options->SetForeColor((int)color_list[idx]);
				}
				
			}
			break;
		case IDR_STYLEOPTIONS_BACKGROUND_COMBOX:
			if (event_id == CBN_SELCHANGE) {
				int idx = style_options->GetBackSelIndex();
				//自定义
				if (idx == 0) {
					style_options->SetBackColor((int)style_options->default_back);
				}
				else if (idx == 1) {
					COLORREF color= TO_BGR(style_options->custom_back);
					if (ShowColorDialog(style_options->_hwnd
						, CC_RGBINIT | CC_ANYCOLOR
						, &color, NULL,0) == IDOK) {
						style_options->SetBackColor(TO_RGB((int)color));
					}
				}
				else if (idx >= 2 && idx < sizeof(color_list)) {
					style_options->SetBackColor((int)color_list[idx]);
				}
			}
			break;
		case IDR_STYLEOPTIONS_ALPHA_PUT:
		case IDR_STYLEOPTIONS_ALPHA_UPDOWN:
			if (event_id == EN_CHANGE) {
				style_options->SetStyleAlpha();
			}
			break;
		case IDR_STYLEOPTIONS_VISIBLE:
			style_options->SetStyleVisible();
			break;
		default:
			break;
		}
	}
	break;
	case WM_CLOSE:
		style_options->SaveStyles();
		ShowWindow(hwnd, SW_HIDE);
		SetActiveWindow(style_options->_parent);
		return FALSE;
		break;
	case WM_DESTROY:
		break;
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);

}

void StyleOptions::SetPreviewFont(LOGFONTA* lf, bool scale_size) {
	if (preview) {
		DeleteObject(preview);
	}
	int height = lf->lfHeight;
	if (scale_size) {
		lf->lfHeight = ((float)abs(height)*GetLogPixelsScale() + 0.499);
	}
	preview = CreateFontIndirectA(lf);
	lf->lfHeight = height;
	SendMessageA(_FontPreview, WM_SETFONT, (WPARAM)preview, NULL);
	InvalidateRect(_FontPreview, NULL, TRUE);
}
void  StyleOptions::LoadCurPreviewFont() {
	if (!CodeBox || cur_sel_string.length()==0) {
		return;
	}
	int style=GetKeywordStyle(cur_sel_string.data());
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	LOGFONTA lf = {};
	pCodeBox->GetLogFontExt(style,&lf);
	SetPreviewFont(&lf,false);
}


bool StyleOptions::GetEditFont(int style, LOGFONTA* lf) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->GetLogFontExt(style, lf);
	return true;
}
bool StyleOptions::SetEditFont(int style, LOGFONTA* lf, bool scale_size) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetLogFontExt(style, lf, scale_size);
	return true;
}
void StyleOptions::SetEditFore(int style, int color) {
	if (!CodeBox) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetForeColor(style, color);
}
void StyleOptions::SetEditBack(int style, int color) {
	if (!CodeBox) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetBackColor(style, color);
}
int StyleOptions::GetEditFore(int style) {
	if (!CodeBox) {
		return 0;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	return pCodeBox->GetForeColor(style);
}
int StyleOptions::GetEditBack(int style) {
	if (!CodeBox) {
		return 0;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	return pCodeBox->GetBackColor(style);
}


void  StyleOptions::SetMatchStyleState(int style_types) {
	if (!CodeBox) {
		return;
	}
	if (cur_sel_string.length() == 0) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	int count = 0;
	for (auto it = style_option_keywords.begin(); it != style_option_keywords.end(); ++it) {
		if (strcmp((*it).second.data(), cur_sel_string.data()) == 0) {
			count++;
			if (style_types & STYLE_TYPE_BOLD) {
				pCodeBox->SetFontBold((*it).first, GetCheck(_Bold));
			}
			if (style_types & STYLE_TYPE_ITALIC) {
				pCodeBox->SetFontItalic((*it).first, GetCheck(_Italic));
			}
			if (style_types & STYLE_TYPE_UNDERLINE) {
				pCodeBox->SetFontUnderline((*it).first, GetCheck(_Underline));
			}
			if (style_types & STYLE_TYPE_FORE) {
				pCodeBox->SetForeColor((*it).first, TO_RGB(custom_fore));
			}
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SetBackColor((*it).first, TO_RGB(custom_back));
			}
		}
	}
	if (!count) {
		if (strcmp(cur_sel_string.data(), "Caret") == 0) {
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SendEditor(SCI_SETCARETLINEBACK, TO_RGB(custom_back), 0);
			}
		}
		else if (strcmp(cur_sel_string.data(), "Selection") == 0) {
			if (style_types & STYLE_TYPE_FORE) {
				pCodeBox->SendEditor(SCI_SETSELFORE, TRUE, TO_RGB(custom_fore));
			}
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SendEditor(SCI_SETSELBACK, TRUE, TO_RGB(custom_back));
			}
		}
		else if (strcmp(cur_sel_string.data(), "SearchRange") == 0) {
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SetSearchRangeBack(TO_RGB(custom_back));
			}
		}
		else if (strcmp(cur_sel_string.data(), "AnchorForWord") == 0) {
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SetAnchorForWordBack(TO_RGB(custom_back));
			}
		}
		else if (strcmp(cur_sel_string.data(), "FoldMarker") == 0) {
			if (style_types & STYLE_TYPE_FORE) {
				pCodeBox->SetMarkerFore(TO_RGB(custom_fore));
				pCodeBox->SetMarkerback2(TO_RGB(custom_fore));
			}
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SetMarkerback1(TO_RGB(custom_back));
			}
		}
		else if (strcmp(cur_sel_string.data(), "Line Numbers") == 0) {
			if (style_types & STYLE_TYPE_FORE) {
				pCodeBox->SendEditor(SCI_STYLESETFORE, STYLE_LINENUMBER, TO_RGB(custom_fore));
			}
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SendEditor(SCI_STYLESETBACK, STYLE_LINENUMBER, TO_RGB(custom_back));
			}
		}
		else if (strcmp(cur_sel_string.data(), "Other") == 0) {
			if (style_types & STYLE_TYPE_FORE) {
				pCodeBox->SetEditMainForeColor(TO_RGB(custom_fore));
			}
			if (style_types & STYLE_TYPE_BACK) {
				pCodeBox->SetEditMainBackColor(TO_RGB(custom_back));
			}
		}
	}
}


void StyleOptions::SetStyleAlpha() {
	if (!CodeBox) {
		return;
	}
	if (cur_sel_string.length() == 0) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	char balpha[32] = {};
	SendMessage(_AlphaPut, WM_GETTEXT, (WPARAM)sizeof(balpha), (LPARAM)balpha);
	simple_analys analys(balpha);
	int alpha = min(256, analys.get_int());
	if (strcmp(cur_sel_string.data(), "Caret") == 0) {
		(*styles)["Caret"][ "Alpha"]= alpha;
		pCodeBox->SendEditor(SCI_SETCARETLINEBACKALPHA, alpha, 0);
	}
	else if (strcmp(cur_sel_string.data(), "Selection") == 0) {
		(*styles)["Selection"][ "Alpha"]= alpha;
		pCodeBox->SendEditor(SCI_SETSELALPHA, alpha, 0);
	}
	else if (strcmp(cur_sel_string.data(), "SearchRange") == 0) {
		(*styles)["SearchRange"][ "Alpha"]= alpha;
		pCodeBox->SetSearchRangeAlpha(alpha);
	}
	else if (strcmp(cur_sel_string.data(), "AnchorForWord") == 0) {
		(*styles)["AnchorForWord"][ "Alpha"]= alpha;
		pCodeBox->SetAnchorForWordAlpha(alpha);
	}
}
void StyleOptions::SetStyleVisible() {
	if (!CodeBox) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	BOOL check = GetCheck(_Visible);
	if (strcmp(cur_sel_string.data(), "Caret") == 0) {
		(*styles)["Caret"]["Visible"]= check;
		pCodeBox->SendEditor(SCI_SETCARETLINEVISIBLE, check, 0);
	}
	else if (strcmp(cur_sel_string.data(), "AnchorForWord") == 0) {
		(*styles)["AnchorForWord"]["Visible"]= check;
		pCodeBox->SetAnchorForWordVisible(check==TRUE);
	}
	
	
}

void StyleOptions::SetFontBold(BOOL enable) {
	if (cur_sel_string.length() > 0) {
		(*styles)[cur_sel_string.data()]["Bold"]=enable;
		SetMatchStyleState(STYLE_TYPE_BOLD);
		LoadCurPreviewFont();
	}
}
void StyleOptions::SetFontItalic(BOOL enable) {
	if (cur_sel_string.length() > 0) {
		(*styles)[cur_sel_string.data()]["Italic"] = enable;
		SetMatchStyleState(STYLE_TYPE_ITALIC);
		LoadCurPreviewFont();
	}
}
void StyleOptions::SetFontUnderline(BOOL enable) {
	if (cur_sel_string.length() > 0) {
		(*styles)[cur_sel_string.data()]["Underline"]= enable;
		SetMatchStyleState(STYLE_TYPE_UNDERLINE);
		LoadCurPreviewFont();
	}
}

void StyleOptions::SetForeColor(int color) {
	if (cur_sel_string.length() > 0) {
		custom_fore = color;
		(*styles)[cur_sel_string.data()][ "Foreground"]=(uint32_t)custom_fore;
		SetMatchStyleState(STYLE_TYPE_FORE);
		LoadCurPreviewFont();
	}
}
void StyleOptions::SetBackColor(int color) {
	if (cur_sel_string.length() > 0) {
		custom_back = color;
		(*styles)[cur_sel_string.data()]["Background"]= (uint32_t)color;
		SetMatchStyleState(STYLE_TYPE_BACK);
		LoadCurPreviewFont();
	}
}

std::string StyleOptions::GetListString(int i) {
	std::string text = "";
	if (i < 0) {
		return text;
	}
	char name[360] = { 0 };
	int len = SendMessage(_List, LB_GETTEXT, i, (LPARAM)name);
	text = name;
	return text;
}
std::string StyleOptions::GetListCurSelString() {
	return GetListString(SendMessage(_List, LB_GETCURSEL, 0, 0));
}
int StyleOptions::FindListString(const char* name) {
	return SendMessage(_List, LB_FINDSTRING, (WPARAM)-1, (LPARAM)name);
}
void StyleOptions::SetListCurSel(int i) {
	SendMessage(_List, LB_SETCURSEL, i, 0);
	std::string text = GetListString(i);
	if (text.length() > 0) {
		ShowItems(text.data());
	}
}
void StyleOptions::SetListFindCurSel(const char* name) {
	int i = FindListString(name);
	if (i >= 0) {
		SetListCurSel(i);
	}
}
void StyleOptions::LoadListCurSel(bool or_selcet_other) {
	int select = SendMessage(_List, LB_GETCURSEL, 0, 0);
	if (select<0 && or_selcet_other) {
		select = 0;
		SendMessage(_List, LB_SETCURSEL, 0, 0);
	}
	if (select>=0) {
		cur_sel_string = GetListString(select);
		if (cur_sel_string.length() > 0) {
			ShowItems(cur_sel_string.data());
		}
	}
	else {
		cur_sel_string = "";
	}
}


int StyleOptions::GetForeSelIndex() {
	return SendMessageA(_ForegroundComBox, CB_GETCURSEL, 0, 0);
}
int StyleOptions::GetBackSelIndex() {
	return SendMessageA(_BackgroundComBox, CB_GETCURSEL, 0, 0);
}

void StyleOptions::SetForeSelIndex(int idx) {
	SendMessageA(_ForegroundComBox, CB_SETCURSEL, idx, 0);
}
void StyleOptions::SetBackSelIndex(int idx) {
	SendMessageA(_BackgroundComBox, CB_SETCURSEL, idx, 0);
}


void StyleOptions::SetForeSelColor(int color) {
	int idx = 0;
	for (int i = 2; i < sizeof(color_list);i++) {
		if (color_list[i]== color) {
			idx = i;
			break;
		}
	}
	SetForeColor(color);
	if (idx>=2) {
		SetForeSelIndex(idx);
		return;
	}
	int default_color = styles->_default.get_int(cur_sel_string.data(), "Foreground");
	if (default_color== color_list[0]) {
		SetForeSelIndex(0);
	}
	else {
		SetForeSelIndex( 1);
	}
}
int StyleOptions::GetForeSelColor() {
	int idx= GetForeSelIndex();
	if (idx < 0 && idx >= sizeof(color_list)) {
		return 0;
	}
	if (idx >= 2) {
		return color_list[idx];
	}
	if (idx ==0) {
		return styles->_default.get_int(GetListCurSelString().data(), "Foreground");
	}
	return custom_fore;
}

void StyleOptions::SetBackSelColor(int color) {
	int idx = 0;
	for (int i = 2; i < sizeof(color_list); i++) {
		if (color_list[i] == color) {
			idx = i;
			break;
		}
	}
	SetBackColor(color);
	if (idx >= 2) {
		SetBackSelIndex(idx);
		return;
	}
	int default_color = styles->_default.get_int(cur_sel_string.data(), "Background");
	if (default_color == color_list[0]) {
		SetBackSelIndex(0);
	}
	else {
		SetBackSelIndex(1);
	}
}
int StyleOptions::GetBackSelColor() {
	int idx = GetBackSelIndex();
	if (idx < 0 && idx >= sizeof(color_list)) {
		return 0;
	}
	if (idx >= 2) {
		return color_list[idx];
	}
	if (idx == 0) {
		return styles->_default.get_int(cur_sel_string.data(), "Background",0xFFFFFF);
	}
	return custom_back;
}


void StyleOptions::ShowItems(const char* name) {
	if (!styles || !name) {
		return;
	}

	ShowHwnd(_Bold, (styles->_default.get(name, "Bold") != NULL));
	ShowHwnd(_Italic, (styles->_default.get(name, "Italic") != NULL));
	ShowHwnd(_Underline, (styles->_default.get(name, "Underline") != NULL));
	ShowHwnd(_ForegroundName, (styles->_default.get(name, "Foreground") != NULL));
	ShowHwnd(_ForegroundComBox, (styles->_default.get(name, "Foreground") != NULL));
	ShowHwnd(_BackgroundName, (styles->_default.get(name, "Background") != NULL));
	ShowHwnd(_BackgroundComBox, (styles->_default.get(name, "Background") != NULL));
	ShowHwnd(_AlphaName, (styles->_default.get(name, "Alpha") != NULL));
	ShowHwnd(_AlphaPut, (styles->_default.get(name, "Alpha") != NULL));
	ShowHwnd(_AlphaUpDown, (styles->_default.get(name, "Alpha") != NULL));
	ShowHwnd(_FontPreview, (styles->_default.get(name, "Bold") != NULL));
	ShowHwnd(_Visible, (styles->_default.get(name, "Visible") != NULL));
	default_fore = 0x000000;
	default_back = 0xFFFFFF;
	custom_fore = 0x000000;
	custom_back = 0xFFFFFF;
	if (styles->_default.get(name, "Foreground") != NULL) {
		default_fore = styles->_default.get(name, "Foreground")->get_int(0x000000);
	}
	if (styles->_default.get(name, "Background") != NULL) {
		default_back = styles->_default.get(name, "Background")->get_int(0xFFFFFF);
	}
	SetCheck(_Bold, styles->get_int(name, "Bold"));
	SetCheck(_Italic, styles->get_int(name, "Italic"));
	SetCheck(_Underline, styles->get_int(name, "Underline"));
	SetForeSelColor(styles->get_rgb(name, "Foreground", 0x000000));
	SetBackSelColor(styles->get_rgb(name, "Background", 0xFFFFFF));
	SetWindowTextA(_AlphaPut, styles->get_str(name, "Alpha"));
	SetCheck(_Visible, styles->get_int(name, "Visible"));
	LoadCurPreviewFont();
}


void  StyleOptions::drawitem(LPDRAWITEMSTRUCT item) {
	//你把背景设置成了透明的，该控件重绘时原来的文字不会被擦除。（原因在于透明的背景导致其绘制都是在父窗口上）。
	//解决方法就是让该控件的父窗口Invalidate()下就可以了。
	HDC hdc = item->hDC;
	RECT rt = item->rcItem;
	//Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
	if (item->CtlType == ODT_COMBOBOX) {
		if (item->itemID>=0 && item->itemID<sizeof(color_list)) {
			HBRUSH hbr = NULL;
			HPEN pen = NULL;

			if ((item->itemAction == ODA_FOCUS)) {
				hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
				pen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
				if ((item->itemState & ODS_FOCUS)) {
					pen = CreatePen(PS_DOT, 1, RGB(102, 102, 102));
				}
				else {
					pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
				}
				SelectObject(hdc, hbr);        //选择画刷
				SelectObject(hdc, pen);        //选择画笔
				SetBkMode(hdc, TRANSPARENT);
				Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
				DeleteObject(hbr);
				DeleteObject(pen);
				InvalidateRect(item->hwndItem, NULL, TRUE);
				return;
				
			}
			else if ((item->itemAction == ODA_DRAWENTIRE)) {
				int color = color_list[item->itemID];
				color = TO_RGB(color);
				if (item->itemID == 0) {
					if (item->hwndItem== _ForegroundComBox) {
						color = TO_RGB(default_fore);
					}
					else if (item->hwndItem == _BackgroundComBox) {
						color = TO_RGB(default_back);
					}
				}
				else if (item->itemID == 1) {
					if (item->hwndItem == _ForegroundComBox) {
						color = TO_RGB(custom_fore);
					}
					else if (item->hwndItem == _BackgroundComBox) {
						color = TO_RGB(custom_back);
					}
				}
				hbr = CreateSolidBrush(color);
				pen = NULL;
				if (item->itemState & ODS_SELECTED) {
					pen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
				}
				else {
					pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				}
				int size = rt.bottom - rt.top - 2;
				SelectObject(hdc, hbr);        //选择画刷
				SelectObject(hdc, pen);        //选择画笔
				SetBkMode(hdc, TRANSPARENT);
				Rectangle(hdc, 3, rt.top + 1, size + 3, rt.bottom - 1);
				DeleteObject(hbr);
				DeleteObject(pen);

				hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
				pen = (HPEN)GetStockObject(NULL_PEN);
				RECT text_rc = { size + 6,rt.top + 1,rt.right - (size + 4),rt.bottom - 1 };
				SelectObject(hdc, hbr);        //选择画刷
				SelectObject(hdc, pen);        //选择画笔
				char buffer[100] = { 0 };
				SendMessageA(item->hwndItem, CB_GETLBTEXT, item->itemID, (LPARAM)buffer);
				//combox edit需设置颜色。
				SetTextColor(hdc, RGB(0, 0, 0));
				TextOutA(hdc,text_rc.left,text_rc.top, buffer, strlen(buffer));
				DeleteObject(hbr);
				DeleteObject(pen);
			}
		}

	}
}

bool StyleOptions::IsShow() {
	return IsWindowVisible(_hwnd);
}
void StyleOptions::Show(BOOL enable) {
	if (enable) {
		//ShowWindow(_hwnd, SW_SHOW);
		SetWindowPos(_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW |  SWP_NOSIZE | SWP_NOMOVE);
		LoadListCurSel(true);
	}
	else {
		SetWindowPos(_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		//ShowWindow(_hwnd, SW_HIDE);
	}
}

void StyleOptions::InitAppStyles() {
	if (!CodeBox || !styles) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	for (auto it = style_option_keywords.begin(); it != style_option_keywords.end(); ++it) {
		pCodeBox->SetFontBold((*it).first, styles->get_int((*it).second.data(), "Bold", 0)==1);
		pCodeBox->SetFontItalic((*it).first, styles->get_int((*it).second.data(), "Italic", 0) == 1);
		pCodeBox->SetFontUnderline((*it).first, styles->get_int((*it).second.data(), "Underline", 0) == 1);
		pCodeBox->SetForeColor((*it).first, TO_RGB(styles->get_int((*it).second.data(), "Foreground", 0x000000)));
		pCodeBox->SetBackColor((*it).first, TO_RGB(styles->get_int((*it).second.data(), "Background", 0xFFFFFF)));
	}
	pCodeBox->SendEditor(SCI_SETCARETLINEBACK, TO_RGB(styles->get_int("Caret", "Background", 0xFFFF00)) );
	pCodeBox->SendEditor(SCI_SETCARETLINEBACKALPHA, styles->get_int("Caret", "Alpha", 60), 0);
	pCodeBox->SendEditor(SCI_SETCARETLINEVISIBLE, styles->get_int("Caret", "Visible", 0));
	pCodeBox->SendEditor(SCI_SETSELFORE, TRUE, TO_RGB(styles->get_int("Selection", "Foreground", 0xFFFFFF)));
	pCodeBox->SendEditor(SCI_SETSELBACK, TRUE, TO_RGB(styles->get_int("Selection", "Background", 0x3399FF)));
	pCodeBox->SendEditor(SCI_SETSELALPHA, styles->get_int("Selection", "Alpha", 60), 0);
	pCodeBox->SetSearchRangeBack(TO_RGB(styles->get_int("SearchRange", "Background", 0x3399FF)));
	pCodeBox->SetSearchRangeAlpha(styles->get_int("SearchRange", "Alpha", 50));
	pCodeBox->SetAnchorForWordBack(TO_RGB(styles->get_int("AnchorForWord", "Background", 0x3399FF)));
	pCodeBox->SetAnchorForWordAlpha(styles->get_int("AnchorForWord", "Alpha", 50));
	pCodeBox->SetAnchorForWordVisible(styles->get_int("AnchorForWord", "Visible", 1)==1);
	pCodeBox->SetMarkerFore(TO_RGB(styles->get_int("FoldMarker", "Foreground", 0x0000FF)));
	pCodeBox->SetMarkerback2(TO_RGB(styles->get_int("FoldMarker", "Foreground", 0x00FFFF)));
	pCodeBox->SetMarkerback1(TO_RGB(styles->get_int("FoldMarker", "Background", 0x00FFFF)));
	int color = GetSysColor(COLOR_MENU);
	pCodeBox->SendEditor(SCI_STYLESETFORE, STYLE_LINENUMBER, TO_RGB(styles->get_int("Line Numbers", "Foreground", 0x000000)));
	pCodeBox->SendEditor(SCI_STYLESETBACK, STYLE_LINENUMBER, TO_RGB(styles->get_int("Line Numbers", "Background", TO_BGR(color))));
	pCodeBox->SetEditMainForeColor(TO_RGB(styles->get_int("Other", "Foreground", 0x000000)));
	pCodeBox->SetEditMainBackColor(TO_RGB(styles->get_int("Other", "Background", 0xFFFFFF)));
	
	if (IsWindowVisible(_hwnd)) {
		if (cur_sel_string.length()>0) {
			SetListFindCurSel(cur_sel_string.data());
		}
	}

}
void StyleOptions::SaveStyles() {
	if (!styles) {
		return;
	}
	styles->cur_path = dllself.getfilepath("Styles.ini");
	styles->save_config(styles->cur_path.data());
}

void StyleOptions::LoadList() {
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Block");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Keyword");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Value");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Type");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Constant");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Native");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"BJ");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Operator");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Number");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"String");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"CharNumber");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Runtextmacro");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Comment");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Preprocessor Comment");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua Keyword");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua JassConstant");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua JassNative");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua JassBJ");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua Operator");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua Number");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua String");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua String2");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua LiteralString");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Lua Comment");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Other");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"FoldMarker");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Line Numbers");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Caret");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"Selection");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"SearchRange");
	SendMessage(_List, LB_ADDSTRING, 0, (LPARAM)"AnchorForWord");

	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"默认值");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"自定义");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"白");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"黑");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"红");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"绿");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"蓝");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"黄");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"青");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"紫");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"橙");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"灰");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"淡灰");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"暗灰");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"茶");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"浅绿");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"深绿");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"淡蓝");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"深蓝");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"粉");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"浅红");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"深红");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"金");
	SendMessage(_ForegroundComBox, CB_ADDSTRING, 0, (LPARAM)"褐灰");

	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"默认值");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"自定义");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"白");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"黑");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"红");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"绿");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"蓝");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"黄");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"青");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"紫");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"橙");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"灰");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"淡灰");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"暗灰");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"茶");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"浅绿");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"深绿");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"淡蓝");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"深蓝");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"粉");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"浅红");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"深红");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"金");
	SendMessage(_BackgroundComBox, CB_ADDSTRING, 0, (LPARAM)"褐灰");

}

void StyleOptions::Init(HWND parent, uintptr_t _CodeBox, ini::config_t* _styles) {
	RECT rect;
	WinClass.Register("StyleOptionsWin", StyleOptions::WndProc);
	preview = NULL;
	_parent = parent;
	GetWindowRect(parent, &rect);
	_hwnd = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, "StyleOptionsWin", "style 选项",  WS_SYSMENU
		, max(0,rect.left + ((rect.right - rect.left) / 2 - 360)), max(0, rect.top + ((rect.bottom - rect.top) / 2 - 300)), 520, 415, parent, (HMENU)0, nullptr, nullptr);
	_List = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("ListBox"), NULL
		, WS_CHILD | WS_VISIBLE | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_VSCROLL
		, 10, 10, 200, 365, _hwnd, (HMENU)IDR_STYLEOPTIONS_LIST, nullptr, nullptr);
	_Bold = CreateWindowA("Button", "Bold", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
			, 230, 10, 100, 24, _hwnd, (HMENU)IDR_STYLEOPTIONS_BOLD, nullptr, nullptr);
	_Italic = CreateWindowA("Button", "Italic", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 230, 35, 100, 24, _hwnd, (HMENU)IDR_STYLEOPTIONS_ITALIC, nullptr, nullptr);
	_Underline = CreateWindowA("Button", "Underline", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 230, 60, 100, 24, _hwnd, (HMENU)IDR_STYLEOPTIONS_UNDERLINE, nullptr, nullptr);
	_ForegroundName = CreateWindowA("static", "Foreground", WS_CHILD | WS_VISIBLE, 230, 100, 72, 24, _hwnd, (HMENU)0, nullptr, nullptr);
	_ForegroundComBox = CreateWindowA("combobox", NULL,
		WS_CHILD | WS_VSCROLL| CBS_DROPDOWNLIST | LBS_OWNERDRAWFIXED | CBS_DISABLENOSCROLL | CBS_HASSTRINGS | WS_VISIBLE
		, 315, 100 - 4, 180, 200, _hwnd, (HMENU)IDR_STYLEOPTIONS_FOREGROUND_COMBOX, nullptr, nullptr);
	_BackgroundName = CreateWindowA("static", "Background", WS_CHILD | WS_VISIBLE, 230, 135, 72, 24, _hwnd, (HMENU)0, nullptr, nullptr);
	_BackgroundComBox = CreateWindowA("combobox", NULL, WS_CHILD  | WS_VSCROLL | CBS_DROPDOWNLIST | LBS_OWNERDRAWFIXED | CBS_DISABLENOSCROLL | CBS_HASSTRINGS | WS_VISIBLE
		, 315, 135 - 4, 180, 200, _hwnd, (HMENU)IDR_STYLEOPTIONS_BACKGROUND_COMBOX, nullptr, nullptr);
	_AlphaName = CreateWindowA("static", "Alpha", WS_CHILD | WS_VISIBLE, 230, 180, 50, 24, _hwnd, (HMENU)0, nullptr, nullptr);
	_AlphaPut = CreateUpDnBuddy(_hwnd, (HMENU)IDR_STYLEOPTIONS_ALPHA_PUT,275,180-2,60,24);
	_AlphaUpDown = CreateUpDnCtl(_hwnd, (HMENU)IDR_STYLEOPTIONS_ALPHA_UPDOWN,0,256);
	_FontPreview = CreateWindowA("static", "字体预览", WS_CHILD | SS_CENTERIMAGE | SS_LEFT | WS_VISIBLE, 230, 230, 150, 30, _hwnd, (HMENU)0, nullptr, nullptr);
	_Visible = CreateWindowA("Button", "visible", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 230, 275, 100, 30, _hwnd, (HMENU)IDR_STYLEOPTIONS_VISIBLE, nullptr, nullptr);

	//-1编辑框高度, 0下拉列表高度。
	SendMessage(_ForegroundComBox, CB_SETITEMHEIGHT, -1, 19);
	SendMessage(_ForegroundComBox, CB_SETITEMHEIGHT, 0, 18);
	SendMessage(_BackgroundComBox, CB_SETITEMHEIGHT, -1, 19);
	SendMessage(_BackgroundComBox, CB_SETITEMHEIGHT, 0, 18);

	SetCtrlFont(_List, "MS Sans Serif", 0, 16);
	SetCtrlFont(_Bold, "MS Sans Serif", 0, 16, FW_BOLD);
	SetCtrlFont(_Italic, "MS Sans Serif", 0, 16, FW_NORMAL,TRUE);
	SetCtrlFont(_Underline, "MS Sans Serif", 0, 16, FW_NORMAL, FALSE,TRUE);
	SetCtrlFont(_ForegroundName, "MS Sans Serif", 0, 16);
	SetCtrlFont(_ForegroundComBox, "MS Sans Serif", 0, 15);
	SetCtrlFont(_BackgroundName, "MS Sans Serif", 0, 16);
	SetCtrlFont(_BackgroundComBox, "MS Sans Serif", 0, 15);
	SetCtrlFont(_AlphaName, "MS Sans Serif", 0, 16);
	SetCtrlFont(_AlphaPut, "MS Sans Serif", 0, 16);
	SetCtrlFont(_FontPreview, "Courier New", 0, 16);
	SetCtrlFont(_Visible, "MS Sans Serif", 0, 16);

	classlist.add(_hwnd,this);
	default_fore = 0;
	default_back = 0;
	custom_fore = 0;
	custom_back = 0;
	styles = _styles;
	CodeBox = _CodeBox;

	LoadList();
}



LRESULT CALLBACK Options::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int id = Options::classlist.getid(hwnd);
	if (id < 0) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	Options* options = Options::classlist.at(id).second;
	if (!options) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	switch (message)
	{
	case WM_SIZE:
		break;
	case WM_NOTIFY:
		return 0;
		break;
	case WM_COMMAND:
	{
		WORD ctrl_id = LOWORD(wParam);
		WORD event_id = HIWORD(wParam);
		// 分析菜单选择:
		switch (ctrl_id)
		{
		case IDR_OPTIONS_FONT_BUTTON: {
			LOGFONTA lf = {};
			GetCtrlLogFont(options->_FontNameText,&lf);
			if (ShowFontDialog(options->_main
				, CF_EFFECTS | CF_SCALABLEONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT | CF_USESTYLE
				| CF_APPLY | CF_ENABLEHOOK | CF_FORCEFONTEXIST
				, &lf,AppFont, &options->cur_fontinfo, 1) == IDOK) {
				//已缩放高度
				options->SetEditMainFont(&lf, false);
				options->_StyleOptions.LoadCurPreviewFont();
			}
		}
		break;
		case IDR_OPTIONS_LINENUMBER_FONT_BUTTON: {
			LOGFONTA lf = {};
			options->GetEditFont(STYLE_LINENUMBER, &lf);
			if (ShowFontDialog(hwnd
				, CF_EFFECTS | CF_SCALABLEONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT | CF_USESTYLE
				| CF_APPLY | CF_ENABLEHOOK | CF_FORCEFONTEXIST
				, &lf, AppFont, &options->cur_fontinfo, 2) == IDOK) {
				//options->cur_fontinfo 不再可用
				options->SetLineNumFont(&lf, false);
			}
		}
		break;
		case IDR_OPTIONS_STYLE_BUTTON:
			options->_StyleOptions.Show(TRUE);
			break;
		case IDR_OPTIONS_LINENUMBER_CHECKBOX:
			options->SetCheckItem("LineNum", GetCheck(options->_Line_Numbers));
			break;
		case IDR_OPTIONS_CODEFOLDING_CHECKBOX:
			options->SetCheckItem("DoFold", GetCheck(options->_CodeFolding));
			break;
		case IDR_OPTIONS_AUTOCOMPLETE_CHECKBOX:
			options->SetCheckItem("AutoComplete", GetCheck(options->_AutoComplete));
			break;
		case IDR_OPTIONS_CALLTIP_CHECKBOX:
			options->SetCheckItem("CallTip", GetCheck(options->_CallTip));
			break;
		case IDR_OPTIONS_ZOOMLEVEL_CHECKBOX:
			options->SetCheckItem("ZoomLevel", GetCheck(options->_ZoomLevel));
			break;
		case IDR_OPTIONS_HIDE_CONVERTWARNING_CHECKBOX:
			options->SetCheckItem("ShowWarning", !GetCheck(options->_HideConvertWarning));
			break;
		case IDR_OPTIONS_SKIPIME_CHECKBOX:
			options->SetCheckItem("SkipIncompatibleIme", GetCheck(options->_SkipIncompatibleIme));
			break;
		case IDR_OPTIONS_LISTONTOP:
			options->SetFuncListTop(GetCheck(options->_ListOnTop));
			break;
		default:
			break;
		}
		return 0;
	}
	break;
	case WM_CLOSE:
		options->SaveConfig(false);
		options->Show(FALSE);
		return FALSE;
		break;
	case WM_DESTROY:
		break;
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);

}

UINT_PTR CALLBACK Options::AppFont(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_COMMAND)
	{
		WORD ctrl_id = LOWORD(wParam);
		WORD event_id = HIWORD(wParam);
		if (ctrl_id == 0x402 && event_id == 0) {
			int id = Options::classlist.getid(GetParent(hwnd));
			if (id<0) {
				return 0;
			}
			Options* options = Options::classlist.at(id).second;
			if (!cur_fontinfo || !options) {
				return 0;
			}
			if (cur_fontinfo->lpLogFont) {
				//得缩放大小,之后需恢复高度,否则会不断增加。
				if (cur_fontinfo->lCustData == 1) {
					options->SetEditMainFont(cur_fontinfo->lpLogFont, true);
					options->_StyleOptions.LoadCurPreviewFont();
				}
				else if (cur_fontinfo->lCustData == 2) {
					options->SetLineNumFont(cur_fontinfo->lpLogFont, true);
				}
			}
		}
	}
	else if (message == WM_NCDESTROY) {
		cur_fontinfo = NULL;
	}
	return 0;
}


void Options::SetPreviewFont(LOGFONTA* lf, bool scale_size) {
	if (preview) {
		DeleteObject(preview);
	}
	int height = lf->lfHeight;
	if (scale_size) {
		lf->lfHeight = ((float)abs(height)*GetLogPixelsScale() + 0.499);
	}
	preview = CreateFontIndirectA(lf);
	SetWindowTextA(_FontNameText, lf->lfFaceName);
	lf->lfHeight = height;
	SendMessageA(_FontNameText, WM_SETFONT, (WPARAM)preview, NULL);
	InvalidateRect(_FontNameText, NULL, TRUE);
}



bool Options::SetEditMainFont(LOGFONTA* lf, bool scale_size) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	SetPreviewFont(lf, scale_size);
	int size=ToFontSize(lf->lfHeight, scale_size);
	pCodeBox->SetEditFontExt(lf->lfFaceName, size);
	return true;

}
bool Options::SetLineNumFont(LOGFONTA* lf, bool scale_size) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	int size = ToFontSize(lf->lfHeight, scale_size);
	pCodeBox->SetLineNumFontExt(lf->lfFaceName, size);
	return true;
}
bool Options::GetEditFont(int style, LOGFONTA* lf) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->GetLogFontExt(style, lf);
	return true;
}
bool Options::SetEditFont(int style, LOGFONTA* lf, bool scale_size) {
	if (!CodeBox || !lf) {
		return false;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetLogFontExt(style, lf, scale_size);
	return true;
}
void Options::SetEditFore(int style, int color) {
	if (!CodeBox) {
		return ;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetForeColor(style, color);
}
void Options::SetEditBack(int style, int color) {
	if (!CodeBox) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	pCodeBox->SetBackColor(style, color);
}
int Options::GetEditFore(int style) {
	if (!CodeBox) {
		return 0;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	return pCodeBox->GetForeColor(style);
}
int Options::GetEditBack(int style) {
	if (!CodeBox) {
		return 0;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	return pCodeBox->GetBackColor(style);
}

void Options::SetCheckItem(const char* name, BOOL enable) {
	if (!config || !name) {
		return;
	}
	if (!CodeBox) {
		return ;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	if (strcmp(name,"DoFold")==0) {
		pCodeBox->ShowFoldExt(enable);
	}
	else if (strcmp(name, "LineNum") == 0) {
		pCodeBox->ShowLineNumberExt(enable);
	}
	else if (strcmp(name, "AutoComplete") == 0) {
		pCodeBox->EnableAutoCompleteExt(enable);
	}
	else if (strcmp(name, "CallTip") == 0) {
		pCodeBox->EnableCallTipExt(enable);
	}
	else if (strcmp(name, "ZoomLevel") == 0) {
		pCodeBox->EnableZoomLevelExt(enable);
	}
	else if (strcmp(name, "ShowWarning") == 0) {
		ShowWarning = enable;
		(*config)["Other"][ "ShowWarning"]=(int)ShowWarning;
	}
	else if (strcmp(name, "SkipIncompatibleIme") == 0) {
		SkipIncompatibleIme = enable;
		(*config)["Other"][ "SkipIncompatibleIme"]= (int)SkipIncompatibleIme;
	}
}

void Options::SetFuncListTop(bool enable) {
	if (!FuncList) {
		return;
	}
	if (enable) {
		SetWindowPos(((FuncListView*)FuncList)->_main, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	else {
		SetWindowPos(((FuncListView*)FuncList)->_main, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	if (config) {
		(*config)["Other"]["ListOnTop"] = (int)enable;
	}
}

bool Options::IsShow() {
	return IsWindowVisible(_main);
}
void Options::Show(BOOL enable) {
	if (enable) {
		LoadConfig();
		//ShowWindow(_main, SW_SHOW);
		SetWindowPos(_main, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		if (_StyleOptions.IsShow()) {
			_StyleOptions.Show(TRUE);
		}
	}
	else {
		if (_StyleOptions.IsShow()) {
			_StyleOptions.Show(FALSE);
		}
		//ShowWindow(_main, SW_HIDE);
		SetWindowPos(_main, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		if (!CodeBox) {
			return;
		}
		CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
		SetWindowPos(GetParent(pCodeBox->_main), HWND_TOP,0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
		//SetActiveWindow(_parent);
	}
}

void Options::InitAppConfig(bool appstyles) {
	if (!config || !CodeBox) {
		return;
	}
	CodeEditView* pCodeBox = (CodeEditView*)CodeBox;
	SetFuncListTop(config->get_int("Other", "ListOnTop",0)==1);
	pCodeBox->ShowFoldExt(config->get_int("Editor", "DoFold", 1) == 1);
	pCodeBox->ShowLineNumberExt(config->get_int("Editor", "LineNum", 1) == 1);
	pCodeBox->EnableAutoCompleteExt(config->get_int("Editor", "AutoComplete", 1) == 1);
	pCodeBox->EnableCallTipExt(config->get_int("Editor", "CallTip", 1) == 1);
	pCodeBox->EnableZoomLevelExt(config->get_int("Editor", "ZoomLevel", 1) == 1);
	pCodeBox->SetTabLineExt(config->get_int("Editor", "TabLine", 0)==1);
	pCodeBox->SetAutoFillSpaceExt(config->get_int("Editor", "AutoFillSpace", 0));
	pCodeBox->EnableAlwaysFoldAllExt(config->get_int("Editor", "AlwaysFoldAll", 0));
	pCodeBox->EnableSaveSrcollPosExt(config->get_int("Editor", "SaveScrollPos", 0) == 1);
	pCodeBox->EnableUTF8Ext(config->get_int("Other", "EnableUTF8", 0) == 1);
	pCodeBox->EnableD2dDrawExt(config->get_int("Other", "EnableD2dDraw", 0) == 1);
	pCodeBox->EnableEnableAntiAliasExt(config->get_int("Other", "EnableAntiAlias", 2));
	ShowWarning = (config->get_int("Other", "ShowWarning", 1) == 1);
	SkipIncompatibleIme = (config->get_int("Other", "SkipIncompatibleIme", 1) == 1);
	pCodeBox->SetEditFontExt(config->get_str("EditorFont", "Name"), config->get_int("EditorFont", "Size", 0));
	pCodeBox->SetLineNumFontExt(config->get_str("LineNumFont", "Name"), config->get_int("LineNumFont", "Size", 0));
	if (appstyles) {
		_StyleOptions.InitAppStyles();
	}
	if (IsWindowVisible(_main)) {
		LoadConfig();
	}
}
void Options::SaveConfig(bool save_styles) {
	if (!config) {
		return;
	}
	config->cur_path = dllself.getfilepath("TESH.ini");
	config->save_config(config->cur_path.data());
	if (save_styles) {
		_StyleOptions.SaveStyles();
	}
}
void Options::LoadConfig() {
	if (!config) {
		return;
	}
	SetCheck(_CodeFolding, config->get_int("Editor", "DoFold"));
	SetCheck(_Line_Numbers, config->get_int("Editor", "LineNum"));
	SetCheck(_AutoComplete, config->get_int("Editor", "AutoComplete"));
	SetCheck(_CallTip, config->get_int("Editor", "CallTip"));
	SetCheck(_ZoomLevel, config->get_int("Editor", "ZoomLevel"));
	SetCheck(_ListOnTop, config->get_int("Other", "ListOnTop") == 1);
	ShowWarning = (config->get_int("Other", "ShowWarning", 1) == 1);
	SkipIncompatibleIme = (config->get_int("Other", "SkipIncompatibleIme", 1) == 1);
	SetCheck(_HideConvertWarning, !ShowWarning);
	SetCheck(_SkipIncompatibleIme, SkipIncompatibleIme);
	
	LOGFONTA lf;
	if (GetEditFont(STYLE_DEFAULT,&lf)) {
		SetPreviewFont(&lf,false);
	}
}

void Options::Init(HWND parent, uintptr_t _CodeBox, uintptr_t _FuncList, ini::config_t* _config, ini::config_t* _styles) {
	int scrWidth = GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = GetSystemMetrics(SM_CYSCREEN);
	WinClass.Register("OptionsWin", Options::WndProc);
	_parent = parent;
	preview = NULL;
	_main = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, "OptionsWin", "选项",WS_SYSMENU
		, max(0, (scrWidth / 2 - 250)), max(0, (scrHeight / 2 - 125)), 500, 250, parent, (HMENU)0, nullptr, nullptr);
	_FontTip = CreateWindowA("static", "字体", WS_CHILD | WS_VISIBLE, 5, 20, 40, 30, _main, (HMENU)0, nullptr, nullptr );
	_FontBtn = CreateButton(_main, IDR_OPTIONS_FONT_BUTTON, WS_CHILD | WS_VISIBLE, "...", "Tahoma", 12, 45, 20 - 2, 20, 20);
	_FontNameText = CreateWindowA("static", "当前字体", WS_CHILD | WS_VISIBLE, 5, 50, 150, 30, _main, (HMENU)0, nullptr, nullptr);
	_LineNumFontTip = CreateWindowA("static", "行数字体", WS_CHILD | WS_VISIBLE, 5, 100, 70, 30, _main, (HMENU)0, nullptr, nullptr);
	_LineNumFontBtn = CreateButton(_main, IDR_OPTIONS_LINENUMBER_FONT_BUTTON, WS_CHILD | WS_VISIBLE, "...", "Tahoma", 12, 70, 100-2, 20, 20);
	_StyleOptionsBtn = CreateButton(_main, IDR_OPTIONS_STYLE_BUTTON, WS_CHILD | BS_CENTER | BS_VCENTER | WS_VISIBLE, "Style 选项", "MS Sans Serif", 16, 30, 130, 90, 30);
	_Line_Numbers = CreateWindowA("Button", "显示行数", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 20, 100, 20, _main, (HMENU)IDR_OPTIONS_LINENUMBER_CHECKBOX, nullptr, nullptr);
	_CodeFolding = CreateWindowA("Button", "显示折叠", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 45, 100, 20, _main, (HMENU)IDR_OPTIONS_CODEFOLDING_CHECKBOX, nullptr, nullptr);
	_AutoComplete = CreateWindowA("Button", "自动完成", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 70, 100, 20, _main, (HMENU)IDR_OPTIONS_AUTOCOMPLETE_CHECKBOX, nullptr, nullptr);
	_CallTip = CreateWindowA("Button", "函数提示", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 95, 100, 20, _main, (HMENU)IDR_OPTIONS_CALLTIP_CHECKBOX, nullptr, nullptr);
	_ZoomLevel = CreateWindowA("Button", "启用缩放", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 130, 100, 20, _main, (HMENU)IDR_OPTIONS_ZOOMLEVEL_CHECKBOX, nullptr, nullptr);
	_HideConvertWarning = CreateWindowA("Button", "隐藏转换为自定义代码警告", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 165, 250, 20, _main, (HMENU)IDR_OPTIONS_HIDE_CONVERTWARNING_CHECKBOX, nullptr, nullptr);
	_SkipIncompatibleIme= CreateWindowA("Button", "启动时跳过不兼容的输入法", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 180, 190, 250, 20, _main, (HMENU)IDR_OPTIONS_SKIPIME_CHECKBOX, nullptr, nullptr);
	
	_GroupBox= CreateWindowA("Button", "函数列表选项", WS_CHILD | BS_GROUPBOX | WS_VISIBLE
		, 315, 20, 150, 60, _main, (HMENU)0, nullptr, nullptr);
	_ListOnTop = CreateWindowA("Button", "窗口置前", WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, 325, 45, 100, 20, _main, (HMENU)IDR_OPTIONS_LISTONTOP, nullptr, nullptr);

	SetCtrlFont(_FontTip, "Tahoma", 0, 16);
	SetCtrlFont(_FontNameText, "Tahoma", 0, 16);
	SetCtrlFont(_LineNumFontTip, "Tahoma", 0, 16);
	SetCtrlFont(_Line_Numbers, "Tahoma", 0, 16);
	SetCtrlFont(_CodeFolding, "Tahoma", 0, 16);
	SetCtrlFont(_AutoComplete, "Tahoma", 0, 16);
	SetCtrlFont(_CallTip, "Tahoma", 0, 16);
	SetCtrlFont(_ZoomLevel, "Tahoma", 0, 16);
	SetCtrlFont(_SkipIncompatibleIme, "Tahoma", 0, 16);
	SetCtrlFont(_HideConvertWarning, "Tahoma", 0, 16);
	SetCtrlFont(_GroupBox, "Tahoma", 0, 16);
	SetCtrlFont(_ListOnTop, "Tahoma", 0, 16);

	config = _config;
	CodeBox = _CodeBox;
	FuncList = _FuncList;
	_StyleOptions.Init(_main, _CodeBox, _styles);
	classlist.add(_main,this);
}
