#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWin.h>
//#include <ScintillaWin.h>

//#pragma comment(lib,"SciLexer.lib")

//����Ŀ¼���ܰ�����Ŀ¼��

//����/���� 16����ɫ��(BGR):BGR:0xFF1020 -> RGB:0x2010FF 
//�ٴ�ת����ת���ء�
#define TO_RGB(c) ((c & 0xFFFFFF) >> 16)+( ((c & 0xFFFF) >> 8 )<< 8)+((c & 0xFF) << 16)
#define TO_BGR(c) TO_RGB(c)

//�����������ܸ�����������Ч����
//
//enum struct SearchFlags {
//	FIND_NONE = SCFIND_NONE,
//	FIND_WHOLEWORD = SCFIND_WHOLEWORD,
//	FIND_MATCHCASE = SCFIND_MATCHCASE,
//	FIND_WORDSTART = SCFIND_WORDSTART,
//	FIND_REGEXP = SCFIND_REGEXP,
//	FIND_POSIX = SCFIND_POSIX,
//	FIND_CXX11REGEX = SCFIND_CXX11REGEX,
//};
typedef int SearchFlags;
typedef  Sci_TextToFind TextToFind;

struct ScintillaStyle {
	char name[LF_FACESIZE];
	size_t size;
	BOOL bold;
	BOOL italic;
	BOOL underline;
	int fore;
	int back;
	int alpha;
	void tologfont(LOGFONTA* lf);
	void copylogfont(LOGFONTA* lf);
};

#define CALLTIP_BUFFER_SIZE 360
#define CALLTIP_MATCH_MAX_RANGE 4096

#define SEARCH_INDIC_VALUE 100
#define ANCHOR_INDIC_VALUE 110

class ScintillaEdit
{
private:
	static bool registered;
	static bool editor_count;
	std::string _search_text;
	std::string _replace_text;
public:
	static BOOL IsEnableUTF8;
	
	HMODULE pSciLexer;
	SciFnDirect fnDirect;
	sptr_t ptrDirect;
	HWND _hwnd;

	ScintillaWin* _module;

	int line_num_mk_idx;
	int fold_mk_idx;
	int fold_mk_width;

	int search_range_indicator;
	bool _searching;
	bool _replaceing;
	bool _is_search_range;
	bool _search_want_update;
	SearchFlags _search_flag;
	POINT _search_range;
	size_t _search_id;
	int _search_pos;

	int anchor_word_indicator;
	bool enable_anchor_word_highlight;
	std::string anchor_word;
	
	std::string AutoCompleteList;
	int AutoCompleteListCount;
	std::string calltip_text;
	int calltip_start_pos;
	std::stack<int> calltip_start_list;

	int last_bracelight_state;
	int last_bracelight_pos;

	ScintillaEdit();
	~ScintillaEdit();

	void* PointerFromWindow(HWND hwnd) noexcept;

	void ChangeBackground(int color);

	BOOL InitEditBox(DWORD exdwStyle, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);

	void ResetEditBox(bool reset_btn_state);

	sptr_t SendEditor(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0);

	void SetCallbackMessageCommand(MessageCommandCallback* proc);
	void SetCallbackMessageNotify(MessageNotifyCallback* proc);

	void SetRedraw();

	void SetTechNology(int _type);
	int GetTechNology();
	void SetBufferedDraw(BOOL enable);
	//��Ч��
	void SetFontLocale(const char* localeName);
	void EnableUTF8(BOOL enable);
	void EnableD2dDraw(BOOL enable);
	//SC_EFF_QUALITY_LCD_OPTIMIZED ...
	void SetFontQuality(int fontQuality);
	int GetFontQuality();

	void SetReadonly(BOOL enable);
	void SetLexer(sptr_t sclex_t);
	void SetCodePage(sptr_t codeid);
	//SC_CHARSET_ANSI ,SC_CHARSET_GB2312
	void SetCharSet(sptr_t _style, sptr_t charset);
	//��������

	//���Ÿ���
	void SetBraceStyle(const char* fontname, bool default_sytle, sptr_t color, sptr_t bad_color, BOOL bold);
	void SetBraceLight(int stpos, int endpos);
	void SetBraceBadLight(int stpos);
	void SetBraceFore(sptr_t fcolor);
	void SetBraceBack(sptr_t bcolor);
	void SetPosBraceBold(BOOL enable);
	void SetPosBraceItalic(BOOL enable);
	void SetPosBraceUnderline(BOOL enable);
	bool IsBrace(char ch);
	int SetBraceMatch(int pos, int length);
	void UpCurBraceFont();
	void UpCurBraceBackColor();
	void UpCurBrace();
	//�ο���
	void SetGuideStyle(sptr_t _type, sptr_t fcolor, sptr_t bcolor);
	//�����ͣ�»���,֧�ֵ��
	void SetStyleHotspot(sptr_t style, sptr_t enable);
	std::string GetHotspotWord(int pos);
	//��������
	void SetZoomLevelMinMax(int min, int max);
	//��ǰ�и���
	void SetCurrentLineStyle(BOOL enable, sptr_t bcolor, sptr_t balpha);
	//�Զ������ʾ
	void SetAutoComplete(sptr_t width, sptr_t maxcount);
	
	
	void AutoCompleteShow(int curlen,const char* list);
	//����λ��
	void AutoCompleteUpdate();
	void AutoCompleteCancel();
	bool AutoCompleteActive();
	void ResetAutoComplete();

