#pragma once
#include <windows.h>
#include <stdint.h>
#include <map>
#include <CtrlFunc.h>
typedef void(__cdecl *splitter_callback)(HWND, HWND);


class splitter_t {
private:
	POINT saved_point;
	BOOL can_uppoint;
public:
	enum move_type {
		horizontal = 0,
		perpendicular,
	};
	HWND _parent;
	HWND _hwnd;
	HCURSOR _cursor;
	move_type _movetype;
	RECT _margins;

	//×´Ì¬
	BOOL moveing;
	HDC hdc;
	HDC hmemdc;
	HBRUSH brush; //»­Ë¢
	HPEN pen;

	splitter_callback callback;

	splitter_t();
	~splitter_t();
	void enter();
	void leave();
	void mousemove();
	void mousedown();
	BOOL mouseup();
	BOOL uppoint();
	BOOL init(HWND parent, RECT rect, LPSTR cursor, int id, move_type movetype, RECT margins, splitter_callback callback_proc);

	static BaseMap<HWND, splitter_t*> classlist;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

splitter_t* GetSplitter(HWND hwnd);