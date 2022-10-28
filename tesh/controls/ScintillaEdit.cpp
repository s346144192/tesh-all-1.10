#include <dll_info.h>
#include <base/encode/convert_utf_8.h>
#include <base/hook/fp_call.h>
#include <text/simple_analys.h>
#include <ScintillaEdit.h>
#include <CtrlFunc.h>

#pragma comment(lib, "SciLexer.lib")


void ScintillaStyle::tologfont(LOGFONTA* lf) {
	if (!lf) {
		return;
	}
	ZeroMemory(lf, sizeof(LOGFONTA));
	strcpy_s(lf->lfFaceName, sizeof(lf->lfFaceName), name);
	lf->lfHeight = ToFontHeight(size);
	lf->lfWeight = (bold) ? 700 : 400;
	lf->lfItalic = italic;
	lf->lfUnderline = underline;
}
void ScintillaStyle::copylogfont(LOGFONTA* lf) {
	if (!lf) {
		return;
	}
	strcpy_s(name, sizeof(name), lf->lfFaceName);
	size = ToFontSize(lf->lfHeight);
	bold = (lf->lfWeight > 400);
	italic = (lf->lfItalic != 0);
	underline = (lf->lfUnderline != 0);
}

BOOL ScintillaEdit::IsEnableUTF8 = FALSE;
bool ScintillaEdit::registered = false;
bool ScintillaEdit::editor_count = 0;

ScintillaEdit::ScintillaEdit() {
	_module = NULL;
	fold_mk_idx = 0;
	fold_mk_width = 0;
	line_num_mk_idx = 0;
	search_range_indicator = -1;
	_search_id = 0;
	_search_pos = -1;
	_search_range.x = 0;
	_search_range.y = 0;
	_searching = false;
	_replaceing = false;
	_search_want_update = false;
	_is_search_range = false;
	anchor_word_indicator = - 1;
	editor_count = editor_count + 1;
	if (!registered) {
		registered = Scintilla_RegisterClasses(GetModuleHandle(NULL));
	}
}

ScintillaEdit::~ScintillaEdit() {
	editor_count = editor_count - 1;
	if (registered && editor_count==0) {
		Scintilla::ResourcesRelease(true);
		registered = false;
	}
	if (pSciLexer) {
		FreeLibrary(pSciLexer);
	}
}
void* ScintillaEdit::PointerFromWindow(HWND hwnd) noexcept {
	return reinterpret_cast<void *>(::GetWindowLongPtr(hwnd, 0));
}

void ScintillaEdit::ChangeBackground(int color) {

	/*
	 RECT rect;
	 GetClientRect(hwnd, &rect);
	 int cxClient = rect.right;
	 int cyClient = rect.bottom;
   */
	HDC hdc = GetDC(_hwnd);//获取当前的显示设备上下文
	int w = 200;
	int h = 100;
	color = color & 0xFFFFFF;
	char r = (color & 0xFF);
	char g = (color & 0xFFFF)>>8;
	char b = (color & 0xFFFFFF)>>16;
	char* rgb = (char*)malloc(3 * w*h);
	for (int i = 0; i < w*h; i++)
	{
		rgb[i * 3 + 0] = r;
		rgb[i * 3 + 1] = g;
		rgb[i * 3 + 2] = b;
	}


	HDC  hdcsource = CreateCompatibleDC(NULL);//创建存放图象的显示缓冲
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, w, h);

	SelectObject(hdcsource, bitmap); //将位图资源装入显示缓冲
	SetStretchBltMode(hdcsource, COLORONCOLOR);

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biSizeImage = 0;

	StretchDIBits(hdcsource,
		0, 0, w, h,
		0, 0, w, h,
		rgb, &bmi, DIB_RGB_COLORS, SRCCOPY);

	BitBlt(hdc, 0, 0, w, h, hdcsource, 0, 0, SRCCOPY);//将图象显示缓冲的内容直接显示到屏幕

	DeleteObject(bitmap);
	DeleteDC(hdcsource);
	ReleaseDC(_hwnd, hdc);
	free(rgb);
}
BOOL ScintillaEdit::InitEditBox(DWORD exdwStyle, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance) {
	if (_hwnd) {
		return TRUE;
	}
	if (!registered) {
		return FALSE;
	}
	char dll_path[MAX_PATH] = { 0 };
	//LinkDllPath("SciLexer.dll", dll_path, MAX_PATH);
	//pSciLexer = LoadLibraryA(dll_path);
	//if (!pSciLexer) {
	//	return FALSE;
	//}
	_hwnd = CreateWindowExW(exdwStyle, L"Scintilla", NULL, dwStyle,
		X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, nullptr);
	if (!_hwnd) {
		return FALSE;
	}
	_module = (ScintillaWin*)PointerFromWindow(_hwnd);
	fnDirect = (SciFnDirect)SendMessageA(_hwnd, SCI_GETDIRECTFUNCTION, 0, 0);
	ptrDirect = (sptr_t)SendMessageA(_hwnd, SCI_GETDIRECTPOINTER, 0, 0);
	//ChangeBackground(RGB(0,0,0));
	return TRUE;
}
void ScintillaEdit::ResetEditBox(bool reset_btn_state) {
	//search_range_indicator = -1;
	_search_id = 0;
	_search_pos = -1;
	_search_range.x = 0;
	_search_range.y = 0;
	_searching = false;
	_replaceing = false;
	_search_want_update = false;
	if (reset_btn_state) {
		_search_flag = SCFIND_NONE;
		_is_search_range = false;
	}

	anchor_word = "";

	calltip_start_pos = -1;
	calltip_text.clear();

	last_bracelight_state = 0;
	last_bracelight_pos = 0;

	//SetCallTipCancel();
	//AutoCompleteCancel();
	ResetAutoComplete();
	SetIndicClearAll();
	SetsSearchStringsClear();
	HideAnchorForWordIndic();
}

sptr_t ScintillaEdit::SendEditor(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
	return fnDirect(ptrDirect, iMessage, wParam, lParam);
}
void ScintillaEdit::SetCallbackMessageCommand(MessageCommandCallback* proc) {
	Scintilla::SetCallbackMessageCommand(_module, proc);
}
void ScintillaEdit::SetCallbackMessageNotify(MessageNotifyCallback* proc) {
	Scintilla::SetCallbackMessageNotify(_module, proc);
}
void ScintillaEdit::SetRedraw() {
	Scintilla::SetRedraw(_module);
}

void ScintillaEdit::SetTechNology(int _type) {
	SendEditor(SCI_SETTECHNOLOGY, _type);
}
int ScintillaEdit::GetTechNology() {
	return SendEditor(SCI_GETTECHNOLOGY);
}

void ScintillaEdit::SetBufferedDraw(BOOL enable) {
	SendEditor(SCI_SETBUFFEREDDRAW, enable);
}
void ScintillaEdit::SetFontLocale(const char* localeName) {
	//SendEditor(SCI_SETFONTLOCALE,0, localeName);
}
void ScintillaEdit::EnableUTF8(BOOL enable) {
	if (enable != IsEnableUTF8) {
		(enable) ? SetCodePage(SC_CP_UTF8) : SetCodePage(SC_CP_GBK);
	}
	IsEnableUTF8 = enable;
}
void ScintillaEdit::EnableD2dDraw(BOOL enable) {
	if (enable) {
		if (GetTechNology() != SC_TECHNOLOGY_DIRECTWRITE) {
			SetTechNology(SC_TECHNOLOGY_DIRECTWRITE);
			//if (GetTechNology() == SC_TECHNOLOGY_DIRECTWRITE) {
			//	//SetBufferedDraw(FALSE); //感觉不更改比较好。
			//	SetFontQuality(SC_EFF_QUALITY_ANTIALIASED);
			//}
		}
	}
	else {
		if (GetTechNology() != SC_TECHNOLOGY_DEFAULT) {
			SetTechNology(SC_TECHNOLOGY_DEFAULT);
			//SetBufferedDraw(TRUE);
		}
	}
}
void ScintillaEdit::SetFontQuality(int fontQuality) {
	if (GetFontQuality()!= fontQuality) {
		SendEditor(SCI_SETFONTQUALITY, fontQuality);
	}
}
int ScintillaEdit::GetFontQuality() {
	return SendEditor(SCI_GETFONTQUALITY);
}