	void SetKeywords(sptr_t index, const char* keywords);
	void SetTabWidth(sptr_t _width);
	//RGB(255, 255, 255)ǰ����͸���Ȳ�����, �Զ�����AdditionalSelect 
	void SetSelectStyle(BOOL enable, sptr_t fcolor, sptr_t bcolor, sptr_t alpha);
	void SetAdditionalSelectStyle(sptr_t fcolor, sptr_t bcolor, sptr_t alpha);
	void SetDefaultStyles(const char* fontname, sptr_t fontsize, sptr_t fcolor, sptr_t bcolor);
	void EnableLineNumber(sptr_t mkIndex, sptr_t width);
	void SetLineNumberWidth(sptr_t width);
	void EnableFold(sptr_t mkIndex, sptr_t mkwidth, bool show_underline, bool enable);
	void SetFoldWidth(sptr_t mkwidth);
	void ShowFold(bool enable);
	void SetMarkerFore(sptr_t color);
	void SetMarkerback1(sptr_t color);
	void SetMarkerback2(sptr_t color);

	sptr_t GetStyleState(sptr_t style, ScintillaStyle*  sstate);
	void SetStyleState(sptr_t style, ScintillaStyle*  sstate);
	//�������߶������Ŵ�С��
	sptr_t GetLogFontExt(sptr_t style, LOGFONTA* lf, bool use_utf8 = false);
	void SetLogFontExt(sptr_t style, LOGFONTA*  lf, bool scale_size);

	sptr_t GetForeColor(sptr_t _type);
	sptr_t GetBackColor(sptr_t _type);
	sptr_t GetFontSize(sptr_t _type);
	sptr_t GetFontBold(sptr_t _type);
	sptr_t GetFontItalic(sptr_t _type);
	sptr_t GetFontUnderline(sptr_t _type);
	void SetFont(sptr_t _type, const char* name, bool default_sytle);
	void SetFont(sptr_t _type, const char* name, sptr_t _size, sptr_t _bold, sptr_t _italic, sptr_t _underline, sptr_t color);
	bool SetFontName(sptr_t _type, const char* name);
	std::string GetFontName(sptr_t _type, bool use_utf8 = false);
	void SetFontStyles(sptr_t _type, sptr_t _size, sptr_t _bold, sptr_t _italic, sptr_t _underline, sptr_t color);
	void SetForeColor(sptr_t _type, sptr_t color);
	void SetBackColor(sptr_t _type, sptr_t color);
	void SetFontSize(sptr_t _type, sptr_t size);
	void SetFontBold(sptr_t _type, sptr_t enable);
	void SetFontItalic(sptr_t _type, sptr_t enable);
	void SetFontUnderline(sptr_t _type, sptr_t enable);

	int GetStyleAt(sptr_t pos);
	int GetPosStyleFontSize(sptr_t pos);
	//���ָ��λ��style,Ȼ���ж����Ӧ��style�Ƿ������
	BOOL GetPosStyleBold(sptr_t pos);

	//��ʾ���Ρ��»��ߵ�style��
	void SetIndicStyle(sptr_t indicator, sptr_t indicatorStyle);
	void SetIndicStyle(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha);
	//��������color
	void SetIndicFore(sptr_t indicator, sptr_t fcolor);
	void SetIndicAlpha(sptr_t indicator, sptr_t alpha);
	//INDIC_ROUNDBOX��INDIC_STRAIGHTBOX����
	void SetIndicOutLineAlpha(sptr_t indicator, sptr_t outline_alpha);
	//�������ͣ
	void SetIndicHoverStyle(sptr_t indicator, sptr_t indicatorStyle);
	void SetIndicHoverFore(sptr_t indicator, sptr_t fcolor);
	//Ԥ���ã�����/ɾ��ָ����Χʱ��Ҫʹ�õ�ǰIndic style��
	void SetCurrentIndic(sptr_t indicator);
	//ֵ������ͬ
	void SetCurrentIndicValue(sptr_t value);
	sptr_t GetCurrentIndicValue();
	sptr_t GetIndicAtValue(int indicator, int pos);
	sptr_t GetIndicStart(int indicator, int pos);
	sptr_t GetIndicEnd(int indicator, int pos);
	sptr_t GetIndicStart(int indicator, int pos, sptr_t value);
	sptr_t GetIndicEnd(int indicator, int pos, sptr_t value);
	//ָ����Χ(a->b), ɾ���ַ��ᱣ��style,ɾ�������ַ���ʱ���á�
	void SetIndicRange(sptr_t start, sptr_t length);
	void SetIndicRange(sptr_t indicator, sptr_t start, sptr_t length);
	void SetIndicRange(sptr_t indicator, sptr_t start, sptr_t length, sptr_t value);

