// Scintilla source code edit control
/** @file ScintillaWin.h
 ** Define functions from ScintillaWin.cxx that can be called from ScintillaDLL.cxx.
 **/
// Copyright 1998-2018 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLAWIN_H
#define SCINTILLAWIN_H

#include <Scintilla.h>

#include <Position.h>

#include <windows.h>

class SCINTILLA_API ScintillaWin;

namespace Scintilla {

	// SCINTILLA_API int Scintilla_RegisterClasses(void *hInstance);
	// This function is externally visible so it can be called from container when building statically.
	// SCINTILLA_API int Scintilla_ReleaseResources();

	SCINTILLA_API int ResourcesRelease(bool fromDllMain) noexcept;
	SCINTILLA_API sptr_t DirectFunction(ScintillaWin *sci, UINT iMessage, uptr_t wParam, sptr_t lParam);

	//新增
	SCINTILLA_API void SetCallbackMessageCommand(ScintillaWin* sci, MessageCommandCallback* callback);
	SCINTILLA_API void SetCallbackMessageNotify(ScintillaWin* sci, MessageNotifyCallback* callback);

	SCINTILLA_API void _cdecl SetBraceLight(ScintillaWin* sci, int stpos, int endpos);

	SCINTILLA_API int _cdecl SetBraceMatch(ScintillaWin* sci, int pos, int length);

	SCINTILLA_API void _cdecl StyleSetMessage(ScintillaWin* sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	SCINTILLA_API sptr_t _cdecl StyleGetMessage(ScintillaWin* sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

	SCINTILLA_API int _cdecl GetStyleAt(ScintillaWin* sci, int i);


	SCINTILLA_API sptr_t _cdecl GetIndicStart(ScintillaWin* sci, sptr_t indicator, sptr_t pos);
	SCINTILLA_API sptr_t _cdecl GetIndicEnd(ScintillaWin* sci, sptr_t indicator, sptr_t pos);
	SCINTILLA_API sptr_t _cdecl GetIndicAtValue(ScintillaWin* sci, sptr_t indicator, sptr_t pos);
	SCINTILLA_API sptr_t _cdecl GetCurrentIndicValue(ScintillaWin* sci);
	SCINTILLA_API void _cdecl SetCurrentIndic(ScintillaWin* sci, sptr_t indicator);
	SCINTILLA_API void _cdecl SetCurrentIndicValue(ScintillaWin* sci, sptr_t value);
	SCINTILLA_API void _cdecl SetIndicRange(ScintillaWin* sci, sptr_t start, sptr_t length);
	SCINTILLA_API void _cdecl SetIndicRangeExt(ScintillaWin* sci, sptr_t indicator, sptr_t start, sptr_t length);
	SCINTILLA_API void _cdecl SetIndicClearRange(ScintillaWin* sci, sptr_t start, sptr_t lengthClear);

	SCINTILLA_API int _cdecl GetWordStartPos(ScintillaWin* sci, int pos);
	SCINTILLA_API int _cdecl GetWordEndPos(ScintillaWin* sci, int pos);

	SCINTILLA_API int _cdecl GetTextRange(ScintillaWin* sci, sptr_t lParam);

	SCINTILLA_API sptr_t _cdecl SetText(ScintillaWin* sci, const char* text);
	SCINTILLA_API int _cdecl GetText(ScintillaWin* sci, sptr_t length, char* buffer);

	SCINTILLA_API int _cdecl GetTextLength(ScintillaWin* sci);
	SCINTILLA_API int _cdecl GetLineCount(ScintillaWin* sci);
	SCINTILLA_API int _cdecl GetLineFromPos(ScintillaWin* sci, int i);
	SCINTILLA_API int _cdecl GetLineStartPos(ScintillaWin* sci, int i);
	SCINTILLA_API int _cdecl GetLineEndPos(ScintillaWin* sci, int i);
	SCINTILLA_API int _cdecl GetLineLength(ScintillaWin* sci, int i);

	SCINTILLA_API char _cdecl GetCharAt(ScintillaWin* sci, int i);
	SCINTILLA_API int _cdecl GetCurrentPos(ScintillaWin* sci);

	SCINTILLA_API void _cdecl EnsureStyledAll(ScintillaWin* sci);
	SCINTILLA_API void _cdecl EnsureCaretVisible(ScintillaWin* sci);
	SCINTILLA_API int _cdecl GetFoldLevel(ScintillaWin* sci, int line);
	SCINTILLA_API int _cdecl GetFoldParent(ScintillaWin* sci, int line);
	SCINTILLA_API BOOL _cdecl GetFoldExpanded(ScintillaWin* sci, int line);
	SCINTILLA_API void _cdecl SetFoldLine(ScintillaWin* sci, int line, int action);
	SCINTILLA_API void _cdecl SetFoldLineNotUpdate(ScintillaWin* sci, int line, int action);
	SCINTILLA_API void _cdecl SetFoldAll(ScintillaWin* sci, int action);
	SCINTILLA_API void _cdecl SetRedraw(ScintillaWin* sci);
	SCINTILLA_API int _cdecl GetAnchor(ScintillaWin* sci);
	SCINTILLA_API int GetSelectionStart(ScintillaWin* sci);
	SCINTILLA_API int GetSelectionEnd(ScintillaWin* sci);
	SCINTILLA_API size_t SetSearchStringRanges(ScintillaWin* sci, const char* str, Sci::Position start
		, Sci::Position end, int SearchFlags, int indic, int indicvalue);
	SCINTILLA_API int GetSearchStringPosStart(ScintillaWin* sci, size_t i);
	SCINTILLA_API int SelectSearchString(ScintillaWin* sci, size_t i);
	//替换删除指定的搜索范围,返回替换字符串长度。
	SCINTILLA_API int ReplaceSearchString(ScintillaWin* sci, size_t i, const char* newstr, bool select);
	//替换自动清理当前搜索范围,返回替换数量。
	SCINTILLA_API size_t ReplaceAllSearchString(ScintillaWin* sci, const char* newstr, bool select);
	SCINTILLA_API size_t GetSearchStringsCount(ScintillaWin* sci);
	SCINTILLA_API void SetsSearchStringsClear(ScintillaWin* sci);

	SCINTILLA_API bool AutoCompleteActive(ScintillaWin* sci);

	SCINTILLA_API void AutoCompleteShow(ScintillaWin* sci, Sci::Position lenEntered, const char *list);

	SCINTILLA_API void AutoCompleteUpdate(ScintillaWin* sci);

	SCINTILLA_API void AutoCompleteCancel(ScintillaWin* sci);

	SCINTILLA_API void CallTipUpdate(ScintillaWin* sci);

}

#endif