void ScintillaEdit::SetReadonly(BOOL enable) {
	SendEditor(SCI_SETREADONLY, enable);
}
void ScintillaEdit::SetLexer(sptr_t sclex_t) {
	SendEditor(SCI_SETLEXER, sclex_t); //语法解析
}
void ScintillaEdit::SetCodePage(sptr_t codeid) {
	SendEditor(SCI_SETCODEPAGE, codeid);
}
//SC_CHARSET_ANSI ,SC_CHARSET_GB2312
void ScintillaEdit::SetCharSet(sptr_t _style, sptr_t charset) {
	SendEditor(SCI_STYLESETCHARACTERSET, _style, charset);
}

void ScintillaEdit::SetBraceStyle(const char* fontname, bool default_sytle, sptr_t color, sptr_t bad_color, BOOL bold) {
	SetFont(STYLE_BRACELIGHT, fontname, default_sytle);
	SetFont(STYLE_BRACEBAD, fontname, default_sytle);
	SendEditor(SCI_STYLESETFORE, STYLE_BRACELIGHT, color);
	SendEditor(SCI_STYLESETFORE, STYLE_BRACEBAD, bad_color);
	SetFontBold(STYLE_BRACELIGHT, bold);
	SetFontBold(STYLE_BRACEBAD, bold);
}
void ScintillaEdit::SetBraceLight(int stpos, int endpos) {
	Scintilla::SetBraceLight(_module, stpos, endpos);
	//SendEditor(SCI_BRACEHIGHLIGHT, stpos, endpos);
}
void ScintillaEdit::SetBraceBadLight(int stpos) {
	Scintilla::SetBraceLight(_module, stpos, -1);
	//SendEditor(SCI_BRACEBADLIGHT, stpos);
}
void ScintillaEdit::SetBraceFore(sptr_t fcolor) {
	SetForeColor(STYLE_BRACELIGHT, fcolor);
	SetForeColor(STYLE_BRACEBAD, fcolor);
}
void ScintillaEdit::SetBraceBack(sptr_t bcolor) {
	SetBackColor(STYLE_BRACELIGHT, bcolor);
	SetBackColor(STYLE_BRACEBAD, bcolor);
}
void ScintillaEdit::SetPosBraceBold(BOOL enable) {
	SetFontBold(STYLE_BRACELIGHT, enable);
	SetFontBold(STYLE_BRACEBAD, enable);
}
void ScintillaEdit::SetPosBraceItalic(BOOL enable) {
	SetFontItalic(STYLE_BRACELIGHT, enable);
	SetFontItalic(STYLE_BRACEBAD, enable);
}
void ScintillaEdit::SetPosBraceUnderline(BOOL enable) {
	SetFontUnderline(STYLE_BRACELIGHT, enable);
	SetFontUnderline(STYLE_BRACEBAD, enable);
}
bool ScintillaEdit::IsBrace(char ch) {
	switch (ch)
	{
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
		return true;
	default:
		break;
	}
	return false;
}
int ScintillaEdit::SetBraceMatch(int pos, int length) {
	return Scintilla::SetBraceMatch(_module, pos, length);
    //return SendEditor(SCI_BRACEMATCH, pos, 16);
}
void ScintillaEdit::UpCurBraceFont() {
	if (last_bracelight_pos >= 0) {
		if (GetStyleAt(last_bracelight_pos) >= SCE_LUA_DEFAULT) {
			SetPosBraceBold(GetFontBold(SCE_LUA_OPERATOR));
			SetPosBraceItalic(GetFontItalic(SCE_LUA_OPERATOR));
			SetPosBraceUnderline(GetFontUnderline(SCE_LUA_OPERATOR));
		}
		else {
			SetPosBraceBold(GetFontBold(SCE_JASS_OPERATOR));
			SetPosBraceItalic(GetFontItalic(SCE_JASS_OPERATOR));
			SetPosBraceUnderline(GetFontUnderline(SCE_JASS_OPERATOR));
		}
	}
}
void ScintillaEdit::UpCurBraceBackColor() {
	if (last_bracelight_pos >= 0) {
		if (GetStyleAt(last_bracelight_pos) >= SCE_LUA_DEFAULT) {
			SetBraceBack(GetBackColor(SCE_LUA_OPERATOR));
		}
		else {
			SetBraceBack(GetBackColor(SCE_JASS_OPERATOR));
		}
	}
}
// SCN_CHARADDED 事件时新输入style=0,相对括号一般style=操作符,不能匹配。
void ScintillaEdit::UpCurBrace() {
	int pos = GetCurrentPos(); //取得当前位置
	if (last_bracelight_pos == pos && last_bracelight_state != 0) {
		UpCurBraceFont();
		return;
	}
	last_bracelight_pos = pos;
	if (pos >= 0) {
		char ch = GetCharAt(pos);
		if (!ch || !IsBrace(ch)) {
			pos--;
			ch = GetCharAt(pos);
			if (!ch || !IsBrace(ch)) {
				if (last_bracelight_state & 0x1)
				{
					last_bracelight_state = 0;
					SetBraceLight(-1, -1);
				}
				return;
			}
		}
		UpCurBraceFont();
		last_bracelight_state = 0x1;
		int brace_pos = SetBraceMatch(pos,16);// SendEditor(SCI_BRACEMATCH, pos, 16);  //length不是括号间距。
		if (brace_pos >= 0) {
			SetBraceLight(pos, brace_pos);
		}
		else {
			SetBraceBadLight(pos);
		}
	}
}


void ScintillaEdit::SetGuideStyle(sptr_t _type, sptr_t fcolor, sptr_t bcolor) {
	if (_type >= 0 && _type <= 3) {
		SendEditor(SCI_SETINDENTATIONGUIDES, _type); //enum IndentView { ivNone, ivReal, ivLookForward, ivLookBoth };
	}
	if (fcolor >= 0) {
		SendEditor(SCI_STYLESETFORE, STYLE_INDENTGUIDE, fcolor);
	}
	if (bcolor >= 0) {
		SendEditor(SCI_STYLESETBACK, STYLE_INDENTGUIDE, bcolor);
	}
	//似乎是选中状态?
	//SendEditor(SCI_SETHIGHLIGHTGUIDE, 1);
}
void ScintillaEdit::SetStyleHotspot(sptr_t style, sptr_t enable) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETHOTSPOT, style, enable);
	//fnDirect(ptrDirect, SCI_STYLESETHOTSPOT, style, enable);
}
std::string ScintillaEdit::GetHotspotWord(int pos) {
	std::string str;
	int style = GetStyleAt(pos);
	int startpos = pos;
	int endpos = pos;
	if (style == GetStyleAt(pos - 1)) {
		startpos = GetWordStartPos(pos);
	}
	if (style == GetStyleAt(pos + 1)) {
		endpos = GetWordEndPos(startpos);
	}
	int len = endpos - startpos;
	if (startpos >= 0 && len > 0 && len < 360) {
		char* word = (char*)malloc(len + 5);
		ZeroMemory(word, sizeof(word));
		/*Sci_TextRange tr;
		tr.chrg.cpMin = startpos;
		tr.chrg.cpMax = endpos;
		tr.lpstrText = word;*/
		//SendEditor(SCI_GETTEXTRANGE, 0, sptr_t(&tr));
		GetTextRange(word, startpos, endpos);
		str.assign(word);
		free(word);
		word = nullptr;
	}
	return str;
}
void ScintillaEdit::SetZoomLevelMinMax(int min, int max) {
	SendEditor(SCI_SETZOOMLEVELMINMAX, min, max);
}
void ScintillaEdit::SetCurrentLineStyle(BOOL enable, sptr_t bcolor, sptr_t balpha) {
	SendEditor(SCI_SETCARETLINEVISIBLE, enable, 0);
	SendEditor(SCI_SETCARETLINEBACK, bcolor, 0);
	SendEditor(SCI_SETCARETLINEBACKALPHA, balpha, 0);

}
void ScintillaEdit::SetAutoComplete(sptr_t width, sptr_t maxcount) {
	SendEditor(SCI_AUTOCSETMAXWIDTH, width);
	SendEditor(SCI_AUTOCSETMAXHEIGHT, maxcount);
}
void ScintillaEdit::AutoCompleteShow(int curlen, const char* list) {
	Scintilla::AutoCompleteShow(_module, curlen, list);
	//SendEditor(SCI_AUTOCSHOW, curlen, (sptr_t)list);
}
void ScintillaEdit::AutoCompleteUpdate() {
	Scintilla::AutoCompleteUpdate(_module);
}
void ScintillaEdit::AutoCompleteCancel() {
	if (AutoCompleteListCount>0) {
		ResetAutoComplete();
		Scintilla::AutoCompleteCancel(_module);
		//SendEditor(SCI_AUTOCCANCEL);
	}
}