	//ɾ��
	void SetIndicClearRange(sptr_t start, sptr_t lengthClear);
	void SetIndicClearRange(sptr_t indicator, sptr_t start, sptr_t value);

	//���´��ĵ�,�����á�
	void SetIndicClearAll();

	void SetSearchRangeIndic(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha);
	void SetSearchRangeBack(int color);
	void SetSearchRangeAlpha(int alpha);

	void SetAnchorForWordIndic(sptr_t indicator, sptr_t indicatorStyle, sptr_t fcolor, sptr_t alpha, sptr_t outline_alpha);
	void SetAnchorForWordBack(int color);
	void SetAnchorForWordAlpha(int alpha);
	void SetAnchorForWordVisible(bool enable);

	void ShowAnchorForWordIndic();
	BOOL IsWantAnchorForWordIndic();
	void HideAnchorForWordIndic();

	//��ת����ǰλ�á�(��ǰλ�á�ѡ��)
	void EnsureCaretVisible();

	void EnsureStyledAll();
	int GetFoldLevel(int line);
	int GetFoldParent(int line);
	BOOL GetFoldExpanded(int line);
	void SetFoldLine(int line, int action);
	void SetFoldLineNotUpdate(int line, int action);
	void SetFoldAll(int action);
	BOOL IsFoldStart(int line);
	BOOL IsFoldEnd(int line);
	bool AutoFoldAnchorLines();
	void SetFoldAppointLevel(int level, BOOL enable);
	void SetFoldFirstLevel(BOOL enable);
	void SetLineFoldExpandVisible(int line);

	int GetAnchor();
	void SetCurrentPos(int pos);
	int GetCurrentPos();
	int GetSelectionStart();
	int GetSelectionEnd();
	//caret<0 ������λ��,anchor<0��caret��ͬ
	void SetSelPos(int anchor, int caret);
	void SetSelection(int pos, int end);

	int GetWordStartPos(int pos);
	int GetWordEndPos(int pos);
	
	//�༭�ı���Ϣ
	sptr_t SetText( const char* text);
	int GetText(sptr_t length, char* buffer);
	int GetTextRange(Sci_TextRange* tr);
	void GetTextRange(char* buffer,sptr_t start, sptr_t end);
	std::string GetTextRange(sptr_t start, sptr_t end);
	std::string GetSelText(int maxlen);
	std::string GetAnchorPosWord();

	int GetTextLength();
	int GetLineCount();
	int GetLineFromPos(int pos);
	int GetLineStartPos(int line);
	int GetLineEndPos(int line);
	int GetLineLength(int line);
	char GetCharAt(int pos);

	//SetSearchStringRanges ����ƥ���ı�����(����������)
	size_t SetSearchStringRanges(const char* str, int start
		, int end, int SearchFlags, int indic, int indicvalue);
	int GetSearchStringPosStart(size_t i);
	//SelectSearchString ����,��0��ʼ,С��ƥ���ı��������Զ���ת��
	int SelectSearchString(size_t i);
	int ReplaceSearchString(size_t i, const char* newstr, bool select);
	size_t ReplaceAllSearchString(const char* newstr, bool select);
	size_t GetSearchStringsCount();
	void SetsSearchStringsClear();

	void AutoFillSpace();

	void EmptyUndoBuffer();

	//������
	void GoToPos(int pos);
	void GoToLine(int line);
	void SetFirstVisibleLine(int line);
	int GetFirstVisibleLine();
	void SetXoffset(int column);
	int GetXoffset();

	void SetCallTipStyle(sptr_t fcolor, sptr_t bcolor, sptr_t highcolor);
	void SetCallTipShow(int pos, const char* definition);
	void SetCallTipUpdate();
	void SetCallTipCancel();
	void SetCallTipHighlight(int highlightStart, int highlightEnd);
	BOOL IsCallTipActive();


	//����
	int find_text(SearchFlags _flag, TextToFind*ft);
	void set_search_flag(SearchFlags _flag);
	void set_search_anchor(int anchor);
	void set_search_start(int i);
	void set_search_end(int i);
	void set_search_range(int i1, int i2);
	
	void set_search_selection();
	void set_search_full();
	void init_search_range();

	//����flag����ʱˢ�¡�
	void search_flag_update();
	void search_pos_update(bool prev=false);
	void set_search_text(std::string str);
	std::string& get_search_text();
	void set_replace_text(std::string str);
	std::string& get_replace_text();

	int search_next();
	int search_prev();
	int replace_text(bool move_scroll);
	size_t replace_all(bool move_scroll);
};
class JassEdit :public ScintillaEdit {
public:
	void SetAllFontSize(sptr_t _size);
	void SetFuncKeywords(const char* _cj_keywords, const char* _bj_keywords, const char* _cst_keywords);
	void SetCjKeywords(const char* _keywords);
	void SetBjKeywords(const char* _keywords);
	void SetConstantKeywords(const char* _keywords);
	void EnableHotspot(BOOL enable);
	bool SetEditMainFontName(const char* name);
	bool SetEditMainFontSize(int size);
	bool SetEditMainFont(const char* name, int size);
	void SetEditMainForeColor(int color);
	void SetEditMainBackColor(int color);
};
