#include <dll_info.h>
#include <Splitter.h>

BaseMap<HWND, splitter_t*> splitter_t::classlist;

LRESULT CALLBACK splitter_t::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
		splitter_t* splitter = GetSplitter(hwnd);
		/*if (splitter==NULL && flw!=NULL) {
			splitter = &(flw->splitter);
		}*/
		if (splitter != NULL) {
			switch (message)
			{
			case WM_MOUSEMOVE:
				splitter->enter();
				splitter->mousemove();
				break;
				//return TRUE;
			case WM_LBUTTONDOWN:
				splitter->mousedown();
				break;
				//return TRUE;
			case WM_LBUTTONUP:
				splitter->mouseup();
				break;
				//return TRUE;
			}
		}
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);

}
splitter_t::splitter_t() {

	_parent = 0;
	_hwnd = 0;
	_cursor = NULL;
	_movetype = horizontal;
	_margins = { 0,0,0,0 };

	moveing = FALSE;
	hdc = NULL;
	brush = NULL;
	pen = NULL;
	hmemdc = NULL;
	can_uppoint = FALSE;
	saved_point = { 0,0 };
	WinClass.Register("c_splitter",WndProc);
}
splitter_t::~splitter_t() {
	if (hmemdc != NULL)
		DeleteDC(hmemdc);
	//if (brush != NULL)
	//	DeleteObject(brush);
	//if (pen != NULL)
	//	DeleteObject(pen);
	_parent = 0;
	_hwnd = 0;
	_cursor = NULL;
	_movetype = horizontal;
	_margins = { 0,0,0,0 };

	moveing = FALSE;
	hdc = NULL;
	brush = NULL;
	pen = NULL;
	hmemdc = NULL;
	can_uppoint = FALSE;
	saved_point = { 0,0 };
	WinClass.UnRegister("c_splitter");
}