bool ScintillaEdit::AutoCompleteActive() {
	return Scintilla::AutoCompleteActive(_module);
	//return SendEditor(SCI_AUTOCACTIVE);
}
void ScintillaEdit::ResetAutoComplete() {
	AutoCompleteListCount = 0;
	AutoCompleteList.clear();
}

void ScintillaEdit::SetKeywords(sptr_t index, const char* keywords) {
	SendEditor(SCI_SETKEYWORDS, index, (sptr_t)keywords);
}
void ScintillaEdit::SetTabWidth(sptr_t _width) {
	SendEditor(SCI_SETTABWIDTH, _width);
}
void ScintillaEdit::SetSelectStyle(BOOL enable, sptr_t fcolor, sptr_t bcolor, sptr_t alpha) {
	SendEditor(SCI_SETSELFORE, enable, fcolor);
	SendEditor(SCI_SETSELBACK, enable, bcolor);
	//SC_ALPHA_NOALPHA = 256
	SendEditor(SCI_SETSELALPHA, alpha, 0);
}
void ScintillaEdit::SetAdditionalSelectStyle(sptr_t fcolor, sptr_t bcolor, sptr_t alpha) {
	SendEditor(SCI_SETADDITIONALSELFORE, fcolor);
	SendEditor(SCI_SETADDITIONALSELBACK, bcolor);
	//SC_ALPHA_NOALPHA = 256
	SendEditor(SCI_SETADDITIONALSELALPHA, alpha, 0);
}
void ScintillaEdit::SetDefaultStyles(const char* fontname, sptr_t fontsize, sptr_t fcolor, sptr_t bcolor) {
	SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)"Courier New");
	SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT, fontsize);
	SendEditor(SCI_STYLESETFORE, STYLE_DEFAULT, fcolor);
	SendEditor(SCI_STYLESETBACK, STYLE_DEFAULT, bcolor);
}
void ScintillaEdit::EnableLineNumber(sptr_t mkIndex, sptr_t width) {
	line_num_mk_idx = mkIndex;
	SendEditor(SCI_SETMARGINTYPEN, mkIndex, SC_MARGIN_NUMBER);
	SendEditor(SCI_SETMARGINWIDTHN, mkIndex, width);
}
void ScintillaEdit::SetLineNumberWidth(sptr_t width) {
	SendEditor(SCI_SETMARGINWIDTHN, line_num_mk_idx, width);
}
void ScintillaEdit::EnableFold(sptr_t mkIndex, sptr_t mkwidth, bool show_underline, bool enable) {
	fold_mk_idx = mkIndex;
	fold_mk_width = mkwidth;
	SendEditor(SCI_SETPROPERTY, (sptr_t)"fold", (enable) ? (sptr_t)"1" : (sptr_t)"0");
	//SendEditor(STYLE_LINENUMBER, SC_MARGIN_BACK, RGB(0xFF, 0x64, 0x64));//
	SendEditor(SCI_SETMARGINTYPEN, mkIndex, SC_MARGIN_SYMBOL);//页边类型  
	SendEditor(SCI_SETMARGINMASKN, mkIndex, SC_MASK_FOLDERS); //页边掩码  
	SendEditor(SCI_SETMARGINWIDTHN, mkIndex, mkwidth); //页边宽度  
	SendEditor(SCI_SETMARGINSENSITIVEN, mkIndex, enable); //响应鼠标消息  
	SendEditor(SCI_SETFOLDFLAGS, (show_underline) ? 16 : 0, 0); //如果折叠就在折叠行的上下各画一条横线  16 | 4 
	// 折叠标签样式  
	//SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");

	//SendEditor(SCI_SETFOLDLEVEL,  (LPARAM)1);
	//外部可展开状态图标"+"
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
	//外部可折叠状态图标"-"
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
	//内部可展开状态图标"+"
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
	//内部可折叠状态图标"-"
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
	//中间段落尾部连接
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
	//中间每行连接
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
	//外部函数尾部连接
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);

}
void ScintillaEdit::SetFoldWidth(sptr_t mkwidth) {
	if (fold_mk_idx>0) {
		fold_mk_width = mkwidth;
		SendEditor(SCI_SETMARGINWIDTHN, fold_mk_idx, mkwidth); //页边宽度  
	}
}
void ScintillaEdit::ShowFold(bool enable) {
	SendEditor(SCI_SETPROPERTY, (sptr_t)"fold", (enable) ? (sptr_t)"1" : (sptr_t)"0");
	if (enable) {
		SendEditor(SCI_SETMARGINWIDTHN, fold_mk_idx, fold_mk_width);
	}
	else {
		SendEditor(SCI_SETMARGINWIDTHN, fold_mk_idx, 0);
	}
}

void ScintillaEdit::SetMarkerFore(sptr_t color) {
	SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, color);
	SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, color);
	SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, color);
	SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, color);
}
void ScintillaEdit::SetMarkerback1(sptr_t color) {
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, color);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, color);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, color);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, color);
}
void ScintillaEdit::SetMarkerback2(sptr_t color) {
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, color);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, color);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, color);
}
sptr_t ScintillaEdit::GetStyleState(sptr_t style, ScintillaStyle*  sstate) {
	if (!sstate) {
		return 0;
	}
	ZeroMemory(sstate, sizeof(ScintillaStyle));
	SendEditor(SCI_STYLEGETFONT, style, (sptr_t)sstate->name);
	sstate->size = GetFontSize(style);
	sstate->bold = GetFontBold(style);
	sstate->italic = GetFontItalic(style);
	sstate->underline = GetFontUnderline(style);
	sstate->fore = GetForeColor(style);
	sstate->back = GetBackColor(style);
	return 1;
}
void ScintillaEdit::SetStyleState(sptr_t style, ScintillaStyle*  sstate) {
	if (!sstate) {
		return;
	}
	SendEditor(SCI_STYLESETFONT, style, (sptr_t)sstate->name);
	SetFontSize(style, sstate->size);
	SetFontBold(style, sstate->bold);
	SetFontItalic(style, sstate->italic);
	SetFontUnderline(style, sstate->underline);
	SetForeColor(style, sstate->fore);
	SetBackColor(style, sstate->back);
}
sptr_t ScintillaEdit::GetLogFontExt(sptr_t style, LOGFONTA* lf, bool use_utf8) {
	if (!lf) {
		return 0;
	}
	ZeroMemory(lf, sizeof(LOGFONT));
	std::string font_name = GetFontName(style, use_utf8);
	strncpy_s(lf->lfFaceName, LF_FACESIZE, font_name.data(), font_name.length());
	lf->lfHeight = ToFontHeight(GetFontSize(style));
	lf->lfWeight = (GetFontBold(style)) ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (GetFontItalic(style) == 1);
	lf->lfUnderline = (GetFontUnderline(style) == 1);
	return 1;
}
void ScintillaEdit::SetLogFontExt(sptr_t style, LOGFONTA*  lf, bool scale_size) {
	if (!lf) {
		return;
	}
	SetFontName(style, lf->lfFaceName);
	int height = lf->lfHeight;
	if (scale_size) {
		height = ((float)abs(lf->lfHeight)*GetLogPixelsScale() + 0.499);
	}
	SetFontSize(style, ToFontSize(height));
	SetFontBold(style, lf->lfWeight > FW_NORMAL);
	SetFontItalic(style, lf->lfItalic);
	SetFontUnderline(style, lf->lfUnderline);
}

