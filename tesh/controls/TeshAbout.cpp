#include <TeshAbout.h>

BaseMap<HWND, TeshAbout*> TeshAbout::classlist;

LRESULT CALLBACK TeshAbout::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TeshAbout* about = classlist.get(hwnd);
	if (!about) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	if (message == WM_CTLCOLORSTATIC) {
		if ((HWND)lParam == about->_email_addr) {
			SetTextColor((HDC)wParam,RGB(0,102,255)); 
			SetBkMode((HDC)wParam,TRANSPARENT);
			//SetBkColor((HDC)wParam, TO_RGB(style_options->GetBackSelColor()));
			return (INT_PTR)GetSysColorBrush(COLOR_MENU); //±≥æ∞ª≠À¢
		}
	}
	if (message==WM_CLOSE) {
		ShowWindow(hwnd, SW_HIDE);
		SetActiveWindow(GetParent(hwnd));
		return FALSE;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
void TeshAbout::Show(bool enable) {
	ShowTopHwnd(_hwnd,enable);
	//ShowHwnd(_hwnd, enable);
}

void TeshAbout::Init(HWND parent, const char* tip, const char* description,  const char* email_addr, const char* developer) {
	int scrWidth = GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = GetSystemMetrics(SM_CYSCREEN);
	_hwnd = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, "TeshAboutWin", "About TESH", WS_SYSMENU
		, max(0, (scrWidth / 2 - 180)), max(0, (scrHeight / 2 - 190)), 425, 190, parent, (HMENU)0, nullptr, nullptr);
	//int error = GetLastError();
	_tip = CreateWindowA("static", tip, WS_CHILD | SS_LEFT  | WS_VISIBLE, 20, 10, 400, 20, _hwnd, (HMENU)0, nullptr, nullptr);
	_description = CreateWindowA("static", description, WS_CHILD | SS_LEFT | WS_VISIBLE, 20, 35, 375, 55, _hwnd, (HMENU)0, nullptr, nullptr);
	_email_tip = CreateWindowA("static", "¡¥Ω”” œ‰:", WS_CHILD | SS_LEFT | WS_VISIBLE, 20, 85, 75, 20, _hwnd, (HMENU)0, nullptr, nullptr);
	_email_addr = CreateWindowA("static", email_addr, WS_CHILD | SS_LEFT | WS_VISIBLE, 85, 85, 200, 20, _hwnd, (HMENU)0, nullptr, nullptr);
	_developer = CreateWindowA("static", developer, WS_CHILD | SS_LEFT | WS_VISIBLE, 20, 110, 375, 50, _hwnd, (HMENU)0, nullptr, nullptr);

	SetCtrlFont(_tip, "Tahoma", 0, 16);
	SetCtrlFont(_description, "Tahoma", 0, 16);
	SetCtrlFont(_email_tip, "Tahoma", 0, 16);
	SetCtrlFont(_email_addr, "Tahoma", 0, 16);
	SetCtrlFont(_developer, "Tahoma", 0, 16);

	classlist.add(_hwnd, this);
}