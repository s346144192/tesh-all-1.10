#pragma once
#include <map>
#include <string>
#include <dll_info.h>
#include <CtrlFunc.h>
#include <text/ini.h>

extern std::map<int,std::string> style_option_keywords;


#define IDR_OPTIONS_FONT_BUTTON 0x10
#define IDR_OPTIONS_LINENUMBER_FONT_BUTTON 0x11
#define IDR_OPTIONS_STYLE_BUTTON 0x12
#define IDR_OPTIONS_SAVE_BUTTON 0x13
#define IDR_OPTIONS_LINENUMBER_CHECKBOX 0x14
#define IDR_OPTIONS_CODEFOLDING_CHECKBOX 0x15
#define IDR_OPTIONS_AUTOCOMPLETE_CHECKBOX 0x16
#define IDR_OPTIONS_CALLTIP_CHECKBOX 0x17
#define IDR_OPTIONS_ZOOMLEVEL_CHECKBOX 0x18
#define IDR_OPTIONS_HIDE_CONVERTWARNING_CHECKBOX 0x19
#define IDR_OPTIONS_SKIPIME_CHECKBOX 0x1A
#define IDR_OPTIONS_LISTONTOP 0x1B

#define IDR_STYLEOPTIONS_LIST 0x20
#define IDR_STYLEOPTIONS_BOLD 0x21
#define IDR_STYLEOPTIONS_ITALIC 0x22
#define IDR_STYLEOPTIONS_UNDERLINE 0x23
#define IDR_STYLEOPTIONS_FOREGROUND_COMBOX 0x24
#define IDR_STYLEOPTIONS_BACKGROUND_COMBOX 0x25
#define IDR_STYLEOPTIONS_ALPHA_PUT 0x26
#define IDR_STYLEOPTIONS_ALPHA_UPDOWN 0x27
#define IDR_STYLEOPTIONS_VISIBLE 0x28


class StyleOptions {
public:
	HWND _parent;
	HWND _hwnd;
	HWND _List;
	HWND _Bold;
	HWND _Italic;
	HWND _Underline;
	HWND _ForegroundName;HWND _ForegroundComBox;
	HWND _BackgroundName;HWND _BackgroundComBox;
	HWND _AlphaName;
	HWND _AlphaPut;HWND _AlphaUpDown;
	HWND _FontPreview;
	HWND _Visible;

	HFONT preview;

	std::string cur_sel_string;
	int default_fore;
	int default_back;
	int custom_fore;
	int custom_back;

	uintptr_t CodeBox;
	ini::config_t* styles;

	void SetPreviewFont(LOGFONTA* lf, bool scale_size = false);
	void LoadCurPreviewFont();

	bool GetEditFont(int style, LOGFONTA* lf);
	bool SetEditFont(int style, LOGFONTA* lf, bool scale_size);
	void SetEditFore(int style, int color);
	void SetEditBack(int style, int color);
	int GetEditFore(int style);
	int GetEditBack(int style);

	void SetMatchStyleState(int style_types);

	void SetStyleAlpha();
	void SetStyleVisible();

	void SetFontBold(BOOL enable);
	void SetFontItalic(BOOL enable);
	void SetFontUnderline(BOOL enable);
	void SetForeColor(int color);
	void SetBackColor(int color);

	std::string GetListString(int i);
	std::string GetListCurSelString();
	int FindListString(const char* name);
	void SetListCurSel(int i);
	void SetListFindCurSel(const char* name);
	void LoadListCurSel(bool or_selcet_other);

	int GetForeSelIndex();
	int GetBackSelIndex();
	void SetForeSelIndex(int idx);
	void SetBackSelIndex(int idx);

	void SetForeSelColor(int color);
	int GetForeSelColor();
	void SetBackSelColor(int color);
	int GetBackSelColor();

	void ShowItems(const char* name);

	void drawitem(LPDRAWITEMSTRUCT item);

	bool IsShow();
	void Show(BOOL enable);

	void InitAppStyles();
	void SaveStyles();

	void LoadList();

	void Init(HWND parent, uintptr_t _CodeBox,ini::config_t* _styles);
	static int color_list[25];
	static BindList<HWND,StyleOptions*> classlist;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class Options {
public:
	HWND _parent;
	HWND _main;
	HWND _FontTip; HWND _FontBtn; HWND _FontNameText;
	HWND _LineNumFontTip; HWND _LineNumFontBtn;
	HWND _StyleOptionsBtn;
	HWND _Line_Numbers;HWND _CodeFolding;HWND _AutoComplete;HWND _CallTip;; HWND _ZoomLevel;
	HWND _SkipIncompatibleIme;
	HWND _HideConvertWarning;
	HWND _GroupBox; HWND _ListOnTop;

	HFONT preview;

	uintptr_t CodeBox;
	uintptr_t FuncList;

	StyleOptions _StyleOptions;

	ini::config_t* config; BOOL ShowWarning; BOOL SkipIncompatibleIme;

	void SetPreviewFont(LOGFONTA* lf, bool scale_size=false);

	bool SetEditMainFont(LOGFONTA* lf, bool scale_size);
	bool SetLineNumFont(LOGFONTA* lf, bool scale_size);
	bool GetEditFont(int style, LOGFONTA* lf);
	bool SetEditFont(int style, LOGFONTA* lf,bool scale_size);

	void SetEditFore(int style, int color);
	void SetEditBack(int style, int color);
	int GetEditFore(int style);
	int GetEditBack(int style);

	void SetCheckItem(const char* name, BOOL enable);
	void SetFuncListTop(bool enable);

	bool IsShow();
	void Show(BOOL enable);

	void InitAppConfig(bool appstyles);
	void SaveConfig(bool save_styles);

	void LoadConfig();

	void Init(HWND parent, uintptr_t _CodeBox, uintptr_t _FuncList, ini::config_t* _config, ini::config_t* _styles);
	static CHOOSEFONTA* cur_fontinfo;
	static BindList<HWND, Options*> classlist;
	static UINT_PTR CALLBACK AppFont(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
