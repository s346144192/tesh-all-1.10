#include <resource.h>
#include <base/encode/convert_utf_8.h>
#include <CtrlFunc.h>
#include <CodeEdit.h>
#include <tesh.h>
#include <text/jassmap.h>

struct NumberCount {
public:
	int* data;
	NumberCount() {
		data = nullptr;
	}
	~NumberCount() {
		if (data) {
			delete data;
		}
	}
	NumberCount(int* ints) {
		data = ints;
	}
	int operator[](size_t i) {
		return data[i];
	}
	int operator()(int add, size_t max) {
		int count = 0;
		for (size_t i = 0UL; i <= max; i++) {
			count += data[i] + add;
		}
		return count;
	}
};

void DrawButtonBackdrop(HWND hwnd, HDC hdc, RECT &rt, HBRUSH primary_hbr, HPEN primary_hpen,BOOL isdown) {
	int add_color=0;
	if (isdown) { 
		add_color=15;
	}
	HBRUSH hbr2 = CreateSolidBrush(RGB(225-add_color, 225 - add_color, 225 - add_color));
	SelectObject(hdc, hbr2);
	SetBkMode(hdc, TRANSPARENT);
	FillRect(hdc, &rt, hbr2);
	DeleteObject(hbr2);
	SelectObject(hdc, primary_hbr);
	HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204-add_color, 204 - add_color, 204 - add_color));
	SelectObject(hdc, pen2);
	SetBkMode(hdc, TRANSPARENT);
	Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
	DeleteObject(pen2);
	SelectObject(hdc, primary_hbr);
	SelectObject(hdc, primary_hpen);
}
void DrawButtonBackdrop(HWND hwnd, BOOL isdown) {
	if (!hwnd) {
		return;
	}
	int add_color = 0;
	if (isdown) {
		add_color = 15;
	}
	HDC hdc = GetDC(hwnd);
	RECT rt;
	GetClientRect(hwnd,&rt);
	HBRUSH hbr = CreateSolidBrush(RGB(225 - add_color, 225 - add_color, 225 - add_color));
	SelectObject(hdc, hbr);
	SetBkMode(hdc, TRANSPARENT);
	FillRect(hdc, &rt, hbr);
	DeleteObject(hbr);
	hbr=(HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(hdc, hbr);
	HPEN pen = CreatePen(PS_SOLID, 2, RGB(204 - add_color, 204 - add_color, 204 - add_color));
	SelectObject(hdc, pen);
	SetBkMode(hdc, TRANSPARENT);
	Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
	DeleteObject(hbr);
	DeleteObject(pen);
	::ReleaseDC(hwnd, hdc);
}


void DrawButton(HWND hwnd, HDC hdc, control_t* ctrl) {
	if (!ctrl) {
		return;
	}
	HBRUSH hbr = (HBRUSH)GetStockObject(NULL_BRUSH);//CreateSolidBrush(RGB(204, 204, 204));
	HPEN pen = (HPEN)GetStockObject(NULL_PEN);
	SetBkMode(hdc, TRANSPARENT);
	DrawButtonBackdrop(hwnd, hdc, ctrl->rt, hbr, pen, ctrl->isdown);

	if (ctrl->isfocus() && !ctrl->hidefocus) {
		RECT focus_rt = { ctrl->rt.left + 2,ctrl->rt.top + 2,ctrl->rt.right - 4,ctrl->rt.bottom - 4 };
		DrawFocusRect(hdc, &focus_rt);
	}
	int length = (ctrl->text) ? strlen(ctrl->text) : 0;
	if (length > 0) {

		if (ctrl->hfont) {
			SelectObject(hdc, ctrl->hfont);
		}
		else {
			SelectObject(hdc, GetButtonFont());
		}
		DrawTextA(hdc, ctrl->text, length, &ctrl->rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	DeleteObject(hbr);
	DeleteObject(pen);
}

void DrawButton(control_t* ctrl) {
	if (!ctrl) {
		return;
	}
	HDC hdc = GetDC(ctrl->parent);
	DrawButton(ctrl->parent, hdc, ctrl);
	::ReleaseDC(ctrl->parent, hdc);
}


void drawbuttonstate(HWND hwnd, HDC hdc, RECT &rt, HBRUSH primary_hbr, HPEN primary_hpen, BOOL is_focus, BOOL is_down) {
	if (is_focus) {
		HBRUSH hbr2;
		if (is_down) {
			hbr2 = CreateSolidBrush(RGB(225, 225, 225));
		}
		else {
			hbr2 = CreateSolidBrush(GetSysColor(COLOR_MENU));
		}
		SelectObject(hdc, hbr2);
		SetBkMode(hdc, TRANSPARENT);
		FillRect(hdc, &rt, hbr2);
		DeleteObject(hbr2);
		SelectObject(hdc, primary_hbr);
		HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
		SelectObject(hdc, pen2);
		SetBkMode(hdc, TRANSPARENT);
		Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
		DeleteObject(pen2);
		SelectObject(hdc, primary_hbr);
		SelectObject(hdc, primary_hpen);
	}
	else {
		//InvalidateRect(hwnd, &rt, TRUE);
	}
}


BindList<HWND, FindTextCtrl*> FindTextCtrl::classlist;
BindList<HWND, CodeEditView*> CodeEditView::classlist;


LRESULT CALLBACK FindTextCtrl::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int id = classlist.getid(hwnd);
	if (id < 0) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	FindTextCtrl* __find_text=classlist.at(id).second;
	id = CodeEditView::classlist.getid(__find_text->_parent);
	if (id < 0) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	CodeEditView* cew = CodeEditView::classlist.at(id).second;
	switch (message)
	{
	case WM_PAINT:
		cew->_findtext.paint();
		return TRUE;
	case WM_COMMAND:
		cew->FindTextEvent(LOWORD(wParam), HIWORD(wParam));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		//SetBkMode((HDC)wParam, TRANSPARENT);
		SetBkColor((HDC)wParam, GetSysColor(COLOR_MENU));
		return (INT_PTR)(HBRUSH)GetStockObject(NULL_BRUSH); //背景画刷
		break;
	case WM_MOUSEMOVE:
	{
		POINT pos; GetCursorPos(&pos);
		ScreenToClient(cew->_findtext._main, &pos);
		cew->_findtext.controls.focusupdate(pos.x, pos.y);
	}
	break;
	case WM_LBUTTONDOWN:
		if (control_t::focus) {
			control_t::focus->isdown = cew->FindTextEvent(control_t::focus->id, 0);
			if (!control_t::focus->singlesel){
				control_t::focus->isdown = TRUE;
			}
			cew->_findtext.drawitem(control_t::focus);
		}
		break;
	case WM_LBUTTONUP:
		if (control_t::focus) {
			if (!(control_t::focus->singlesel)) {
				control_t::focus->isdown = FALSE;
				cew->_findtext.drawitem( control_t::focus);
			}
		}
		break;
	case WM_USER_CONTROL_UP:
	{
		control_t* ctrl = cew->_findtext.controls.get(wParam);
		if (ctrl) {
			cew->_findtext.drawitem(ctrl);
		}
		return TRUE;
	}
	break;
	default:
		break;
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void FindTextCtrl::Init(HWND parent, int x, int y){
	NumberCount lines = NumberCount(new int[4]{ 0,25,25,0 });
	NumberCount columns1 = NumberCount(new int[12] { 0,25,200,25,25,25,2,25,25,25,25,0 });
	NumberCount columns2 = NumberCount(new int[12] {0,25,200,25,25,0,0,0,0,0,0,0 });
	_width = columns1(2, 10);
	_height = lines(2, 2);
	_foldheight = lines(2, 1);
	_parent = parent;
	_main = CreateWindowExA(NULL,"FindTextCtrl", NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		x, y, _width, _height, _parent, (HMENU)IDR_FINDTEXT_MAIN, nullptr, nullptr);
	
	controls.add(control_t(">", _main, IDR_FINDTEXT_FOLD, { columns1(2, 0), lines(2, 0),25,25 },TRUE));
	_findput = CreateWindowA("Edit", NULL, WS_CHILD | ES_AUTOHSCROLL | WS_VISIBLE
		, columns1(2, 1), lines(2, 0), columns1[2], 25, _main, (HMENU)IDR_FINDTEXT_PUT, nullptr, nullptr);
	_replaceput = CreateWindowA("Edit", NULL, WS_CHILD| ES_AUTOHSCROLL | WS_VISIBLE
		, columns2(2, 1), lines(2, 1), columns2[2], 25, _main, (HMENU)IDR_FINDTEXT_REPPUT, nullptr, nullptr);
	controls.add(control_t( "Aa", _main, IDR_FINDTEXT_ENABLECASE,  { columns1(2, 2), lines(2, 0),25,25 }, TRUE, TRUE));
	controls.add(control_t("Ab ", _main, IDR_FINDTEXT_FULLMATCH, { columns1(2, 3), lines(2, 0),25,25 }, TRUE, TRUE));
	controls.add(control_t( "*", _main, IDR_FINDTEXT_REGEX,  { columns1(2, 4), lines(2, 0),25,25 }, TRUE, TRUE));
	_find_count = CreateWindowA("static", NULL, SS_CENTERIMAGE | SS_CENTER | WS_CHILD | WS_VISIBLE
		, columns1(2, 5), lines(2, 0), 0, 25, _main, (HMENU)IDR_FINDTEXT_COUNT, nullptr, nullptr);
	controls.add(control_t( "<", _main, IDR_FINDTEXT_PREV,  { columns1(2, 6), lines(2, 0),25,25 }, TRUE));
	controls.add(control_t( ">", _main, IDR_FINDTEXT_NEXT,  { columns1(2, 7), lines(2, 0),25,25 }, TRUE));
	controls.add(control_t("", _main, IDR_FINDTEXT_TATGETRANGE, { columns1(2, 8), lines(2, 0),25,25 }, TRUE,TRUE));
	controls.add(control_t( "×", _main, IDR_FINDTEXT_CLOSE,  { columns1(2, 9), lines(2, 0),25,25 }, TRUE));

	controls.add(control_t("", _main, IDR_FINDTEXT_REP, { columns1(2, 2), lines(2, 1),25,25 }, TRUE));
	controls.add(control_t("", _main, IDR_FINDTEXT_REPALL,  { columns1(2, 3), lines(2, 1),25,25 }, TRUE));
	
	SendMessageA(_findput, EM_LIMITTEXT,150,0);
	SendMessageA(_replaceput, EM_LIMITTEXT, 150, 0);

	SetCtrlFont(_findput, "Courier New", 0, 20);
	SetCtrlFont(_replaceput, "Courier New", 0, 20);
	SetCtrlFont(_find_count, "Courier New", 0, 20);

	controls.get(IDR_FINDTEXT_FOLD)->hfont = CreateNewFont("新宋体",17,700);
	controls.get(IDR_FINDTEXT_FULLMATCH)->hfont = CreateNewFont("Tahoma", 14, 400);
	controls.get(IDR_FINDTEXT_PREV)->hfont = CreateNewFont("新宋体", 19, 700);
	controls.get(IDR_FINDTEXT_NEXT)->hfont = CreateNewFont("新宋体", 19, 700);
	controls.get(IDR_FINDTEXT_CLOSE)->hfont = CreateNewFont("新宋体", 19, 700);
	controls.get(IDR_FINDTEXT_REP)->hfont = CreateNewFont("Tahoma", 9, 700);
	controls.get(IDR_FINDTEXT_REPALL)->hfont = CreateNewFont("Tahoma", 9, 700);
	fold(true);
	classlist.add(_main,this);
}

bool FindTextCtrl::isshow() {
	return IsWindowVisible(_main);
}
void FindTextCtrl::show(bool enable, bool enablefold, bool setfocus, const char* text) {
	int id = CodeEditView::classlist.getid(_parent);
	CodeEditView* cew = NULL;
	if (id >= 0) {
		cew = CodeEditView::classlist.at(id).second;
	}
	if (enable) {
		ShowTopHwnd(_main,true);
		//ShowWindow(_main, SW_SHOW);
		fold(enablefold,setfocus);
		if (setfocus) {
			SetFocus(_findput);
		}
		if (text) {
			if (strlen(text)>0) {
				SetEditText(_findput, text);
			}
		}
		if (cew) {
			cew->init_search_range();
			cew->_searching=true;
		}
		
	}
	else {
		ShowTopHwnd(_main, false);
		//ShowWindow(_main, SW_HIDE);
		SendMessageA(_findput, EM_SETSEL, -1, -1);
		if (cew) {
			cew->_searching = false;
			cew->SetsSearchStringsClear();
		}
	}
}
void FindTextCtrl::setpos(int x, int y) {
	SetWindowPos(_main, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}
void FindTextCtrl::uppos() {
	RECT rect;
	GetClientRect(_parent, &rect);
	setpos(rect.right - _width - 24, _height-15);
}
void FindTextCtrl::setsize(int w, int h) {
	SetWindowPos(_main, NULL, 0, 0, w, h, SWP_NOMOVE);
	//SetWindowPos(splitter._hwnd, NULL, 0, 0, 2, h, SWP_NOMOVE);
}
void FindTextCtrl::setbuttonstate(int _type, bool _enable) {
	if (_type== IDR_FINDTEXT_FOLD) {
		 buttons_state[0]= _enable;
	}
	else if (_type == IDR_FINDTEXT_ENABLECASE) {
		buttons_state[1] = _enable;
	}
	else if (_type == IDR_FINDTEXT_FULLMATCH) {
		buttons_state[2] = _enable;
	}
	else if (_type == IDR_FINDTEXT_REGEX) {
		buttons_state[3] = _enable;
	}
	else if (_type == IDR_FINDTEXT_TATGETRANGE) {
		buttons_state[4] = _enable;
	}
}
bool FindTextCtrl::getbuttonstate(int _type) {
	if (_type == IDR_FINDTEXT_FOLD) {
		return buttons_state[0];
	}
	else if (_type == IDR_FINDTEXT_ENABLECASE) {
		return buttons_state[1];
	}
	else if (_type == IDR_FINDTEXT_FULLMATCH) {
		return buttons_state[2];
	}
	else if (_type == IDR_FINDTEXT_REGEX) {
		return buttons_state[3];
	}
	else if (_type == IDR_FINDTEXT_TATGETRANGE) {
		return buttons_state[4];
	}
	return false;
}

void FindTextCtrl::fold(bool enable, bool setfocus) {
	setbuttonstate(IDR_FINDTEXT_FOLD, enable);
	control_t* ctrl = controls.get(IDR_FINDTEXT_FOLD);
	if (enable) {
		ShowWindow(_replaceput, SW_HIDE);
		if (ctrl) {
			ctrl->setsize(25, _foldheight-4);
		}
		setsize(_width, _foldheight);
	}
	else {
		ShowWindow(_replaceput, SW_SHOW);
		if (ctrl) {
			ctrl->setsize(25, _height - 4);
		}
		setsize(_width, _height);
	}
	if (setfocus) {
		SetFocus(_findput);
	}
	//InvalidateRect(_main, NULL, TRUE);
}
bool FindTextCtrl::isfold() {
	return getbuttonstate(IDR_FINDTEXT_FOLD);
}
void FindTextCtrl::paint() {
	PAINTSTRUCT pt;
	HDC hdc = BeginPaint(_main, &pt);
	RECT rt = pt.rcPaint;
	GetClientRect(_main, &rt);
	HBRUSH hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	HPEN pen = CreatePen(PS_SOLID, 2, RGB(225, 225, 225));
	SelectObject(hdc, hbr);        //选择画刷
	SelectObject(hdc, pen);        //选择画笔
	Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
	DeleteObject(hbr); DeleteObject(pen);

	int i = 0;
	for (control_t* ctrl = controls.at(i); ctrl;) {
		drawitem(ctrl);
		i++;
		ctrl = controls.at(i);
	}

	EndPaint(_main, &pt);
}

void FindTextCtrl::drawitem(control_t* ctrl) {
	if (!ctrl) {
		return;
	}
	HDC hdc = GetDC(ctrl->parent);
	HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_MENU)); //(HBRUSH)GetStockObject(NULL_BRUSH); //CreateSolidBrush(GetSysColor(COLOR_MENU));// (HBRUSH)GetStockObject(NULL_BRUSH);
	HPEN pen = (HPEN)GetStockObject(NULL_PEN);
	RECT rt = ctrl->rt;
	SelectObject(hdc, hbr);        //选择画刷
	SelectObject(hdc, pen);        //选择画笔
	Rectangle(hdc, rt.left-1, rt.top-1, rt.right+2, rt.bottom + 2);
	SetBkMode(hdc, TRANSPARENT);
	switch (ctrl->id)
	{
	case IDR_FINDTEXT_FOLD:
		drawbuttonstate(ctrl->parent, hdc, rt, hbr, pen, ctrl->isfocus(), ctrl->isdown);
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		if (getbuttonstate(IDR_FINDTEXT_FOLD)) {
			DrawTextA(hdc, ">", 1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
		else {
			//字体较大时粗体失效
			XFORM xform;
			SetGraphicsMode(hdc, GM_ADVANCED);
			xform.eM11 = cos(0.5*3.1415926); xform.eM12 = sin(0.5*3.1415926); xform.eM21 = -sin(0.5*3.1415926); xform.eM22 = cos(0.5*3.1415926);
			xform.eDx = (float)(rt.right / 2 - xform.eM11*rt.right / 2 + xform.eM12*rt.right / 2) -2;
			xform.eDy = (float)(rt.right / 2 - xform.eM11*rt.right / 2 - xform.eM12*rt.right / 2);
			rt.right = rt.bottom;
			SetWorldTransform(hdc, &xform);
			DrawTextA(hdc, ">", 1, &rt, DT_CENTER | DT_VCENTER);
			xform.eM11 = (float)1.0; xform.eM12 = (float)0; xform.eM21 = (float)0; xform.eM22 = (float)1.0; xform.eDx = (float)0; xform.eDy = (float)0;
			SetWorldTransform(hdc, &xform);
			SetGraphicsMode(hdc, GM_COMPATIBLE);
		}
		break;
	case IDR_FINDTEXT_ENABLECASE:
		if (getbuttonstate(IDR_FINDTEXT_ENABLECASE)) {
			HBRUSH hbr2 = CreateSolidBrush(RGB(204, 204, 204));
			SelectObject(hdc, hbr2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
		}
		SelectObject(hdc, pen);
		SelectObject(hdc, (ctrl->hfont)? ctrl->hfont: GetButtonFont());
		DrawTextA(hdc, "Aa", 2, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		break;
	case IDR_FINDTEXT_FULLMATCH:
	{
		if (getbuttonstate(IDR_FINDTEXT_FULLMATCH)) {
			HBRUSH hbr2 = CreateSolidBrush(RGB(204, 204, 204));
			SelectObject(hdc, hbr2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
		}
		RECT rt2 = { rt.left - 1,rt.top + 0,rt.right - 1 ,rt.bottom };
		HPEN pen2 = CreatePen(PS_ENDCAP_FLAT, 2, RGB(0, 0, 0));
		SelectObject(hdc, pen2);
		Rectangle(hdc, rt.left + 2, rt.top + 5, rt.right - 4, rt.top + 6);
		Rectangle(hdc, rt.left + 2, rt.bottom - 6, rt.right - 4, rt.bottom - 5);
		Rectangle(hdc, rt.right - 6, rt.top + 7, rt.right - 5, rt.bottom - 7);
		DeleteObject(pen2);
		SelectObject(hdc, hbr);
		SelectObject(hdc, pen);
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		DrawTextA(hdc, "Ab ", 3, &rt2, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
		break;
	case IDR_FINDTEXT_REGEX:
	{
		if (getbuttonstate(IDR_FINDTEXT_REGEX)) {
			HBRUSH hbr2 = CreateSolidBrush(RGB(204, 204, 204));
			SelectObject(hdc, hbr2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(hbr2);

			SelectObject(hdc, hbr);
		}
		SelectObject(hdc, pen);
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		DrawTextA(hdc, "*", 1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		HBRUSH hbr2 = CreateSolidBrush(RGB(0, 0, 0));
		HPEN pen2 = CreatePen(PS_ENDCAP_FLAT, 2, RGB(0, 0, 0));
		SelectObject(hdc, hbr2);
		SelectObject(hdc, pen2);
		Rectangle(hdc, rt.left + 5, rt.bottom - 9, rt.left + 9, rt.bottom - 5);
		DeleteObject(hbr2);
		DeleteObject(pen2);
		hbr2 = NULL;
		pen2 = NULL;
	}
		break;
	case IDR_FINDTEXT_TATGETRANGE:
	{
		if (getbuttonstate(IDR_FINDTEXT_TATGETRANGE)) {
			HBRUSH hbr2 = CreateSolidBrush(RGB(204, 204, 204));
			SelectObject(hdc, hbr2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(hbr2);

			SelectObject(hdc, hbr);
		}
		SelectObject(hdc, pen);
		HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		SelectObject(hdc, pen2);
		LONG center_y = (rt.bottom - rt.top) / 2;
		Rectangle(hdc, rt.left + 2, center_y - 5, rt.right - 7, center_y - 4);
		Rectangle(hdc, rt.left + 2, center_y+1, rt.right - 4, center_y + 2);
		Rectangle(hdc, rt.left + 2, center_y + 6, rt.right - 10, center_y + 7);
		DeleteObject(pen2);
		pen2 = NULL;
	}
		break;
	case IDR_FINDTEXT_PREV:
	{
		if (ctrl->isfocus()) {
			HBRUSH hbr2 = NULL;
			if (ctrl->isdown) {
				hbr2 = CreateSolidBrush(RGB(217, 217, 217));
				SelectObject(hdc, hbr2);
			}
			HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
			SelectObject(hdc, pen2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(pen2);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
			SelectObject(hdc, pen);
		}
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		DrawTextA(hdc, "<", 1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	break;
	case IDR_FINDTEXT_NEXT:
	{
		if (ctrl->isfocus()) {
			HBRUSH hbr2 = NULL;
			if (ctrl->isdown) {
				hbr2 = CreateSolidBrush(RGB(217, 217, 217));
				SelectObject(hdc, hbr2);
			}
			HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
			SelectObject(hdc, pen2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(pen2);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
			SelectObject(hdc, pen);
		}
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		DrawTextA(hdc, ">", 1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
		break;
	case IDR_FINDTEXT_CLOSE:
	{
		if (ctrl->isfocus()) {
			HBRUSH hbr2 = NULL;
			if (ctrl->isdown) {
				hbr2 = CreateSolidBrush(RGB(217, 217, 217));
				SelectObject(hdc, hbr2);
			}
			HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
			SelectObject(hdc, pen2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(pen2);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
			SelectObject(hdc, pen);
		}
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		DrawTextA(hdc, "×", 2, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
		break;
	case IDR_FINDTEXT_REP:
	{
		if (ctrl->isfocus()) {
			HBRUSH hbr2 = NULL;
			if (ctrl->isdown) {
				hbr2 = CreateSolidBrush(RGB(217, 217, 217));
				SelectObject(hdc, hbr2);
			}
			HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
			SelectObject(hdc, pen2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(pen2);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
			SelectObject(hdc, pen);
		}
		HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		SelectObject(hdc, pen2);
		RoundRect(hdc, rt.left + 4, rt.bottom - 14, rt.right - 10, rt.bottom - 2, 2, 2);
		DeleteObject(pen2);
		pen2 = NULL;
		SelectObject(hdc, pen);
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		RECT rt2 = { rt.left + 5, rt.top + 1, rt.right - 2, rt.bottom };
		DrawTextA(hdc, "↓b", 3, &rt2, DT_SINGLELINE | DT_TOP);
		RECT rt3 = { rt.left + 4, rt.bottom - 14, rt.right - 10, rt.bottom - 4 };
		DrawTextA(hdc, "c", 1, &rt3, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
		break;
	case IDR_FINDTEXT_REPALL:
	{
		if (ctrl->isfocus()) {
			HBRUSH hbr2 = NULL;
			if (ctrl->isdown) {
				hbr2 = CreateSolidBrush(RGB(217, 217, 217));
				SelectObject(hdc, hbr2);
			}
			HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(204, 204, 204));
			SelectObject(hdc, pen2);
			Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
			DeleteObject(pen2);
			DeleteObject(hbr2);
			SelectObject(hdc, hbr);
			SelectObject(hdc, pen);
		}
		HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		SelectObject(hdc, pen2);
		RoundRect(hdc, rt.left + 4, rt.bottom - 15, rt.right - 7, rt.bottom - 2, 2, 2);
		RoundRect(hdc, rt.left + 4, rt.bottom - 12, rt.right - 10, rt.bottom - 2, 2, 2);
		DeleteObject(pen2);
		pen2 = NULL;
		SelectObject(hdc, pen);
		SelectObject(hdc, (ctrl->hfont) ? ctrl->hfont : GetButtonFont());
		RECT rt2 = { rt.left + 5, rt.top + 1, rt.right - 2, rt.bottom };
		DrawTextA(hdc, "↓ab", 4, &rt2, DT_SINGLELINE | DT_TOP);
		RECT rt3 = { rt.left + 4, rt.bottom - 12, rt.right - 10, rt.bottom - 4 };
		DrawTextA(hdc, "bc", 2, &rt3, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		
	}
		break;
	default:
		break;
	}
	
	DeleteObject(hbr); DeleteObject(pen);
	::ReleaseDC(ctrl->parent, hdc);
}


FindTextCtrl::FindTextCtrl() {
	WinClass.Register("FindTextCtrl", WndProc);
	for (int i = 0; i < sizeof(buttons_state); i++) {
		buttons_state[i] = false;
	}
}
FindTextCtrl::~FindTextCtrl() {
	WinClass.UnRegister("FindTextCtrl");
}

CodeEditView::CodeEditView() 
{
	_hwnd = NULL;
	scroll_pos = -1;
	always_fold = 0;
	_edit_state = 0;
	EditFocusCallback = NULL;
}
CodeEditView::~CodeEditView()
{
	_hwnd = NULL;
	_edit_state = 0;
	EditFocusCallback = NULL;
}
////获取菜单栏的高度
	//MENUBARINFO menuInfo{};
	//menuInfo.cbSize = sizeof(MENUBARINFO);
	//int rtn = GetMenuBarInfo(parent, OBJID_MENU, 0, &menuInfo);
	//int menu_bar_height = 0;
	//if (rtn != 0)
	//	menu_bar_height = menuInfo.rcBar.bottom - menuInfo.rcBar.top;



BOOL CodeEditView::SetEditPutText(LPCSTR lpString) {

	BOOL result = FALSE;
	scroll_pos = 0;
	always_fold = 0;
	if (!(*lpString)) {
		result = SetText( "");
	}
	else {
		LPSTR CurString = (char*)lpString;
		simple_analys analys(NULL);
		size_t len = 0;
		if (CurString[0] == '/') {
			if (strncmp(CurString, "//TESH.scrollpos=", 17) == 0) {
				CurString += 17;
				scroll_pos = analys.get_int(CurString, &len);
				analys.next_line(CurString, TRUE);
			}
		}
		if (CurString[0] == '/') {
			if (strncmp(CurString, "//TESH.alwaysfold=", 18) == 0) {
				CurString += 18;
				always_fold = analys.get_int(CurString, &len);
				analys.next_line(CurString, TRUE);
			}
		}
		if (IsEnableUTF8) {
			int length = strlen(lpString) * 2;
			length -= (CurString - lpString);
			char * str = (char*)malloc(length + 4);
			base::G2U(CurString, str, length + 2);
			result = SetText(str);
			free(str);
		}
		else {
			result = SetText(CurString);
		}
	}
	EmptyUndoBuffer();
	ResetEditBox(false);
	AutoAlwaysFoldAll(TRUE);
	SetFirstVisibleLine(GetLineCount()-1);//跳到最后一行(移动滚动条不会发送不必要的文档已更改消息)
	if (scroll_pos > 0 && enable_save_scroll_postion) {
		SetFirstVisibleLine(scroll_pos);//跳到保存的行
	}
	else {
		SetFirstVisibleLine(0);//跳回第一行
	}
	_edit_state = 0;
	_edit_once_change = FALSE;
	//printf("设置文本结束\n");
	if (_findtext.isshow() && get_search_text().length()>0) {
		search_next();
		SetWindowPos(_findtext._main, 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE);
	}
	return result;
}
LPSTR CodeEditView::GetEditPutText() {
	int length = GetTextLength();
	if (length <= 0) {
		return nullptr;
	}
	char* curstr = nullptr;
	int flaglen = 0;
	char teshflag[100] = { 0 };
	if ((enable_save_scroll_postion || scroll_pos > 0) || always_fold) {
		int curpos = (enable_save_scroll_postion) ? GetFirstVisibleLine() : scroll_pos;
		//printf("保存当前行位置:%d\n", curpos);
		sprintf_s(teshflag, "//TESH.scrollpos=%d\n//TESH.alwaysfold=%d\n", curpos, always_fold);
		flaglen = strlen(teshflag);
	}
	char* srcstr = (char*)malloc(length + flaglen + 2);
	curstr = srcstr;
	if (flaglen > 0) {
		memcpy(srcstr, teshflag, flaglen);
		curstr += flaglen;
	}
	length = GetText((uptr_t)length + 1, curstr);
	curstr = srcstr;
	if (length>0){
		length = length + flaglen;
		if (IsEnableUTF8) {
			char* gbstr = (char*)malloc((length + 2));
			base::U2G(srcstr, gbstr, length + 1);
			free(srcstr);
			curstr = gbstr;
		}
	}
	return curstr;
}

void CodeEditView::SetConfig(const char* node, const char* keyword, const char* value) {
	if (!_Options || !node || !keyword || !value) {
		return;
	}
	ini::config_t* config = _Options->config;
	if (config) {
		(*config)[node][keyword] = value;
	}
}
void  CodeEditView::SetConfig(const char* node, const char* keyword, int value) {
	if (!_Options || !node || !keyword) {
		return;
	}
	ini::config_t* config = _Options->config;
	if (config) {
		(*config)[node][keyword] = value;
	}
}
int CodeEditView::GetConfigInt(const char* node, const char* keyword, int default_value) {
	if (!_Options || !node || !keyword) {
		return default_value;
	}
	ini::config_t* config = _Options->config;
	if (config) {
		return config->get_int(node, keyword, default_value);
	}
	return default_value;
}

void CodeEditView::ShowFoldExt(bool enable) {
	SetConfig("Editor", "DoFold", (int)enable);
	ShowFold(enable);
}
void CodeEditView::ShowLineNumberExt(bool enable) {
	SetConfig("Editor", "LineNum", (int)enable);
	if (enable) {
		SetLineNumberWidth(line_num_width);
	}
	else {
		SetLineNumberWidth(0);
	}
}
void CodeEditView::EnableAutoCompleteExt(bool enable) {
	SetConfig("Editor", "AutoComplete", (int)enable);
	enable_auto_complete = enable;
}
void CodeEditView::EnableCallTipExt(bool enable) {
	SetConfig("Editor", "CallTip", (int)enable);
	enable_calltip = enable;
}
void CodeEditView::EnableZoomLevelExt(bool enable) {
	SetConfig("Editor", "ZoomLevel", (int)enable);
	if (enable) {
		SetZoomLevelMinMax(-5, 10);
	}
	else {
		SetZoomLevelMinMax(0, 0);
	}
}
void CodeEditView::EnableAlwaysFoldAllExt(BOOL enable) {
	SetConfig("Editor", "AlwaysFoldAll", (int)enable);
	enable_alwaysfoldall = enable;
	SetCheck(_always_fold_all_checkbox, enable);
}
void CodeEditView::EnableSaveSrcollPosExt(BOOL enable) {
	SetConfig("Editor", "SaveScrollPos", (int)enable);
	enable_save_scroll_postion = enable;
	SetCheck(_save_scroll_postion_checkbox, enable);
}
void CodeEditView::SetTabLineExt(BOOL enable) {
	SetConfig("Editor", "TabLine", (int)enable);
	SendEditor(SCI_SETINDENTATIONGUIDES, (enable)? 3:0);
}
void CodeEditView::SetAutoFillSpaceExt(BOOL enable) {
	SetConfig("Editor", "AutoFillSpace", (int)enable);
	enable_autofillspace = enable;
}
void CodeEditView::EnableUTF8Ext(BOOL enable) {
	if (GetConfigInt("Other", "EnableUTF8", (int)enable) != enable) {
		SetConfig("Other", "EnableUTF8", (int)enable);
	}
	EnableUTF8(enable);
}
void CodeEditView::EnableD2dDrawExt(BOOL enable) {
	if (GetConfigInt("Other", "EnableD2dDraw", (int)enable) != enable) {
		SetConfig("Other", "EnableD2dDraw", (int)enable);
	}
	EnableD2dDraw(enable);
}
void CodeEditView::EnableEnableAntiAliasExt(int type) {
	if (GetConfigInt("Other", "EnableAntiAlias", (int)type) != type) {
		SetConfig("Other", "EnableAntiAlias", (int)type);
	}
	if (type==1) {
		SetFontQuality(SC_EFF_QUALITY_ANTIALIASED);
	}
	else if (type == 2) {
		SetFontQuality(SC_EFF_QUALITY_LCD_OPTIMIZED);
	}
	else
	{
		SetFontQuality(SC_EFF_QUALITY_DEFAULT);
	}
}
void  CodeEditView::SetEditFontExt(const char* name, int size) {
	if (SetEditMainFontName(name)) {
		SetConfig("EditorFont", "Name", name);
	}
	if (SetEditMainFontSize(size)) {
		SetConfig("EditorFont", "Size", size);
	}
}
void  CodeEditView::SetLineNumFontExt(const char* name, int size) {
	if (SetFontName(STYLE_LINENUMBER, name)) {
		SetConfig("LineNumFont", "Name", name);
	}
	if (size >= 8 && size <= 72) {
		SetFontSize(STYLE_LINENUMBER, size);
		SetConfig("LineNumFont", "Size", size);
	}
}

void CodeEditView::AutoAlwaysFoldAll(BOOL IsInitFold) {
	//printf("折叠:%d\n", enable_alwaysfoldall);
	if (enable_alwaysfoldall) {
		//先展开全部,使折叠应用。
		if (IsInitFold) {
			EnsureStyledAll();
			//SetFoldAll(SC_FOLDACTION_EXPAND);
		}
		SetFoldFirstLevel(TRUE);
		//SetFoldAll(SC_FOLDACTION_CONTRACT);
	}
}

int CodeEditView::LoadCompleteList(const char* word, int wn,size_t max,bool isluaword) {
	if (max == 0) { return 0; }
	static const char* jass_keywordslist[] = { JassKeywords ,JassKeywords2,JassKeywords3,JassTypes };
	static const int jass_keywordslist_count = sizeof(jass_keywordslist) / sizeof(const char*);
	static const char* lua_keywordslist[] = { LuaKeywords};
	static const int lua_keywordslist_count = sizeof(lua_keywordslist) / sizeof(const char*);

	static jass_map* jass_text_list[] = { &cj_text, &bj_text, &custom_function_text
			,&cj_globals_text, &bj_globals_text, &custom_globals_text };
	static const int jass_text_list_count = sizeof(jass_text_list) / sizeof(jass_map*);
	
	size_t n = 0;
	AutoCompleteListCount = 0;
	AutoCompleteList.clear();

	const char** curkeywordslist = jass_keywordslist;
	int curkeywordslist_count = jass_keywordslist_count;
	if (isluaword) {
		curkeywordslist = lua_keywordslist;
		curkeywordslist_count = lua_keywordslist_count;
	}
	for (int i = 0; i < curkeywordslist_count; i++) {
		const char* str = curkeywordslist[i];
		int wlen = 0;
		for (; (str = strstr(str, word)); ++str) {
			if (str == JassKeywords || isspace(str[-1])) {
				if (!AutoCompleteList.empty()) { AutoCompleteList.push_back(' '); }
				wlen = strcspn(str, " "); AutoCompleteList.append(str, wlen); str += (wlen - 1);
				if (++n >= max) { AutoCompleteList.pop_back(); return n; }
			}
		}
	}

	for (int i = 0; i < jass_text_list_count; i++) {
		for (auto it = jass_text_list[i]->begin(); it != jass_text_list[i]->end(); ++it) {
			if (strncmp((*it).first.c_str(),word,wn)==0) {
				if (!AutoCompleteList.empty()) {
					AutoCompleteList.push_back(' ');
				}
				AutoCompleteList.append((*it).first);
				if (++n>=max) {
					AutoCompleteList.pop_back();
					return n;
				}
			}
		}
	}
	return n;
}

std::string CodeEditView::GetCallTip(std::string function_name) {
	static jass_map* func_map_list[] = { &cj_text, &bj_text, &custom_function_text};
	static const int func_map_list_count = sizeof(func_map_list) / sizeof(jass_map*);
	std::string function_code("");
	for (int i = 0; i < func_map_list_count; i++) {
		auto it = func_map_list[i]->find(function_name);
		if (it != func_map_list[i]->end()) {
			function_code = (*it).second;
			break;
		}
	}
	if (!function_code.length()) {
		return "";
	}
	std::string returns("");
	char* st_args = nullptr;
	char* end_args = nullptr;
	simple_analys analys((char*)function_code.c_str());
	analys.next_word(true);
	for (; *analys._pos;) {
		if (*analys._pos=='\n'|| *analys._pos=='\r') {
			return "";
		}
		if (analys.test_word("takes")) {
			st_args = analys._pos+sizeof("takes");
		}
		else if (analys.test_word("returns")) {
			if (!st_args) {
				return "";
			}
			char* ret_start = analys._pos;
			end_args = analys._pos - 1;
			analys.next_word_s(false);
			analys.next_spacechar();
			if (ret_start<analys._pos) {
				returns.assign(ret_start, analys._pos - ret_start);
			}
			
			break;
		}
		else if (analys.test_word("nothing")) {
			return "";
		}
		analys.next_spacechar();
		analys.next_word(false);
	}
	if (st_args< end_args) {
		std::string tip(function_name);
		tip.push_back('(');
		tip.append(st_args, end_args - st_args);
		tip.push_back(')');
		if (returns.length()) {
			tip.push_back('\n');
			tip.append(returns);
		}
		return tip;
	}
	return "";
}


void CodeEditView::ShowCompleteList() {
	int pos = GetCurrentPos(); //当前位置 
	int startpos = GetWordStartPos(pos);//当前单词起始位置 
	int len = pos - (int)startpos;
	static int savedstartpos = 0;
	static int savedlen = 0;
	if (savedstartpos != startpos) {
		AutoCompleteListCount = 0;
	}
	savedstartpos = startpos;
	savedlen = len;
	if (len < 3) {
		return;
	}
	if (AutoCompleteActive() && AutoCompleteListCount > 0) {
		if (AutoCompleteListCount < (100-min(90,2 << ((len) >> 1 ) ) ))  {
			return;
		}
	}
	const char* word = GetWordText(pos, startpos);
	if (!word) {
		return;
	}
	int OldCount = AutoCompleteListCount;
	bool isluaword = GetStyleAt(startpos) >= SCE_LUA_DEFAULT;
	AutoCompleteListCount = LoadCompleteList(word, len, 100, isluaword);
	if (AutoCompleteListCount != OldCount && AutoCompleteListCount > 0) {
		if (AutoCompleteListCount==1 && (len== AutoCompleteList.length() || AutoCompleteList.length() < 6 )) {
			return;
		}
		AutoCompleteShow(len, AutoCompleteList.data());
	}
}

const char* CodeEditView::GetWordText(int pos, int startpos) {
	static char word[CALLTIP_BUFFER_SIZE];
	//int pos = SendEditor(SCI_GETCURRENTPOS); //当前位置 
	//int startpos = SendEditor(SCI_WORDSTARTPOSITION, pos);//当前单词起始位置 
	int len = pos - startpos;
	if (len >= sizeof(word) || len <= 0) {
		return nullptr;
	}
	ZeroMemory(word, sizeof(word));
	GetTextRange(word, startpos, pos);
	word[sizeof(word) - 1] = 0;
	return word;
}
void CodeEditView::CallTipHighlight() {
	int cur_pos= GetCurrentPos();
	if (cur_pos<calltip_start_pos || !(cur_pos-calltip_start_pos< CALLTIP_MATCH_MAX_RANGE)) {
		SetCallTipCancel();
		return;
	}
	std::string textbuffer = GetTextRange(calltip_start_pos, cur_pos);
	if (textbuffer.length()==0) {
		SetCallTipCancel();
		return;
	}
	const char* func = textbuffer.data();
	const char* pos = strchr(func,'(');
	int cur_dim_count = 0;
	if (pos) {
		++pos;
		int complevel = 0;
		bool brace = false;
		char ch_prev = 0;
		for (char ch = 0; (ch = *pos); ++pos) {
			if (ch_prev!='\\' && (ch == '\"' || ch == '\'')) {
				brace = !brace;
			}
			if (!brace) {
				if (ch == '(') {
					complevel++;
				}
				else if (ch == ')') {
					complevel--;
				}
				else if (ch == ',' && complevel == 0) {
					cur_dim_count++;
				}
			}
			ch_prev = ch;
		}
	}
	const char* str = calltip_text.c_str();
	const char* st = strchr(str,'(');
	const char* end = nullptr;
	const char* dim = st;
	if (!st) {
		return;
	}
	if (cur_dim_count >=2) {
		int i = 0;
	}
	int dim_count = -1;
	for (; dim_count < cur_dim_count;++dim) {
		if (!(dim = strchr(dim, ','))) {
			dim = strchr(str, ')');
		}
		if (!dim) {
			break;
		}
		if (end) {
			st = end;
		}
		end = dim;
		dim_count++;
	}
	if (end > 0 && st + 1 < end) {
		SetCallTipHighlight((int)((int)st - (int)str + 1), (int)end - (int)str);
	}
	
	
}

void CodeEditView::ShowCallTip() {
	int pos = GetCurrentPos(); //当前位置 
	int startpos = GetWordStartPos( pos-1);//当前单词起始位置 
	const char* word = GetWordText(pos-1, startpos);
	if (!word) {
		return;
	}
	int len = pos -1- (int)startpos;
	if (len > 0) {
		calltip_text = GetCallTip(word);
		if (calltip_text.length()) {
			SetCallTipShow(startpos, calltip_text.data());
			CallTipHighlight();
		}
	}
}
bool CodeEditView::ShowParentCallTip() {
	if (!calltip_start_list.empty()) {
		calltip_start_list.pop();
	}
	if (calltip_start_list.empty()) {
		return false;
	}
	int start_pos = 0;
	int end_pos = 0;
	const char* word = nullptr;
	while (!calltip_start_list.empty()) {
		start_pos = calltip_start_list.top();
		end_pos = GetWordEndPos(start_pos);
		if (start_pos < end_pos && (end_pos - start_pos < CALLTIP_BUFFER_SIZE)) {
			word = GetWordText(end_pos, start_pos);
			if (word) {
				break;
			}
		}
		calltip_start_list.pop();
	}
	int len = end_pos - (int)start_pos;
	if (word && len > 0) {
		calltip_text = GetCallTip(word);
		if (calltip_text.length()) {
			SetCallTipShow(start_pos, calltip_text.data());
			CallTipHighlight();
			return true;
		}
	}
	return false;
}
bool CodeEditView::SearchPrevCallTip(int pos) {
	int start_pos= GetLineStartPos(GetLineFromPos(pos));
	if (start_pos < 0 || pos<= start_pos) {
		return false;
	}
	std::string textbuffer = GetTextRange(start_pos, pos);
	if (textbuffer.length() == 0) {
		return false;
	}
	SetCallTipCancel();
	const char* text = textbuffer.data();
	const char* s = text;
	int count = 0;
	int cur_dim_count = 0;
	int cur_space_count = 0;
	if (s) {
		int complevel = 0;
		bool brace = false;
		char ch_prev = 0;
		int funcpos = 0;
		for (char ch = 0; (ch = *s); ++s) {
			if (ch_prev != '\\' && (ch == '\"' || ch == '\'')) {
				brace = !brace;
				funcpos = 0;
			}
			if (!brace) {
				if (ch == '(') {
					complevel++;
					if (funcpos) {
						calltip_start_list.push(funcpos);
						funcpos = 0;
					}
				}
				else if (ch == ')') {
					complevel--;
					if (!calltip_start_list.empty()) {
						calltip_start_list.pop();
					}
					funcpos = 0;
				}
				else if (ch == ',') {
					cur_dim_count++;
					funcpos = 0;
				}
				else if (symbols.is_keyword(ch)) {
					if (funcpos == 0 || (!symbols.is_keyword(ch_prev) && !(ch_prev>='0' && ch_prev<='9' ) ) ) {
						funcpos = start_pos + count;
					}
				}
			}
			count++;
			ch_prev = ch;
		}
		if (!calltip_start_list.empty()) {
			calltip_start_pos =calltip_start_list.top();
			pos = calltip_start_pos- start_pos;
			count = 0;
			int maxlen = strlen(text);
			if (pos>=0 && pos<maxlen) {
				s = text + pos;
				for (char ch = 0; (ch = *s);++s) {
					if (!symbols.is_keyword(ch) && !( ch>='0' && ch<='9')) {
						break;
					}
					count++;
				}
				if (count) {
					std::string word = std::string(text + pos, count);
					calltip_text = GetCallTip(word);
					if (calltip_text.length()) {
						SetCallTipShow(calltip_start_pos, calltip_text.data());
						CallTipHighlight();
						return true;
					}
				}
				
			}
		}
	}
	return false;
}
/*
void CodeEditView::TimerSyncEditMouseClick(uint32_t pCodeEditView, uint32_t args2) {
	CodeEditView* cew = (CodeEditView*)pCodeEditView;
	if (cew) {
		PostMessageA(cew->_main,WM_SINGLE_CLICK_EDIT,0, args2);
		cew->_timer.Pause();
	}
}
*/
void CodeEditView::CallCtrlF() {
	int selpos = GetAnchor();
	int scroll_pos = GetFirstVisibleLine();
	int xoffset = GetXoffset();
	std::string selstr = GetSelText(100);
	if (!_findtext.isshow()) {
		_findtext.show(true, !_findtext.isshow() || (_findtext.isshow()
			&& _findtext.isfold()), true, (selstr.length()>0) ? selstr.data():nullptr);
	}
	else {
		if ((selstr.length() > 0)) {
			SetEditText(_findtext._findput, selstr.data());
		}
	}
	SendMessageA(_findtext._findput, EM_SETSEL, 0, -1);
	SetFocus(_findtext._findput);
	_search_pos = selpos;
	search_flag_update();
	SetFirstVisibleLine(scroll_pos);
	SetXoffset(xoffset);
}
void CodeEditView::CallCtrlH() {
	int selpos = GetAnchor();
	int scroll_pos = GetFirstVisibleLine();
	int xoffset = GetXoffset();
	std::string selstr = GetSelText(100);
	_findtext.show(true, false, true, (selstr.length() > 0) ? selstr.data() : nullptr);
	SendMessageA(_findtext._findput, EM_SETSEL, 0, -1);
	SetFocus(_findtext._findput);
	_search_pos = selpos;
	search_flag_update();
	SetFirstVisibleLine(scroll_pos);
	SetXoffset(xoffset);
}
BOOL CodeEditView::FindTextEvent(WORD ctrlid, WORD eventid) {

	if (eventid == BN_CLICKED ) {

		if (ctrlid == IDR_FINDTEXT_CLOSE) {
			_findtext.show(false, false);
			_search_id = 0;
			return TRUE;
		}
		if (ctrlid == IDR_FINDTEXT_FOLD) {
			_findtext.fold(!_findtext.isfold());
			return TRUE;
		}

		if (ctrlid == IDR_FINDTEXT_PREV) {
			search_prev();
			return TRUE;
		}
		else if (ctrlid == IDR_FINDTEXT_NEXT) {
			search_next();
			return TRUE;
		}
		else  if (ctrlid == IDR_FINDTEXT_ENABLECASE) {
			_findtext.setbuttonstate(IDR_FINDTEXT_ENABLECASE, !_findtext.getbuttonstate(IDR_FINDTEXT_ENABLECASE));
			if (_findtext.getbuttonstate(IDR_FINDTEXT_ENABLECASE)) {
				_search_flag |= SCFIND_MATCHCASE;
				search_flag_update();
				return TRUE;
			}
			else {
				_search_flag ^= SCFIND_MATCHCASE;
				search_flag_update();
				return FALSE;
			}
		}
		else if (ctrlid == IDR_FINDTEXT_FULLMATCH) {
			_findtext.setbuttonstate(IDR_FINDTEXT_FULLMATCH, !_findtext.getbuttonstate(IDR_FINDTEXT_FULLMATCH));
			if (_findtext.getbuttonstate(IDR_FINDTEXT_FULLMATCH)) {
				_search_flag |= SCFIND_WHOLEWORD;
				search_flag_update();
				return TRUE;
			}
			else {
				_search_flag ^= SCFIND_WHOLEWORD;
				search_flag_update();
				return FALSE;
			}
		}
		else if (ctrlid == IDR_FINDTEXT_REGEX) {
			_findtext.setbuttonstate(IDR_FINDTEXT_REGEX, !_findtext.getbuttonstate(IDR_FINDTEXT_REGEX));
			if (_findtext.getbuttonstate(IDR_FINDTEXT_REGEX)) {
				_search_flag |= SCFIND_REGEXP;
				_search_flag |= SCFIND_CXX11REGEX;
				search_flag_update();
				return TRUE;
			}
			else {
				_search_flag ^= SCFIND_REGEXP;
				_search_flag ^= SCFIND_CXX11REGEX;
				search_flag_update();
				return FALSE;
			}
		}
		else if (ctrlid == IDR_FINDTEXT_TATGETRANGE) {
			_findtext.setbuttonstate(IDR_FINDTEXT_TATGETRANGE, !_findtext.getbuttonstate(IDR_FINDTEXT_TATGETRANGE));
			_is_search_range = _findtext.getbuttonstate(IDR_FINDTEXT_TATGETRANGE);
			init_search_range();
			search_next();
			return _is_search_range;
		}
		else if (ctrlid == IDR_FINDTEXT_REP) {
			replace_text(true);
			return TRUE;
		}
		else if (ctrlid == IDR_FINDTEXT_REPALL) {
			replace_all(true);
			return TRUE;
		}
	}
	else if (eventid == EN_UPDATE) {
		if (ctrlid == IDR_FINDTEXT_PUT) {
			//g2u
			set_search_text(GetEditText(_findtext._findput, 0, IsEnableUTF8));
		}
		else if (ctrlid == IDR_FINDTEXT_REPPUT) {
			set_replace_text(GetEditText(_findtext._replaceput, 0, IsEnableUTF8));
		}
	}
	return TRUE;
}


void CodeEditView::ClickControl(int id) {
	switch (id)
	{
	case IDR_CODEEDIT_FIND_BUTTON:
		_findtext.show(!_findtext.isshow(), _findtext.isfold());
		break;
	case IDR_CODEEDIT_FUNCLIST_BUTTON:
		if (IsWindowVisible(FuncList->_main)) {
			FuncList->Show(SW_HIDE);
		}
		else {
			FuncList->Show(SW_SHOW);
		}
		break;
	case IDR_CODEEDIT_OPTIONS_BUTTON:
		if (_Options != NULL) {
			_Options->Show(!_Options->IsShow());
		}
		break;
	case IDR_CODEEDIT_FOLDALL_BUTTON:
		AutoFoldAnchorLines();
		break;
	case IDR_CODEEDIT_SYNTAXCHECK_BUTTON:
		double_click_time = GetDoubleClickTime();
		break;
	default:
		break;
	}
}

LRESULT CodeEditView::EditEvent(HWND hwnd, SCNotification* notify) {

	switch (notify->nmhdr.code)
	{
	case WM_LBUTTONDOWN:
		//if (enable_anchor_word_highlight) {
		//	if (IsWantAnchorForWordIndic()) {
		//		ShowAnchorForWordIndic();
		//		_timer.Update();
		//	}
		//	else {
		//		_timer.Pause();
		//	}
		//}
		break;
	case WM_LBUTTONUP:
		if (enable_anchor_word_highlight) {
			if (IsWantAnchorForWordIndic()) {
				ShowAnchorForWordIndic();
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (enable_anchor_word_highlight) {
			HideAnchorForWordIndic();
			//_timer.Pause();
		}
		break;
	case WM_RBUTTONDOWN:
		if (enable_anchor_word_highlight) {
			//_timer.Pause();
		}
		break;
	case SCN_CHARADDED:
	{
		if (enable_autofillspace) {
			if (notify->ch == '\n') {
				AutoFillSpace();
			}
		}
		if (enable_calltip) {
			if (notify->ch == '(') {
				ShowCallTip();
				break;
			}
			if (IsCallTipActive()) {
				if (notify->ch == ',') {
					CallTipHighlight();
					break;
				}
				else if (notify->ch == ')') {
					if (!ShowParentCallTip()) {
						SetCallTipCancel();
					}
				}
				break;
			}
		}
		if (enable_auto_complete) {
			if (symbols.is_keyword(notify->ch)) {
				ShowCompleteList();
				break;
			}
		}
	}
	break;
	case SCN_UPDATEUI:
	{
		//printf("ui更新:%d\n", notify->updated);
		if (notify->updated == SC_UPDATE_V_SCROLL) {
			if (enable_save_scroll_postion/* && _edit_once_change*/) {
				_edit_state = 1;
			}
		}
		//输入或删除到括号时需要更新。
		if (_edit_focus && (notify->updated & SC_UPDATE_CONTENT 
			|| notify->updated & SC_UPDATE_SELECTION)) {
			UpCurBrace();
			//if (enable_anchor_word_highlight && notify->updated == SC_UPDATE_SELECTION) {
			//	ShowAnchorForWordIndic();
			//	//printf("SC_UPDATE_SELECTION\n");
			//}
		}
	}
	break;
	case SCN_MODIFIED:
	{
		if (enable_calltip) {
			if ((notify->modificationType & SC_MOD_DELETETEXT) && notify->length == 1 && notify->text != nullptr) {
				if (IsCallTipActive()) {
					CallTipHighlight();
				}
				else if (*notify->text==',' || *notify->text == ')') {
					SearchPrevCallTip(notify->position);
				}

			}
		}
		return TRUE;
		/*if (notify->modificationType & SC_MOD_INSERTTEXT) {
			cew->_edit_state = cew->_edit_state | SC_MOD_INSERTTEXT;
		}
		else if (notify->modificationType & SC_MOD_CHANGEFOLD) {

			cew->_edit_state = cew->_edit_state | SC_MOD_CHANGEFOLD;
		}*/
	}
	break;
	case SCN_MARGINCLICK:
	{
		// 确定是页边点击事件
		const int line_number = GetLineFromPos(notify->position);
		const int lev = GetFoldLevel(line_number);
		//防止点击连接线折叠
		// (lev & SC_FOLDLEVELWHITEFLAG) 
		if (lev & SC_FOLDLEVELHEADERFLAG) {
			SendEditor(SCI_TOGGLEFOLD, line_number);
		}
		return TRUE;
	}
	break;
	case SCN_HOTSPOTCLICK: {
		std::string str = GetHotspotWord(notify->position);
		if (str.length()) {
			FuncList->Show(TRUE);
			FuncList->SetFindCurSel(str.data());
		}
	}
	break;
	case SCN_AUTOCCANCELLED:
		if (enable_auto_complete) {
			ResetAutoComplete();
		}
		break;
	case SCN_AUTOCCHARDELETED:
		if (enable_auto_complete) {
			ShowCompleteList();
		}
		break;
	default:
		break;
	}
	return FALSE;
}

LRESULT CodeEditView::TriggerCtrlEvent(HWND hwnd, int ctrlid, int eventid) {
	
	if (eventid == BN_CLICKED) {
		if (hwnd == _always_fold_all_checkbox) {
			EnableAlwaysFoldAllExt(GetCheck(hwnd));
		}
		else if (hwnd == _save_scroll_postion_checkbox) {
			EnableSaveSrcollPosExt(GetCheck(hwnd));
			if (enable_save_scroll_postion && _edit_once_change) {
				_edit_state = 1;
			}
		}
	}
	if (hwnd == _hwnd) {
		switch (eventid)
		{
		case SCEN_CHANGE: {
			//printf("文本已更改\n");
			_edit_state = 1;
			_edit_once_change = TRUE;
			if (enable_anchor_word_highlight) {
				HideAnchorForWordIndic();
			}
			if (_searching && !_replaceing) {
				if (!_search_want_update) {
					SetsSearchStringsClear();
				}
				_search_want_update=true;
			}
		}
		break;
		case SCEN_SETFOCUS:
			//printf("获得焦点\n");
			_edit_focus = TRUE;
			if (EditFocusCallback != NULL) {
				EditFocusCallback(_hwnd, TRUE);
			}
			break;
		case SCEN_KILLFOCUS:
			//printf("离开焦点\n");
			_edit_focus = FALSE;
			if (EditFocusCallback != NULL) {
				EditFocusCallback(_hwnd, FALSE);
			}
			break;
		}
	}
	return TRUE;
}
void __stdcall CodeEditView::MessageNotify(sptr_t ptr, SCNotification* notify) {
	int id = CodeEditView::classlist.getid((HWND)ptr);
	if (id < 0) {
		return;
	}
	CodeEditView* cew = CodeEditView::classlist.at(id).second;
	if (cew) {
		cew->EditEvent((HWND)ptr, notify);
	}
}
void __stdcall CodeEditView::MessageCommand(sptr_t ptr, int chidctrlid, int eventid) {
	int id = CodeEditView::classlist.getid((HWND)ptr);
	if (id<0) {
		return;
	}
	CodeEditView* cew = CodeEditView::classlist.at(id).second;
	if (cew) {
		cew->TriggerCtrlEvent((HWND)ptr, chidctrlid, eventid);
	}
}

LRESULT CALLBACK CodeEditView::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int id = CodeEditView::classlist.getid(hwnd);
	if (id < 0) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	CodeEditView* cew = CodeEditView::classlist.at(id).second;
	if (cew) {
		switch (message)
		{
		//case WM_SIZE:
		//{
		//	//不能在这更新,搜索窗口可能会丢失。
		//	//cew->_findtext.uppos();
		//}
		case WM_PAINT:
		{
			PAINTSTRUCT pt;
			HDC hdc = BeginPaint(cew->_main, &pt);
			RECT rt = pt.rcPaint;
			GetClientRect(cew->_main, &rt);

			int i = 0;
			for (control_t* ctrl = cew->controls.at(i); ctrl;) {
				DrawButton(cew->_main, hdc, ctrl);
				i++;
				ctrl = cew->controls.at(i);
			}

			EndPaint(cew->_main, &pt);
			return TRUE;
		}
		case WM_NOTIFY:
		{
			switch (wParam)
			{
			case IDR_CODEEDIT_EDIT:
				return cew->EditEvent(hwnd, (SCNotification*)lParam);
			default:
				break;
			}
			return 0;
		}
		break;
		case WM_COMMAND:
		{
			cew->TriggerCtrlEvent((HWND)lParam, (int)LOWORD(wParam), (int)HIWORD(wParam));
			if ((HWND)lParam == cew->_hwnd) {
				return 0;
			}
		}
		break;

		case WM_CTLCOLORSTATIC:
			//SetBkMode((HDC)wParam, TRANSPARENT);
			SetBkColor((HDC)wParam, GetSysColor(COLOR_MENU));
			return (INT_PTR)(HBRUSH)GetStockObject(NULL_BRUSH); //背景画刷
			break;
		case WM_MOUSEMOVE:
		{
			POINT pos; GetCursorPos(&pos);
			ScreenToClient(cew->_main, &pos);
			cew->controls.focusupdate(pos.x, pos.y);
		}
		break;
		case WM_LBUTTONDOWN:
			if (control_t::focus) {
				control_t::focus->isdown = TRUE;
				control_t::lastdownfocus = control_t::focus;
				DrawButton(control_t::focus);
				
			}

			break;
		case WM_LBUTTONUP:
			if (control_t::focus) {
				control_t* lastfocus = control_t::lastdownfocus;
				control_t::lastdownfocus = NULL;
				control_t::focus->isdown = FALSE;
				DrawButton(control_t::focus);
				if(control_t::focus == lastfocus) {
					cew->ClickControl(control_t::focus->id);
				}
			}
			break;
		case WM_USER_CONTROL_UP:
		{
			control_t* ctrl = cew->controls.get(wParam);
			if (ctrl) {
				DrawButton(ctrl);
			}
			return TRUE;
		}
		break;
		//case WM_SINGLE_CLICK_EDIT:
		//	if (cew->enable_anchor_word_highlight) {
		//		cew->ShowAnchorForWordIndic();
		//	}
		break;
		}
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);

}

void CodeEditView::Resize() {
	if (!_parent) { return; }
	RECT parent_rect;
	GetClientRect(_parent, &parent_rect);
	MoveWindow(_main, 0, 0, parent_rect.right, parent_rect.bottom, TRUE);
	MoveWindow(_hwnd, 0, ToolbarHeight, parent_rect.right, parent_rect.bottom - ToolbarHeight, TRUE);
	_findtext.uppos();
}
void CodeEditView::MovePos(int x, int y) {
	SetWindowPos(_main, NULL, x, y, 0, 0, SWP_NOSIZE);
	SetWindowPos(_hwnd, NULL, 2, ToolbarHeight, 0, 0, SWP_NOSIZE);
	SetCallTipUpdate();
	AutoCompleteUpdate();
	_findtext.uppos();
}
void CodeEditView::SetSize(int w, int h) {
	SetWindowPos(_main, NULL, 0, 0, w, h, SWP_NOMOVE);
	SetWindowPos(_hwnd, NULL, 0, 0, w - 0, h - ToolbarHeight, SWP_NOMOVE);
	_findtext.uppos();
}

BOOL CodeEditView::Init(HWND parent, Options* Options) {
	ToolbarHeight = 39;

	if (_parent)
	{
		return TRUE;
	}

	_parent = parent;

	_Options = Options;

	RECT parent_rect;
	GetClientRect(parent, &parent_rect);


	WinClass.Register("CodeEditWin",WndProc);

	_main = CreateWindowA("CodeEditWin", NULL, WS_CHILD | WS_EX_WINDOWEDGE | WS_VISIBLE,
		0, 0, parent_rect.right, parent_rect.bottom, parent, (HMENU)0, nullptr, nullptr);

	//WS_EX_CLIENTEDGE 内边框
	BOOL rt=InitEditBox(WS_EX_CLIENTEDGE,WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, ToolbarHeight, parent_rect.right, parent_rect.bottom - ToolbarHeight, _main, (HMENU)IDR_CODEEDIT_EDIT, nullptr);
	
	if (!rt)
	{
		CHAR error[260] = { 0 };
		wsprintfA(error, "tesh error:%d", GetLastError());
		MessageBoxA(parent, error, "提示", MB_OK);
		return FALSE;
	}
	_findtext.Init(_main,0,0);
	_findtext.setpos(parent_rect.right - _findtext._width - 24, _findtext._height-15);

	//创建按钮控件
	int btn_offset = 10;
	controls.add(control_t("搜索/替换", _main, IDR_CODEEDIT_FIND_BUTTON,{ btn_offset ,3,90,30}));
	btn_offset = btn_offset + 110;
	controls.add(control_t("函数列表", _main, IDR_CODEEDIT_FUNCLIST_BUTTON,  { btn_offset ,3,90,30 }));
	btn_offset = btn_offset + 110;
	controls.add(control_t("选项", _main, IDR_CODEEDIT_OPTIONS_BUTTON,  { btn_offset ,3,90,30 }));
	btn_offset = btn_offset + 110;
	controls.add(control_t("折叠/展开", _main, IDR_CODEEDIT_FOLDALL_BUTTON, { btn_offset ,3,90,30 }));
	btn_offset = btn_offset + 110;
	controls.add(control_t("语法检查", _main, IDR_CODEEDIT_SYNTAXCHECK_BUTTON, { btn_offset ,3,90,30 }));
	btn_offset = btn_offset + 120;
	_always_fold_all_checkbox = CreateButton(_main, IDR_CODEEDIT_CHID_CTRL,
		WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE, "Always fold all", "Tahoma", 16, btn_offset, 3, 150, 15);
	_save_scroll_postion_checkbox = CreateButton(_main, IDR_CODEEDIT_CHID_CTRL,
		WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE, "Save scroll postion", "Tahoma", 16, btn_offset, 22, 150, 15);

	classlist.add(_main,this);
	classlist.add(_hwnd, this);

	SetCallbackMessageCommand(MessageCommand);
	SetCallbackMessageNotify(MessageNotify);

	SetLexer(SCLEX_JASS); //语法解析
	(IsEnableUTF8) ? SetCodePage(SC_CP_UTF8) :SetCodePage(SC_CP_GBK);

	SetFontQuality(SC_EFF_QUALITY_LCD_OPTIMIZED);
	//默认文本
	SetDefaultStyles("Courier New",10, RGB(0x00, 0x00, 0x00), RGB(0xff, 0xff, 0xff));
	// 显示行号
	line_num_width = 50;
	EnableLineNumber(0, line_num_width);

	EnableFold(1,14,true,true);
	SetMarkerFore(RGB(0, 255, 255));
	SetMarkerback1(RGB(0,0,255));
	SetMarkerback2(RGB(0, 255, 255));
	
	
	//设置TAB 宽度
	SetTabWidth(4);
	SetZoomLevelMinMax(-5,10);

	enable_auto_complete=true;
	enable_calltip=true;

	SetAutoComplete(320, 10);
	
	SetBraceStyle("Courier New",true,RGB(0, 0, 0), RGB(255, 0, 0),FALSE);

	SetGuideStyle(3, RGB(204, 204, 204), -1);
	//SetFont(STYLE_CONTROLCHAR, "Courier New", true);
	//SendEditor(SCI_SETVIEWWS, 255);
	//SendEditor(SCI_STYLESETVISIBLE, STYLE_CONTROLCHAR,	0);
	//for (int i = 0; i <=7; i++) {
	//	SetIndicStyle(i, INDIC_PLAIN);
	//	SendEditor(SCI_INDICSETFORE, i, RGB(51, 153, 255));
	//	SendEditor(SCI_INDICSETALPHA, i,50);
	//	SendEditor(SCI_INDICSETHOVERSTYLE, i, INDIC_PLAIN);
	//	SendEditor(SCI_INDICSETHOVERFORE, i, RGB(51, 153, 255));
	//}
	//选择范围
	SetSearchRangeIndic(1, INDIC_FULLBOX, RGB(102, 102, 102),50,50);

	SetAnchorForWordIndic(2, INDIC_ROUNDBOX, RGB(255, 100, 0), 50, 50);
	enable_anchor_word_highlight=true;

	//选中的颜色
	//SetCurrentLineStyle(TRUE,RGB(255,255,102),150);
	SetSelectStyle(TRUE, RGB(255, 255, 255), RGB(51, 153, 255), SC_ALPHA_NOALPHA);
	//SetAdditionalSelectStyle(RGB(0, 0, 0), RGB(204, 204, 204), SC_ALPHA_NOALPHA);
	//EnableHotspot(TRUE);
	//SetPosBraceBold(TRUE);
	for (int i = SCE_JASS_DEFAULT; i <= SCE_JASS_NMAX; i++) {
		SetFont(i, "Courier New", true);
	}
	
	SetFontBold(SCE_JASS_WORD, TRUE);
	SetFontBold(SCE_JASS_WORD2, TRUE);
	SetForeColor(SCE_JASS_WORD3, RGB(0, 0, 170));//SetFontBold(SCE_JASS_WORD3, TRUE); 
	SetFontBold(SCE_JASS_WORD4, TRUE); SetForeColor(SCE_JASS_WORD4, RGB(0, 85, 170));
	
	SetForeColor(SCE_JASS_WORD5, RGB(0, 58, 213));
	SetForeColor(SCE_JASS_WORD6, RGB(173, 58, 128));
	SetForeColor(SCE_JASS_WORD7, RGB(255, 0, 58));
	

	SetFontItalic(SCE_JASS_COMMENT, TRUE); SetForeColor(SCE_JASS_COMMENT, RGB(0, 136, 0));
	SetFontItalic(SCE_JASS_COMMENTLINE, TRUE); SetForeColor(SCE_JASS_COMMENTLINE, RGB(0, 136, 0));
	SetFontItalic(SCE_JASS_COMMENTFLAG, TRUE); SetForeColor(SCE_JASS_COMMENTFLAG, RGB(0, 136, 0));
	SetForeColor(SCE_JASS_NUMBER, RGB(0, 0, 170));
	
	SetFontItalic(SCE_JASS_STRING, TRUE); SetForeColor(SCE_JASS_STRING, RGB(0, 58, 213));
	SetFontBold(SCE_JASS_CHARACTER, TRUE); SetFontUnderline(SCE_JASS_CHARACTER, TRUE); //SetForeColor(SCE_JASS_CHARACTER, RGB(173, 58, 128));
	SetFontBold(SCE_JASS_PREPROCESSOR, TRUE);// SetForeColor(SCE_JASS_PREPROCESSOR, RGB(173, 58, 128));
	SetFontBold(SCE_JASS_OPERATOR, TRUE); SetForeColor(SCE_JASS_OPERATOR, RGB(170, 85, 0));
	//SetForeColor(SCE_JASS_IDENTIFIER, RGB(0, 0, 0));
	SetForeColor(SCE_JASS_RUNTEXTMACRO, RGB(153, 153, 153));

	SetForeColor(SCE_LUA_COMMENT, RGB(0, 136, 0));
	SetForeColor(SCE_LUA_COMMENTLINE, RGB(0, 136, 0));
	SetForeColor(SCE_LUA_COMMENTDOC, RGB(0, 136, 0));
	SetForeColor(SCE_LUA_NUMBER, RGB(0, 0, 170));
	SetFontBold(SCE_LUA_WORD, TRUE); //SetForeColor(SCE_LUA_WORD, RGB(197,134, 161));
	SetForeColor(SCE_LUA_STRING, RGB(0, 58, 213));
	//SetFontBold(SCE_LUA_CHARACTER, TRUE);// SetForeColor(SCE_LUA_CHARACTER, RGB(0, 58, 213));
	SetForeColor(SCE_LUA_LITERALSTRING, RGB(0, 58, 213));
	//SetForeColor(SCE_LUA_PREPROCESSOR, RGB(0, 58, 213));
	SetFontBold(SCE_LUA_OPERATOR, TRUE); SetForeColor(SCE_LUA_OPERATOR, RGB(170, 85, 0));
	SetFontBold(SCE_LUA_IMPORT_FALG, TRUE); SetForeColor(SCE_LUA_IMPORT_FALG, RGB(170, 85, 0));//[[]] style = SCE_LUA_OPERATOR

	SetForeColor(SCE_LUA_NAME, RGB(255, 0, 0));
	SetForeColor(SCE_LUA_KEY, RGB(0, 255, 0));

	//SetForeColor(SCE_LUA_IDENTIFIER, RGB(255, 153, 0));
	//SetForeColor(SCE_LUA_LABEL, RGB(0, 58, 213));
	//SetForeColor(SCE_LUA_IMPORT_FALG, RGB(0, 58, 213));

	SetForeColor(SCE_LUA_WORD5, RGB(0, 58, 213));
	SetForeColor(SCE_LUA_WORD6, RGB(173, 58, 128));
	SetForeColor(SCE_LUA_WORD7, RGB(255, 0, 58));
	

	SetKeywords(1, LuaKeywords);
	SetKeywords(0, JassKeywords);
	SetKeywords(2, JassKeywords2);
	SetKeywords(3, JassKeywords3);
	SetKeywords(4, JassTypes);

	//_timer.Start(double_click_time, 0, &TimerSyncEditMouseClick, (uint32_t)this, 0,true);

	return TRUE;
}

BOOL CodeEditView::Destroy() {
	DestroyWindow(_hwnd);
	DestroyWindow(_main);
	_parent = NULL;
	_main = NULL;
	_hwnd = NULL;
	_edit_state = 0;
	EditFocusCallback = NULL;
	return TRUE;
}