sptr_t ScintillaEdit::GetForeColor(sptr_t _type) {
	return Scintilla::StyleGetMessage( _module, SCI_STYLEGETFORE, _type, 0);
	//return SendEditor(SCI_STYLEGETFORE, _type, 0);
}
sptr_t ScintillaEdit::GetBackColor(sptr_t _type) {
	return Scintilla::StyleGetMessage(_module, SCI_STYLEGETBACK, _type, 0);
	//return SendEditor(SCI_STYLEGETBACK, _type, 0);
}
sptr_t ScintillaEdit::GetFontSize(sptr_t _type) {
	return Scintilla::StyleGetMessage(_module, SCI_STYLEGETSIZE, _type, 0);
	//return SendEditor(SCI_STYLEGETSIZE, _type, 0);
}
sptr_t ScintillaEdit::GetFontBold(sptr_t _type) {
	return Scintilla::StyleGetMessage(_module, SCI_STYLEGETBOLD, _type, 0);
	//return SendEditor(SCI_STYLEGETBOLD, _type, 0);
}
sptr_t ScintillaEdit::GetFontItalic(sptr_t _type) {
	return Scintilla::StyleGetMessage(_module, SCI_STYLEGETITALIC, _type, 0);
	//return SendEditor(SCI_STYLEGETITALIC, _type, 0);
}
sptr_t ScintillaEdit::GetFontUnderline(sptr_t _type) {
	return Scintilla::StyleGetMessage(_module, SCI_STYLEGETUNDERLINE, _type, 0);
	//return SendEditor(SCI_STYLEGETUNDERLINE, _type, 0);
}
void ScintillaEdit::SetFont(sptr_t _type, const char* name, bool default_sytle) {
	SetFontName(_type, name);
	if (default_sytle) {
		SetForeColor(_type, GetForeColor(STYLE_DEFAULT));
		//SetBackColor(_type, GetBackColor(STYLE_DEFAULT));
		SetFontSize(_type, GetFontSize(STYLE_DEFAULT));
		SetFontBold(_type, GetFontBold(STYLE_DEFAULT));
		SetFontItalic(_type, GetFontItalic(STYLE_DEFAULT));
		SetFontUnderline(_type, GetFontUnderline(STYLE_DEFAULT));
	}
}
void ScintillaEdit::SetFont(sptr_t _type, const char* name, sptr_t _size, sptr_t _bold, sptr_t _italic, sptr_t _underline, sptr_t color) {
	SetFontName(_type, name);
	SetForeColor(_type, color);
	SetBackColor(_type, GetBackColor(STYLE_DEFAULT));
	SetFontSize(_type, _size);
	SetFontBold(_type, _bold);
	SetFontItalic(_type, _italic);
	SetFontUnderline(_type, _underline);

}
bool  ScintillaEdit::SetFontName(sptr_t _type, const char* name) {
	if (!name) {
		return false;
	}
	if (strlen(name) == 0)
	{
		return false;
	}
	if (!base::is_utf8(name, LF_FACESIZE)) {
		CHAR lfFaceName[LF_FACESIZE] = { 0 };
		base::G2U(name, lfFaceName, LF_FACESIZE);
		Scintilla::StyleSetMessage(_module, SCI_STYLESETFONT, _type, (sptr_t)lfFaceName);
		//SendEditor(SCI_STYLESETFONT, _type, (sptr_t)lfFaceName);
	}
	else {
		Scintilla::StyleSetMessage(_module, SCI_STYLESETFONT, _type, (sptr_t)name);
		//SendEditor(SCI_STYLESETFONT, _type, (sptr_t)name);
	}
	return true;
}
std::string ScintillaEdit::GetFontName(sptr_t _type, bool use_utf8) {
	CHAR lfFaceName[LF_FACESIZE] = { 0 };
	Scintilla::StyleGetMessage(_module, SCI_STYLEGETFONT, _type, (sptr_t)lfFaceName);
	//SendEditor(SCI_STYLEGETFONT, _type, (sptr_t)lfFaceName);
	if (!use_utf8 && strlen(lfFaceName) > 0) {
		CHAR newName[LF_FACESIZE] = { 0 };
		base::U2G(lfFaceName, newName, LF_FACESIZE);
		return std::string(newName);
	}
	return std::string(lfFaceName);
}

void ScintillaEdit::SetFontStyles(sptr_t _type, sptr_t _size, sptr_t _bold, sptr_t _italic, sptr_t _underline, sptr_t color) {
	SetForeColor(_type, color);
	SetFontSize(_type, _size);
	SetFontBold(_type, _bold);
	SetFontItalic(_type, _italic);
	SetFontUnderline(_type, _underline);
}
void ScintillaEdit::SetForeColor(sptr_t _type, sptr_t color) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETFORE, _type, color);
	//SendEditor(SCI_STYLESETFORE, _type, color);
}
void ScintillaEdit::SetBackColor(sptr_t _type, sptr_t color) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETBACK, _type, color);
	if (_type == SCE_JASS_OPERATOR || _type == SCE_LUA_OPERATOR) {
		UpCurBraceBackColor();
	}
	//SendEditor(SCI_STYLESETBACK, _type, color);
}
void ScintillaEdit::SetFontSize(sptr_t _type, sptr_t size) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETSIZE, _type, size);
	//SendEditor(SCI_STYLESETSIZE, _type, size);
}
void ScintillaEdit::SetFontBold(sptr_t _type, sptr_t enable) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETBOLD, _type, enable);
	//SendEditor(SCI_STYLESETBOLD, _type, enable);
	if (_type == SCE_JASS_OPERATOR || _type == SCE_LUA_OPERATOR) {
		UpCurBraceFont();
	}
}
void ScintillaEdit::SetFontItalic(sptr_t _type, sptr_t enable) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETITALIC, _type, enable);
	//SendEditor(SCI_STYLESETITALIC, _type, enable);
	if (_type == SCE_JASS_OPERATOR || _type == SCE_LUA_OPERATOR) {
		UpCurBraceFont();
	}
}
void ScintillaEdit::SetFontUnderline(sptr_t _type, sptr_t enable) {
	Scintilla::StyleSetMessage(_module, SCI_STYLESETUNDERLINE, _type, enable);
	//SendEditor(SCI_STYLESETUNDERLINE, _type, enable);
	if (_type == SCE_JASS_OPERATOR || _type == SCE_LUA_OPERATOR) {
		UpCurBraceFont();
	}
}
int ScintillaEdit::GetStyleAt(sptr_t pos) {
	return Scintilla::GetStyleAt(_module,(int)pos);
	//return SendEditor(SCI_GETSTYLEAT, pos);
}
int ScintillaEdit::GetPosStyleFontSize(sptr_t pos) {
	return GetFontSize(GetStyleAt(pos));
}
BOOL ScintillaEdit::GetPosStyleBold(sptr_t pos) {
	return GetFontBold(GetStyleAt(pos));
}