void splitter_t::enter() {
	if (moveing) {
		return;
	}
	SetCursor(_cursor);
	//debug("进入splitter焦点\n");
}
void splitter_t::leave() {
	//无效！
	//if (moveing || !focus) {
	//	return;
	//}
	//focus = FALSE;
	//debug("离开splitter焦点\n");
}
void splitter_t::mousemove() {
	if (!_hwnd || !moveing) {
		return;
	}
	//每一步都是必要的。
	POINT pos;
	static RECT rt = { 0,0,0,0 };
	//static RECT rt2= { 0,0,0,0 };
	int w;
	//InvalidateRect(_parent, &rt, FALSE);
	//UpdateWindow(_parent);
	RedrawWindow(_parent, &rt, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
	GetCursorPos(&pos);
	ScreenToClient(_parent, &pos);
	GetClientRect(_parent, &rt);
	if ((pos.x < _margins.left) || (pos.x > rt.right - _margins.right) || (pos.y < _margins.top) || (pos.y > rt.bottom - _margins.bottom)) {
		return;
	}

	GetClientRect(_hwnd, &rt);
	if (_movetype == horizontal) {
		w = rt.right - rt.left;
		rt = { pos.x - w / 2 - 1, rt.top - 2, pos.x + w + 2, rt.bottom + 2 };
		Rectangle(hdc, pos.x - w / 2 + 1, rt.top, pos.x + w, rt.bottom);
		/*	rt2 = { pos.x - w / 2 + 1, rt.top, pos.x + w, rt.bottom };
			FillRect(hdc, &rt, brush);*/
	}
	else {
		w = rt.bottom - rt.top;
		rt = { pos.x - w / 2 - 1, rt.top - 2, pos.x + w + 2, rt.bottom + 2 };
		Rectangle(hdc, rt.left, pos.y - w / 2 + 1, rt.right, pos.y + w);
		/*	rt2 = { rt.left, pos.y - w / 2 + 1, rt.right, pos.y + w };
			FillRect(hdc, &rt, brush);*/
	}

}
void splitter_t::mousedown() {
	if (!_hwnd || moveing) {
		return;
	}
	POINT pos;
	RECT rt;
	int w;
	moveing = TRUE;

	SetCapture(_hwnd);
	SetCursor(_cursor);

	GetCursorPos(&pos);
	ScreenToClient(_parent, &pos);
	saved_point = { pos.x,pos.y };

	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, brush);        //选择画刷
	SelectObject(hdc, pen);        //选择画笔
	GetClientRect(_hwnd, &rt);
	if (_movetype == horizontal) {
		w = rt.right - rt.left;
		Rectangle(hdc, pos.x - w / 2 + 1, rt.top, pos.x + w, rt.bottom);
		//rt = { pos.x - w / 2 + 1, rt.top, pos.x + w, rt.bottom };
		//FillRect(hdc, &rt, brush);
	}
	else {
		w = rt.bottom - rt.top;
		Rectangle(hdc, rt.left, pos.y - w / 2 + 1, rt.right, pos.y + w);
		/*rt = { rt.left, pos.y - w / 2 + 1, rt.right, pos.y + w };
		FillRect(hdc,&rt,brush);*/
	}
	can_uppoint = FALSE;
}
BOOL splitter_t::mouseup() {
	if (!_hwnd || !moveing) {
		return FALSE;
	}
	POINT pos;
	RECT rt;
	GetCursorPos(&pos);
	ScreenToClient(_parent, &pos);
	GetClientRect(_parent, &rt);
	pos.x = max(_margins.left, min(pos.x, rt.right - _margins.right));
	pos.y = max(_margins.top, min(pos.y, rt.bottom - _margins.bottom));
	GetClientRect(_hwnd, &rt);
	ReleaseCapture();
	BOOL retval;
	if (_movetype == horizontal) {
		retval = (saved_point.x != pos.x);
		pos.x = (pos.x - (rt.right - rt.left) / 2) + 1;
		//SetWindowPos(_hwnd, NULL, pos.x,rt.top,0,0 ,SWP_NOSIZE);
	}
	else {
		retval = (saved_point.y != pos.y);
		pos.y = (pos.y - (rt.bottom - rt.top) / 2) + 1;
		//SetWindowPos(_hwnd, NULL,rt.left, pos.y,0,0 ,SWP_NOSIZE);
	}
	saved_point = { pos.x,pos.y };
	InvalidateRect(_parent, NULL, FALSE);
	UpdateWindow(_parent);
	moveing = FALSE;
	can_uppoint = TRUE;
	if (retval && callback) {
		callback(_parent, _hwnd);
	}
	return retval;
}
BOOL splitter_t::uppoint() {
	if (!can_uppoint) { return FALSE; }
	can_uppoint = FALSE;
	RECT rt;
	GetClientRect(_hwnd, &rt);
	if (_movetype == horizontal) {
		SetWindowPos(_hwnd, NULL, saved_point.x, rt.top, 0, 0, SWP_NOSIZE);
	}
	else {
		SetWindowPos(_hwnd, NULL, rt.left, saved_point.y, 0, 0, SWP_NOSIZE);
	}
	return TRUE;
}

BOOL splitter_t::init(HWND parent, RECT rect, LPSTR cursor, int id, move_type movetype, RECT margins, splitter_callback callback_proc) {
	if (_hwnd) { return TRUE; }
	_parent = parent;
	_hwnd = CreateWindowA("c_splitter", NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		rect.left, rect.top, rect.right, rect.bottom, parent, (HMENU)id, nullptr, nullptr);
	_cursor = LoadCursorA(NULL, cursor);
	_movetype = movetype;
	classlist.add(_hwnd ,this);
	_margins = { margins.left,margins.top,margins.right,margins.bottom };
	callback = callback_proc;

	moveing = FALSE;
	hdc = GetDC(parent);
	hmemdc = CreateCompatibleDC(GetDC(parent));
	brush = CreateSolidBrush(RGB(153, 153, 153));// (HBRUSH)GetStockObject(NULL_BRUSH);//(HBRUSH)GetClassLong(parent, GCL_HBRBACKGROUND);
	if (movetype == horizontal) {
		pen = CreatePen(PS_SOLID, rect.right, RGB(153, 153, 153));//(HPEN)GetStockObject(NULL_PEN);
	}
	else {
		pen = CreatePen(PS_SOLID, rect.bottom, RGB(153, 153, 153));//(HPEN)GetStockObject(NULL_PEN);
	}
	return TRUE;
}


splitter_t* GetSplitter(HWND hwnd) {
	return splitter_t::classlist.get(hwnd);
}