void ScintillaEdit::SetIndicStyle(sptr_t indicator, sptr_t indicatorStyle) {
	SendEditor(SCI_INDICSETSTYLE, indicator, indicatorStyle);
}
void ScintillaEdit::SetIndicStyle(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha) {
	SetIndicStyle(indicator, indicatorStyle);
	SendEditor(SCI_INDICSETFORE, indicator, fcolor);
	SendEditor(SCI_INDICSETALPHA, indicator, alpha);
	SendEditor(SCI_INDICSETOUTLINEALPHA, indicator, outline_alpha);
}
void ScintillaEdit::SetIndicFore(sptr_t indicator, sptr_t fcolor) {
	SendEditor(SCI_INDICSETFORE, indicator, fcolor);
}
void ScintillaEdit::SetIndicAlpha(sptr_t indicator, sptr_t alpha) {
	SendEditor(SCI_INDICSETALPHA, indicator, alpha);
}
//INDIC_ROUNDBOX和INDIC_STRAIGHTBOX矩形
void ScintillaEdit::SetIndicOutLineAlpha(sptr_t indicator, sptr_t outline_alpha) {
	SendEditor(SCI_INDICSETOUTLINEALPHA, indicator, outline_alpha);
}
//当鼠标悬停
void ScintillaEdit::SetIndicHoverStyle(sptr_t indicator, sptr_t indicatorStyle) {
	SendEditor(SCI_INDICSETHOVERSTYLE, indicator, indicatorStyle);
}
void ScintillaEdit::SetIndicHoverFore(sptr_t indicator, sptr_t fcolor) {
	SendEditor(SCI_INDICSETHOVERFORE, indicator, fcolor);
}
//预设置，设置指定范围时会使用当前Indic style。
void ScintillaEdit::SetCurrentIndic(sptr_t indicator) {
	Scintilla::SetCurrentIndic(_module, indicator);
	//SendEditor(SCI_SETINDICATORCURRENT, indicator);
}
void ScintillaEdit::SetCurrentIndicValue(sptr_t value) {
	Scintilla::SetCurrentIndicValue(_module, value);
	//SendEditor(SCI_SETINDICATORVALUE, value);
}
sptr_t ScintillaEdit::GetCurrentIndicValue() {
	return Scintilla::GetCurrentIndicValue(_module);
	//return SendEditor(SCI_GETINDICATORVALUE);
}
sptr_t ScintillaEdit::GetIndicAtValue(int indicator, int pos) {
	return Scintilla::GetIndicAtValue(_module, indicator, pos);
	//return SendEditor(SCI_INDICATORVALUEAT, indicator, pos);
}
sptr_t ScintillaEdit::GetIndicStart(int indicator, int pos) {
	return Scintilla::GetIndicStart(_module, indicator, pos);
	//SendEditor(SCI_INDICATORSTART, indicator, pos);
}
sptr_t ScintillaEdit::GetIndicEnd(int indicator, int pos) {
	return Scintilla::GetIndicEnd(_module, indicator, pos);
	// SendEditor(SCI_INDICATOREND, indicator, pos);
}
sptr_t ScintillaEdit::GetIndicStart(int indicator, int pos, sptr_t value) {
	int start = GetIndicStart( indicator, pos);
	if (GetIndicAtValue(indicator, pos) == value) {
		return start;
	}
	if (GetIndicAtValue(indicator, start) != value) {
		//向后搜索一次
		start = GetIndicEnd(indicator, pos);
		if (GetIndicAtValue(indicator, start) == value) {
			return start;
		}
	}
	start = GetIndicStart( indicator, start);
	if (start >= 0) {
		return start;
	}
	return -1;
}
sptr_t ScintillaEdit::GetIndicEnd(int indicator, int pos, sptr_t value) {
	int end = GetIndicEnd( indicator, pos);
	if (GetIndicAtValue(indicator, pos) == value) {
		return end;
	}
	if (GetIndicAtValue(indicator, end) != value) {
		//向前搜索一次
		end = GetIndicStart(indicator, pos);
		if (GetIndicAtValue(indicator, end) == value) {
			return end;
		}
	}
	end = GetIndicEnd(indicator, end);
	if (end >= 0) {
		return end;
	}
	return -1;
}
//指定范围(a->b), 删除字符会保留style。
void ScintillaEdit::SetIndicRange(sptr_t start, sptr_t length) {
	Scintilla::SetIndicRange(_module, start, length);
	//SendEditor(SCI_INDICATORFILLRANGE, start, length);
}
void ScintillaEdit::SetIndicRange(sptr_t indicator, sptr_t start, sptr_t length) {
	SetCurrentIndic(indicator);
	SetIndicRange(start, length);
}
void ScintillaEdit::SetIndicRange(sptr_t indicator, sptr_t start, sptr_t length, sptr_t value) {
	SetCurrentIndic(indicator);
	SetCurrentIndicValue(value);
	SetIndicRange(start, length);
}
void ScintillaEdit::SetIndicClearRange(sptr_t start, sptr_t lengthClear) {
	Scintilla::SetIndicClearRange(_module, start, lengthClear);
	//SendEditor(SCI_INDICATORCLEARRANGE, start, lengthClear);
}
void  ScintillaEdit::SetIndicClearRange(sptr_t indicator, sptr_t start, sptr_t value) {
	int st = GetIndicStart(indicator, start, value);
	if (st >= 0) {
		int end = GetIndicEnd(indicator, st, value);
		if (end >= 0)
		{
			SetCurrentIndic(indicator);
			SetIndicClearRange(st, end - st);
		}
	}
}

//重新打开文档,需重置。(SendEditor(SCI_INDICATORCLEARRANGE, 0, GetTextLength()); 可能无效)
void ScintillaEdit::SetIndicClearAll() {
	SetIndicClearRange( 0, GetTextLength());
}

void ScintillaEdit::SetSearchRangeIndic(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha) {
	search_range_indicator = indicator;
	SetIndicStyle(indicator, indicatorStyle, fcolor, alpha, outline_alpha);
}
void ScintillaEdit::SetSearchRangeBack(int color) {
	if (search_range_indicator >= 0) {
		SendEditor(SCI_INDICSETFORE, search_range_indicator, color);
	}
}
void ScintillaEdit::SetSearchRangeAlpha(int alpha) {
	if (search_range_indicator >= 0) {
		SendEditor(SCI_INDICSETALPHA, search_range_indicator, alpha);
		SendEditor(SCI_INDICSETOUTLINEALPHA, search_range_indicator, alpha);
	}
}
void ScintillaEdit::SetAnchorForWordIndic(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha){
	anchor_word_indicator = indicator;
	SetIndicStyle(indicator, indicatorStyle, fcolor, alpha, outline_alpha);
}
void ScintillaEdit::SetAnchorForWordBack(int color) {
	if (anchor_word_indicator >= 0) {
		SendEditor(SCI_INDICSETFORE, anchor_word_indicator, color);
	}
}
void ScintillaEdit::SetAnchorForWordAlpha(int alpha) {
	if (anchor_word_indicator >= 0) {
		SendEditor(SCI_INDICSETALPHA, anchor_word_indicator, alpha);
		SendEditor(SCI_INDICSETOUTLINEALPHA, anchor_word_indicator, alpha);
	}
}
void ScintillaEdit::SetAnchorForWordVisible(bool enable) {
	enable_anchor_word_highlight= enable;
	if (!enable) {
		HideAnchorForWordIndic();
	}
}
void ScintillaEdit::ShowAnchorForWordIndic() {
	if (anchor_word_indicator == -1 || !enable_anchor_word_highlight) {
		return;
	}
	if (GetSelectionStart() < GetSelectionEnd()) {
		HideAnchorForWordIndic();
		return;
	}
	std::string word= GetAnchorPosWord();
	if (strcmp(word.data(), anchor_word.data()) == 0) {
		return;
	}
	HideAnchorForWordIndic();
	int flag = SCFIND_WHOLEWORD;
	anchor_word = word;
	if (anchor_word.length() == 0 || anchor_word_indicator==-1) {
		return;
	}
	SetSearchStringRanges(anchor_word.c_str(),0,-1, flag, anchor_word_indicator,ANCHOR_INDIC_VALUE);
}
BOOL ScintillaEdit::IsWantAnchorForWordIndic() {
	if (anchor_word_indicator == -1 || !enable_anchor_word_highlight) {
		return FALSE;
	}
	if (GetSelectionStart() < GetSelectionEnd()) {
		HideAnchorForWordIndic();
		return FALSE;
	}
	std::string word = GetAnchorPosWord();
	if (strcmp(word.data(), anchor_word.data()) == 0) {
		return FALSE;
	}
	HideAnchorForWordIndic();
	return TRUE;
}
void ScintillaEdit::HideAnchorForWordIndic() {
	if (anchor_word.length()) {
		SetsSearchStringsClear();
		anchor_word = "";
	}
}

void ScintillaEdit::EnsureCaretVisible() {
	Scintilla::EnsureCaretVisible(_module);
	//SendEditor(SCI_SCROLLCARET);
}

void ScintillaEdit::EnsureStyledAll() {
	Scintilla::EnsureStyledAll(_module);
	//SetFoldAll(SC_FOLDACTION_EXPAND);
}
int ScintillaEdit::GetFoldLevel(int line) {
	return Scintilla::GetFoldLevel(_module,line);
	//return SendEditor(SCI_GETFOLDLEVEL, line);
}
int ScintillaEdit::GetFoldParent(int line) {
	return Scintilla::GetFoldParent(_module, line);
	//return SendEditor(SCI_GETFOLDPARENT, line);
}
BOOL ScintillaEdit::GetFoldExpanded(int line) {
	return Scintilla::GetFoldExpanded(_module, line);
	//return SendEditor(SCI_GETFOLDEXPANDED, line);
}
//不能及时更新折叠。
//BOOL ScintillaEdit::SetFoldExpanded(int line,BOOL enable) {
//	return SendEditor(SCI_SETFOLDEXPANDED, line, enable);
//}
void ScintillaEdit::SetFoldLine(int line, int action) {
	Scintilla::SetFoldLine(_module, line, action);
	//SendEditor(SCI_FOLDLINE, line, action);
}
void ScintillaEdit::SetFoldLineNotUpdate(int line, int action) {
	Scintilla::SetFoldLineNotUpdate(_module, line, action);
}
void ScintillaEdit::SetFoldAll(int action) {
	Scintilla::SetFoldAll(_module, action);
	//SendEditor(SCI_FOLDALL, action);
}
BOOL ScintillaEdit::IsFoldStart(int line) {
	return GetFoldLevel(line) & SC_FOLDLEVELHEADERFLAG;
}
BOOL ScintillaEdit::IsFoldEnd(int line) {
	return GetFoldLevel(line) & SC_FOLDLEVELWHITEFLAG;
}

//SC_FOLDACTION_CONTRACT	0	折叠。
//SC_FOLDACTION_EXPAND	1	扩张。
//SC_FOLDACTION_TOGGLE	2	在收缩和扩展之间切换。
bool ScintillaEdit::AutoFoldAnchorLines() {
	int anchor = GetAnchor();
	int line = GetLineFromPos(anchor);
	int maxline = GetLineCount() - 1;
	if (line < 0 || line >= maxline) {
		line = 0;
	}
	if (maxline > 0) {
		int foldlevel = 0;
		BOOL isexpand = FALSE;
		while (!IsFoldStart(line) && line < maxline) {
			int ParentLine = GetFoldParent(line);
			if (IsFoldStart(ParentLine)) {
				line = ParentLine;
				break;
			}
			line++;
		}
		if (!IsFoldStart(line)) {
			return false;
		}
		foldlevel = GetFoldLevel(line);
		isexpand = !GetFoldExpanded(line);
		line = 0;
		int next_foldlevel = 0;
		for (; line < maxline; ++line) {
			next_foldlevel = GetFoldLevel(line);
			if ((next_foldlevel == foldlevel) || (isexpand && next_foldlevel < foldlevel)) {
				//printf("折叠行:%d,等级:%d\n", line, next_foldlevel);
				SetFoldLineNotUpdate(line, isexpand);
			}
		}
		SetRedraw();
	}
	return false;
}

void ScintillaEdit::SetFoldAppointLevel(int level, BOOL enable) {
	if (level < 0) {
		return;
	}
	int line = 0;
	int maxline = GetLineCount() - 1;
	if (maxline > 0) {
		int foldlevel = 0;
		int parentlevel = 0;
		BOOL isexpand = (BOOL)(enable == 0);
		for (int curlevel = 0; line <= maxline; ++line) {
			foldlevel = GetFoldLevel(line);
			if ((foldlevel & SC_FOLDLEVELHIGHHEADERFLAG)) {
				if (GetFoldLevel(GetFoldParent(line)) & SC_FOLDLEVELHIGHHEADERFLAG) {
					curlevel = 1;
				}
				else {
					curlevel = 0;
				}
			}
			else if ((foldlevel & SC_FOLDLEVELHEADERFLAG)) {
				parentlevel = GetFoldLevel(GetFoldParent(line));
				if ((parentlevel & SC_FOLDLEVELHEADERFLAG) && !(parentlevel & SC_FOLDLEVELHIGHHEADERFLAG)) {
					curlevel = 2;
				}
				else {
					curlevel = 1;
				}
			}
			else {
				curlevel = 3;
			}
			if (curlevel == level) {
				//printf("折叠行:%d,等级:%d\n",line,level);
				SetFoldLineNotUpdate(line, isexpand);
			}
		}
		SetRedraw();
	}
}
void ScintillaEdit::SetFoldFirstLevel(BOOL enable) {
	SetFoldAppointLevel(1, enable);
}

void ScintillaEdit::SetLineFoldExpandVisible(int line) {
	do {
		SetFoldLineNotUpdate(line, SC_FOLDACTION_EXPAND);
	} while ((line = GetFoldParent(line)) >= 0);
	SetRedraw();
}

int ScintillaEdit::GetAnchor() {
	return Scintilla::GetAnchor(_module);
	//return SendEditor(SCI_GETANCHOR);
}
void ScintillaEdit::SetCurrentPos(int pos) {
	SendEditor(SCI_SETCURRENTPOS, pos);
}
int ScintillaEdit::GetCurrentPos() {
	return Scintilla::GetCurrentPos(_module);
	//return SendEditor(SCI_GETCURRENTPOS);
}
int ScintillaEdit::GetSelectionStart() {
	return Scintilla::GetSelectionStart(_module);
}
int ScintillaEdit::GetSelectionEnd() {
	return Scintilla::GetSelectionEnd(_module);
}
void ScintillaEdit::SetSelPos(int anchor, int caret) {
	SendEditor(SCI_SETSEL, anchor, caret);
}
void ScintillaEdit::SetSelection(int pos, int end) {
	SendEditor(SCI_SETSELECTION, pos, end);
}

int ScintillaEdit::GetWordStartPos(int pos) {
	return Scintilla::GetWordStartPos(_module,pos);
	//return SendEditor(SCI_WORDSTARTPOSITION, pos);
}

int ScintillaEdit::GetWordEndPos(int pos) {
	return Scintilla::GetWordEndPos(_module, pos);
	//return SendEditor(SCI_WORDENDPOSITION, pos);
}

sptr_t ScintillaEdit::SetText(const char* text) {
	return Scintilla::SetText(_module, text);
	//return SendEditor(SCI_SETTEXT, (uptr_t)0, (uptr_t)text);
}
int ScintillaEdit::GetText(sptr_t length, char* buffer) {
	return Scintilla::GetText(_module, length, buffer);
	//return SendEditor(SCI_GETTEXT, (uptr_t)length, (uptr_t)buffer);
}
int ScintillaEdit::GetTextRange(Sci_TextRange* tr) {
	return Scintilla::GetTextRange(_module, (sptr_t)tr);
	//return SendEditor(SCI_GETTEXTRANGE, 0, sptr_t(tr));
}
void ScintillaEdit::GetTextRange(char* buffer, sptr_t start, sptr_t end) {
	Sci_TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = buffer;
	GetTextRange(&tr);
	//SendEditor(SCI_GETTEXTRANGE, 0, sptr_t(&tr));
}
std::string ScintillaEdit::GetTextRange(sptr_t start, sptr_t end) {
	if (start == end) {
		end++;
	}
	if (start < 0 || start > end) {
		return "";
	}
	if (end > GetTextLength()) {
		return "";
	}
	int len = end - start;
	Sci_TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = (char*)malloc(len + 1);
	ZeroMemory(tr.lpstrText, len + 1);
	GetTextRange(&tr);
	//SendEditor(SCI_GETTEXTRANGE, 0, sptr_t(&tr));
	std::string str(tr.lpstrText);
	free(tr.lpstrText);
	return str;
}
std::string ScintillaEdit::GetSelText(int maxlen) {
	int start = SendEditor(SCI_GETSELECTIONSTART);
	int end = SendEditor(SCI_GETSELECTIONEND);
	if (end - start < 1 || maxlen == 0) {
		return std::string("");
	}
	end = (end - start > maxlen && maxlen>0) ? start + maxlen : end;
	return GetTextRange(start,end);
}
std::string ScintillaEdit::GetAnchorPosWord() {
	int pos = GetAnchor();
	if (pos<0) {
		return std::string("");
	}
	char cur_ch = GetCharAt(pos);
	char ch = cur_ch;
	if (symbols.is_symbol(ch) ||symbols.is_space(ch)) {
		if (pos == 0) {
			return std::string("");
		}
		ch = GetCharAt(--pos);
		if (symbols.is_symbol(ch) || symbols.is_space(ch)) {
			return std::string("");
		}
	}
	int startpos = pos;
	int endpos = pos;
	int style = GetStyleAt(pos);
	if (style == SCE_JASS_NUMBER || style == SCE_LUA_NUMBER) {
		while (style == GetStyleAt(startpos - 1)) { --startpos; }
		while (style == GetStyleAt(endpos)) { ++endpos; }
	}
	else {
		ch = GetCharAt(pos - 1);
		if (!symbols.is_symbol(ch) && !symbols.is_space(ch)) {
			startpos = GetWordStartPos(pos);
		}
		ch = GetCharAt(startpos + 1);
		if (!symbols.is_symbol(ch) && !symbols.is_space(ch)) {
			endpos = GetWordEndPos(startpos);
			if (style != GetStyleAt(endpos - 1)) {
				endpos = pos;
			}
		}
	}
	int len = endpos - startpos+1;
	if (len < 0) {
		return std::string("");
	}
	if (len == 1) {
		return std::string({ GetCharAt(startpos) });
	}
	return GetTextRange(startpos, endpos);
}
int ScintillaEdit::GetTextLength() {
	return Scintilla::GetTextLength(_module);
	//return SendEditor(SCI_GETTEXTLENGTH);
}
int ScintillaEdit::GetLineCount() {
	return  Scintilla::GetLineCount(_module);
	//return SendEditor(SCI_GETLINECOUNT);
}
int ScintillaEdit::GetLineFromPos(int pos) {
	return Scintilla::GetLineFromPos(_module, pos);
	//return SendEditor(SCI_LINEFROMPOSITION, pos);
}
int ScintillaEdit::GetLineStartPos(int line) {
	return Scintilla::GetLineStartPos(_module, line);
	//return SendEditor(SCI_POSITIONFROMLINE, line);
}
int ScintillaEdit::GetLineEndPos(int line) {
	return Scintilla::GetLineEndPos( _module, line);
	//return SendEditor(SCI_GETLINEENDPOSITION, line);
}
int ScintillaEdit::GetLineLength(int line) {
	return Scintilla::GetLineLength(_module, line);
	//return SendEditor(SCI_LINELENGTH, line);
}
char ScintillaEdit::GetCharAt(int pos) {
	return Scintilla::GetCharAt(_module, pos);
	//return SendEditor(SCI_GETCHARAT, pos);
}

size_t ScintillaEdit::SetSearchStringRanges(const char*	str,
	int start, int end, int SearchFlags, int indic, int indicvalue) {
	return Scintilla::SetSearchStringRanges(_module, str, start, end, SearchFlags, indic, indicvalue);
}
int ScintillaEdit::GetSearchStringPosStart(size_t i) {
	return Scintilla::GetSearchStringPosStart(_module, i);
}
int ScintillaEdit::SelectSearchString(size_t i) {
	return Scintilla::SelectSearchString(_module,i);
}
int ScintillaEdit::ReplaceSearchString(size_t i, const char* newstr, bool select) {
	return Scintilla::ReplaceSearchString(_module,i, newstr, select);
}
size_t ScintillaEdit::ReplaceAllSearchString(const char* newstr, bool select) {
	return Scintilla::ReplaceAllSearchString(_module, newstr, select);
}
size_t ScintillaEdit::GetSearchStringsCount() {
	return Scintilla::GetSearchStringsCount(_module);
}
void ScintillaEdit::SetsSearchStringsClear() {
	Scintilla::SetsSearchStringsClear(_module);
}

void ScintillaEdit::AutoFillSpace() {
	int i = SendEditor(SCI_GETLINEINDENTATION, GetLineFromPos(GetCurrentPos()) - 1);
	if (i > 0) {
		i = i / 4;
		if (i > 0) {
			char* str = (char*)malloc(i + 1);
			memset(str, '	', i);
			str[i] = 0;
			SendEditor(SCI_ADDTEXT, i, (int)str);
			free(str);
		}
	}
}

void ScintillaEdit::EmptyUndoBuffer() {
	SendEditor(SCI_EMPTYUNDOBUFFER);
}

void ScintillaEdit::GoToPos(int pos) {
	SendEditor(SCI_GOTOPOS, pos);
}
void ScintillaEdit::GoToLine(int line) {
	SendEditor(SCI_GOTOLINE, line);
}
void ScintillaEdit::SetFirstVisibleLine(int line) {
	SendEditor(SCI_SETFIRSTVISIBLELINE, line);
}
int ScintillaEdit::GetFirstVisibleLine() {
	return SendEditor(SCI_GETFIRSTVISIBLELINE);
}
void ScintillaEdit::SetXoffset(int column) {
	SendEditor(SCI_SETXOFFSET, column);
}
int ScintillaEdit::GetXoffset() {
	return SendEditor(SCI_GETXOFFSET);
}
void ScintillaEdit::SetCallTipStyle(sptr_t fcolor, sptr_t bcolor, sptr_t highcolor) {
	SendEditor(SCI_CALLTIPSETFORE, fcolor);
	SendEditor(SCI_CALLTIPSETBACK, bcolor);
	SendEditor(SCI_CALLTIPSETFOREHLT, highcolor);
}
void ScintillaEdit::SetCallTipShow(int pos, const char* definition) {
	while (!calltip_start_list.empty()) {
		//==pos 相当于重新添加
		if (calltip_start_list.top() < pos) {
			break;
		}
		calltip_start_list.pop();
	}
	calltip_start_list.push(pos);
	calltip_start_pos = pos;
	SendEditor(SCI_CALLTIPSHOW, pos, (sptr_t)definition);
}
void ScintillaEdit::SetCallTipUpdate() {
	if (!calltip_start_list.empty()) {
		return Scintilla::CallTipUpdate(_module);
	}
}
void ScintillaEdit::SetCallTipCancel() {
	while (!calltip_start_list.empty()) {
		calltip_start_list.pop();
	}
	if (calltip_start_pos!=-1) {
		calltip_start_pos = -1;
		SendEditor(SCI_CALLTIPCANCEL);
	}
}

void ScintillaEdit::SetCallTipHighlight(int highlightStart, int highlightEnd) {
	SendEditor(SCI_CALLTIPSETHLT, highlightStart, highlightEnd);
}
BOOL ScintillaEdit::IsCallTipActive() {
	return SendEditor(SCI_CALLTIPACTIVE);
}




int ScintillaEdit::find_text(SearchFlags _flag, TextToFind*ft) {
	return SendEditor(SCI_FINDTEXT, (sptr_t)_flag, (sptr_t)ft);
}
void ScintillaEdit::set_search_flag(SearchFlags _flag) {
	SendEditor(SCI_SETSEARCHFLAGS, (sptr_t)_flag);
}
void ScintillaEdit::set_search_anchor(int anchor) {
	SendEditor(SCI_SEARCHANCHOR, anchor, 0);
	// anchor:
	// -1 text_start/range_start
	// -2 text_end/range_end
	// -3 select_start
	// -4 select_end
	// >=0
}
void ScintillaEdit::set_search_start(int i) {
	SendEditor(SCI_SETTARGETSTART, i);
}
void ScintillaEdit::set_search_end(int i) {
	SendEditor(SCI_SETTARGETEND, i);
}
void ScintillaEdit::set_search_range(int i1, int i2) {
	SendEditor(SCI_SETTARGETRANGE, i1, i2);
}


void ScintillaEdit::set_search_selection() {
	SetIndicClearRange(search_range_indicator, _search_range.x, SEARCH_INDIC_VALUE);
	_is_search_range = true;
	_search_id = 0;
	_search_pos = -1;
	_search_range.x = GetSelectionStart();
	_search_range.y = GetSelectionEnd();
	SetCurrentIndic(search_range_indicator);
	SetIndicRange(search_range_indicator, _search_range.x, _search_range.y - _search_range.x, SEARCH_INDIC_VALUE);
}
void ScintillaEdit::set_search_full() {
	SetIndicClearRange(1, _search_range.x, SEARCH_INDIC_VALUE);
	_is_search_range = false;
	_search_id = 0;
	_search_pos = -1;
	_search_range.x = 0;
	_search_range.y = GetTextLength();
}
void ScintillaEdit::init_search_range() {
	anchor_word = "";
	_search_want_update = false;
	if (!_is_search_range) {
		set_search_full();
	}
	else {
		set_search_selection();
	}
	SetSearchStringRanges(_search_text.data(), _search_range.x, _search_range.y,
		_search_flag, anchor_word_indicator, ANCHOR_INDIC_VALUE);

}

void ScintillaEdit::search_flag_update() {
	int pos = _search_pos;
	init_search_range();
	if (pos!=-1) {
		size_t count = GetSearchStringsCount();
		for (size_t i = 0; i < count;i++) {
			if (GetSearchStringPosStart(i)>=pos) {
				break;
			}
			_search_id = i + 1;
		}
	}
	//-1返回原位置。
	if (_search_id > 0) {
		_search_id--;
	}
	search_next();
}
void ScintillaEdit::search_pos_update(bool prev) {
	int pos = GetAnchor();
	if (pos != _search_pos) {
		_search_pos = pos;
		_search_id = 0;
		size_t count = GetSearchStringsCount();
		for (size_t i = 0; i < count; i++) {
			if (GetSearchStringPosStart(i) >= pos) {
				break;
			}
			_search_id = i + 1;
		}

		if (prev) {
			//如大于最后一个Id,下一次跳转返回最后一个位置
			_search_id++;
		}
	}
}
void ScintillaEdit::set_search_text(std::string str) {
	_search_text.assign(str);
	init_search_range();
	search_next();
}
std::string& ScintillaEdit::get_search_text() {
	return _search_text;
}
void ScintillaEdit::set_replace_text(std::string str) {
	if (_search_want_update || anchor_word.length() || GetSearchStringsCount() == 0) {
		init_search_range();
	}
	_replace_text.assign(str);
}
std::string& ScintillaEdit::get_replace_text() {
	return _replace_text;
}

int ScintillaEdit::search_next() {
	if (_search_want_update || anchor_word.length() || GetSearchStringsCount() == 0) {
		init_search_range();
	}
	search_pos_update(false);
	_search_id++;
	if (_search_id > GetSearchStringsCount()) {
		_search_id = 1;
	}
	_search_pos= SelectSearchString(_search_id-1);
	return _search_pos;
}
int ScintillaEdit::search_prev() {
	if (_search_want_update || anchor_word.length() || GetSearchStringsCount()==0) {
		init_search_range();
	}
	search_pos_update(true);
	if (_search_id > 0) {
		_search_id--;
	}
	if (_search_id < 1) {
		_search_id = GetSearchStringsCount();
	}
	_search_pos = SelectSearchString(_search_id-1);
	return _search_pos;
}
int ScintillaEdit::replace_text(bool move_scroll) {
	if (_search_want_update || anchor_word.length() || GetSearchStringsCount() == 0) {
		init_search_range();
	}
	search_pos_update(true);
	if (_search_id == 0 ||_search_id > GetSearchStringsCount()) {
		_search_id = 1;
	}
	_replaceing = true;
	_search_pos =ReplaceSearchString(_search_id-1, _replace_text.data(), move_scroll);
	_replaceing = false;
	return _search_pos;
}
size_t ScintillaEdit::replace_all(bool move_scroll) {
	if (_search_want_update || anchor_word.length() || GetSearchStringsCount() == 0) {
		init_search_range();
	}
	_search_id = 0;
	_search_pos = -1;
	_replaceing = true;
	size_t count= ReplaceAllSearchString(_replace_text.data(), move_scroll);
	_replaceing = false;
	_search_pos = GetAnchor();
	return count;
}

void JassEdit::SetAllFontSize(sptr_t _size) {
	SetFontSize(STYLE_DEFAULT, _size);
	for (int i = 0; i <= SCE_JASS_NMAX; i++) {
		SetFontSize(i, _size);
	}
}
void JassEdit::SetFuncKeywords(const char* _cj_keywords, const char* _bj_keywords, const char* _cst_keywords) {
	SetKeywords(5, _cst_keywords);
	SetKeywords(6, _cj_keywords);
	SetKeywords(7, _bj_keywords);
}
void JassEdit::SetConstantKeywords(const char* _keywords) {
	SetKeywords(5, _keywords);
}
void JassEdit::SetCjKeywords(const char* _keywords) {
	SetKeywords(6, _keywords);
}
void JassEdit::SetBjKeywords(const char* _keywords) {
	SetKeywords(7, _keywords);
}

void JassEdit::EnableHotspot(BOOL enable) {
	SetStyleHotspot(SCE_JASS_WORD5, enable);
	SetStyleHotspot(SCE_JASS_WORD6, enable);
}

bool JassEdit::SetEditMainFontName(const char* name) {
	SetFontName( STYLE_DEFAULT, name);
	SetFontName( STYLE_BRACELIGHT, name);
	SetFontName( STYLE_BRACEBAD, name);
	SetFontName( STYLE_CALLTIP, name);

	SetFontName( SCE_JASS_DEFAULT, name);
	SetFontName( SCE_LUA_DEFAULT, name);
	SetFontName( SCE_JASS_IDENTIFIER, name);
	SetFontName( SCE_LUA_IDENTIFIER, name);

	for (int i = SCE_JASS_DEFAULT; i <= SCE_JASS_NMAX; i++) {
		SetFontName( i, name);
	}
	return true;
}
bool JassEdit::SetEditMainFontSize(int size) {
	if (size < 8 || size>72) {
		return false;
	}

	if (size <= 12){
		SetFoldWidth(14);
	}
	else if (size <= 24){
		SetFoldWidth(15);
	}
	else {
		SetFoldWidth(16);
	}
	SetFontSize(STYLE_DEFAULT, size);
	SetFontSize(STYLE_BRACELIGHT, size);
	SetFontSize(STYLE_BRACEBAD, size);
	SetFontSize(STYLE_CALLTIP, size);

	SetFontSize(SCE_JASS_DEFAULT, size);
	SetFontSize(SCE_LUA_DEFAULT, size);
	SetFontSize(SCE_JASS_IDENTIFIER, size);
	SetFontSize(SCE_LUA_IDENTIFIER, size);

	for (int i = SCE_JASS_DEFAULT; i <= SCE_JASS_NMAX; i++) {
		SetFontSize(i, size);
	}
	return true;
}
bool JassEdit::SetEditMainFont(const char* name, int size) {
	if (!name) {
		return false;
	}
	SetEditMainFontName(name);
	SetEditMainFontSize(size);
	return true;
}
void JassEdit::SetEditMainForeColor(int color) {
	SetForeColor(STYLE_DEFAULT, color);
	SetForeColor(SCE_JASS_DEFAULT, color);
	SetForeColor(SCE_LUA_DEFAULT, color);
	SetForeColor(SCE_JASS_IDENTIFIER, color);
	SetForeColor(SCE_LUA_IDENTIFIER, color);
}
void JassEdit::SetEditMainBackColor(int color) {
	SetBackColor(STYLE_DEFAULT, color);
	SetBackColor(SCE_JASS_DEFAULT, color);
	SetBackColor(SCE_LUA_DEFAULT, color);
	SetBackColor(SCE_JASS_IDENTIFIER, color);
	SetBackColor(SCE_LUA_IDENTIFIER, color);

}