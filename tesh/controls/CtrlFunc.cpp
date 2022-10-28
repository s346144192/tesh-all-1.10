#include <CtrlFunc.h>
#include <base/encode/convert_utf_8.h>
#include <Commctrl.h>
//ͨ�öԻ���:�ļ������塢��ɫ (��������)
#include <Commdlg.h> 
#pragma comment (lib, "Comdlg32.lib")
//BOOL GetOpenFileNameA([in, out] LPOPENFILENAMEA unnamedParam1);
//BOOL GetSaveFileNameA([in, out] LPOPENFILENAMEA unnamedParam1);
//BOOL WINAPI ChooseFont(_Inout_ LPCHOOSEFONT lpcf); CHOOSEFONT cf.nFontType = SCREEN_FONTTYPE;
//BOOL WINAPI ChooseColor(_Inout_ LPCHOOSECOLOR lpcc);

uint32_t double_click_time = GetDoubleClickTime();

control_t* control_t::focus = NULL;
control_t* control_t::lastdownfocus = NULL;



int GetLogPixelsY() {
	static int logpixel = 0;
	if (logpixel !=0) {
		return logpixel;
	}
	HDC hDC = GetDC(HWND_DESKTOP);
	logpixel = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(HWND_DESKTOP, hDC);
	return logpixel;
}
float GetLogPixelsScale() {
	static float scale = ((float)GetLogPixelsY() / USER_DEFAULT_SCREEN_DPI);
	return scale;
}
int ToFontSize(int height, bool scale_size) {
	if (scale_size) {
		height = ((float)abs(height)*GetLogPixelsScale() + 0.499);
	}
	return abs(::MulDiv(height/ max(0.1,GetLogPixelsScale())+0.490, 72, GetLogPixelsY()));
}
int ToFontHeight(int size, bool scale_size) {
	if (size < 1) {
		return 0;
	}
	int height= (float)MulDiv(size, GetLogPixelsY(), 72)* GetLogPixelsScale();
	if (scale_size) {
		height = ((float)height*GetLogPixelsScale() + 0.499);
	}
	return height;
}
BOOL GetClientRectExt(HWND hwnd, LPRECT lpRect) {
	if (lpRect == NULL) {
		return FALSE;
	}
	HWND parent = GetParent(hwnd);
	if (parent == NULL) {
		GetClientRect(hwnd, lpRect);
		return TRUE;
	}

	GetWindowRect(hwnd, lpRect);
	POINT point = { lpRect->left,lpRect->top };
	ScreenToClient(parent, &point);
	lpRect->left = point.x; lpRect->top = point.y;
	point = { lpRect->right,lpRect->bottom };
	ScreenToClient(parent, &point);
	lpRect->right = point.x; lpRect->bottom = point.y;
	return TRUE;
}

BOOL MoveWindowExt(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
	if (!hwnd) { return FALSE; }
	RECT rect1;
	if (!GetClientRectExt(hwnd, &rect1)) {
		return FALSE;
	}
	RECT rect;
	rect.left = (X >= 0) ? X : rect1.left;
	rect.top = (Y >= 0) ? Y : rect1.top;
	rect.right = (nWidth >= 0) ? nWidth : rect1.right - rect1.left;
	rect.bottom = (nHeight >= 0) ? nHeight : rect1.bottom - rect1.top;
	return MoveWindow(hwnd, rect.left, rect.top, rect.right, rect.bottom, bRepaint);
}
HFONT GetButtonFont() {
	static HFONT font = NULL;
	if (font) { return font; }
	LOGFONTA lf = { };
	strcpy_s(lf.lfFaceName, "Tahoma");
	lf.lfHeight = 18; lf.lfWeight = 400; lf.lfCharSet = DEFAULT_CHARSET; lf.lfQuality = DEFAULT_QUALITY;
	lf.lfOutPrecision = OUT_CHARACTER_PRECIS; lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	font = CreateFontIndirectA(&lf);
	return font;
}
HFONT CreateNewFont(LPCSTR fontname, int heigth, int Bold , BOOL bItalic ,BOOL bUnderline, BOOL bStrikeOut) {
	return CreateFont(
		heigth/*�߶�*/, 0/*���*/, 0/*���ù�*/, 0/*���ù�*/, Bold /*һ�����ֵ��Ϊ400*/,
		bItalic/*����б��*/, bUnderline/*�����»���*/, bStrikeOut/*����ɾ����*/,
		DEFAULT_CHARSET,  //����ʹ��Ĭ���ַ��������������� _CHARSET ��β�ĳ�������
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,  //���в������ù�
		DEFAULT_QUALITY,  //Ĭ���������
		FF_DONTCARE,  //��ָ��������*/
		fontname  //������
	);
}

void SetCtrlFont(HWND w, LPCSTR fontname, int width, int heigth, BOOL Bold, BOOL bItalic,
	BOOL bUnderline, BOOL bStrikeOut) {
	if (!w) { return; }
	//�����߼�����
	//ʹ��ʱɾ��hFont object��ָ�Ĭ�����塣
	HFONT hFont = CreateFontA(
		heigth/*�߶�*/, width/*���*/, 0/*���ù�*/, 0/*���ù�*/, Bold /*һ�����ֵ��Ϊ400*/,
		bItalic/*����б��*/, bUnderline/*�����»���*/, bStrikeOut/*����ɾ����*/,
		DEFAULT_CHARSET,  //����ʹ��Ĭ���ַ��������������� _CHARSET ��β�ĳ�������
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,  //���в������ù�
		DEFAULT_QUALITY,  //Ĭ���������
		FF_DONTCARE,  //��ָ��������*/
		fontname  //������
	);
	SendMessageA(w, WM_SETFONT, (WPARAM)hFont, NULL); //���ð�ť����
}
int GetLogFont(HFONT hFont, LOGFONT* lf) {
	ZeroMemory(lf,sizeof(LOGFONT));
	return GetObjectA(hFont, sizeof(LOGFONT), lf);
}
int GetCtrlLogFont(HWND hwnd,LOGFONT* lf) {
	HFONT hFont=(HFONT)SendMessageA(hwnd, WM_GETFONT, NULL, NULL);
	if (!hFont) {
		return 0;
	}
	return GetLogFont(hFont,lf);
}
HWND CreateButton(HWND parent, uint32_t id, DWORD dwStyle, LPCSTR name, LPCSTR fontname,int fontsize, int x, int y, int width, int heigth) {
	HWND hBtn = CreateWindowA(
		"Button", //��ť�ؼ�������
		name,
		dwStyle ,
		x /*X����*/, y /*Y����*/, width /*���*/, heigth/*�߶�*/,
		parent, (HMENU)id /*�ؼ�Ψһ��ʶ��*/, nullptr, NULL
	);
	SetCtrlFont(hBtn, fontname, 0, fontsize); //���ð�ť����
	return hBtn;
}
int GetEditTextLength(HWND hwnd){
	return SendMessageA(hwnd, WM_GETTEXTLENGTH, 0, 0);
}
std::string GetEditText(HWND hwnd,int maxlen, bool UseUTF8) {
	if (maxlen<=0) {
		maxlen= SendMessageA(hwnd, WM_GETTEXTLENGTH, 0, 0);
	}
	char* tmp=(char*)malloc(maxlen+2);
	ZeroMemory(tmp, maxlen + 2);
	SendMessageA(hwnd, WM_GETTEXT, (WPARAM)maxlen+1, (LPARAM)tmp);
	std::string str( (UseUTF8) ? base::a2u(tmp) : tmp );
	free(tmp);
	return str;
}
void SetEditText(HWND hwnd, std::string str) {
	SendMessageA(hwnd, WM_SETTEXT, 0, (LPARAM)str.data());
}
BOOL GetCheck(HWND hwnd) {
	return SendMessageA(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}
void SetCheck(HWND hwnd, BOOL enable) {
	SendMessageA(hwnd, BM_SETCHECK, enable, 0);
}
void ShowHwnd(HWND hwnd, bool enable) {
	if (enable) {
		ShowWindow(hwnd, SW_SHOW);
	}
	else {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		//ShowWindow(hwnd, SW_HIDE);
	}
}
void ShowTopHwnd(HWND hwnd, bool enable) {
	if (enable) {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	}
	else {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	}
}
void EnableHwnd(HWND hwnd, BOOL enable) {
	int style = GetWindowLongA(hwnd, GWL_STYLE);
	if (enable) {
		if (style & WS_DISABLED) {
			SetWindowLongA(hwnd, GWL_STYLE, style^WS_DISABLED);
		}
	}
	else {
		if (!(style & WS_DISABLED)) {
			SetWindowLongA(hwnd, GWL_STYLE, style | WS_DISABLED);
		}
	}
}
HWND GetChildCtrl(HWND hwnd, int id) {
	HWND hwndChild = ::GetWindow(hwnd, GW_CHILD); //�г����пؼ�
	while (hwndChild)
	{
		if (::GetDlgCtrlID(hwndChild)==id) {
			break;
		}
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	return hwndChild;
}
//cf.Flags = CF_EFFECTS | CF_SCALABLEONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT | CF_USESTYLE;
//CF_INITTOLOGFONTSTRUCT ��ҪLOGFONT
BOOL ShowFontDialog(HWND hwnd, DWORD Flags , LOGFONTA* lf, LPCFHOOKPROC lpfnHook, CHOOSEFONTA** pcf, LPARAM userdata) {
	static char elfStyle[LF_FACESIZE];
	ZeroMemory(elfStyle, sizeof(elfStyle));
	CHOOSEFONTA cf = { sizeof(CHOOSEFONTA) };
	cf.Flags = Flags;
	cf.nFontType = SCREEN_FONTTYPE;
	cf.hwndOwner = hwnd;
	cf.lpfnHook = lpfnHook;
	cf.lCustData = userdata;
	cf.lpLogFont = lf;
	cf.lpszStyle = elfStyle;
	if (pcf) {
		*pcf = &cf;
	}
	strcpy_s(elfStyle,sizeof(elfStyle) ,"����");
	//�Ǳߵ����嶼����������,�ҿؼ��������úͻ�ö���Ҫ�������塣����Ի�����⴦��
	if (lf) {
		lf->lfHeight = ((float)lf->lfHeight/max(0.1,GetLogPixelsScale())+0.499);
		if (lf->lfWeight > FW_NORMAL) {
			strcpy_s(elfStyle, sizeof(elfStyle), "����");
		}
		if (lf->lfItalic) {
			if (lf->lfWeight> FW_NORMAL) {
				strcpy_s(elfStyle, sizeof(elfStyle), "��б��");
			}
			else {
				strcpy_s(elfStyle, sizeof(elfStyle), "б��");
			}
		}
	}
	BOOL ret= ChooseFontA(&cf);
	if (lf) {
		lf->lfHeight = (LONG)((float)abs(lf->lfHeight)*GetLogPixelsScale()+0.499);
	}
	return ret;
}
//CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT
BOOL ShowColorDialog(HWND hwnd, DWORD Flags, COLORREF* lpc, LPCCHOOKPROC lpfnHook,  LPARAM userdata) {
	static COLORREF colorlist[16] = {0}; //�Զ�����ɫ��ĺ졢�̡���(RGB) ֵ�� 16 ��ֵ������
	CHOOSECOLORA cc = {};
	cc.lStructSize = (sizeof(CHOOSECOLORA));
	cc.lpCustColors = colorlist;
	cc.Flags = Flags;
	cc.hwndOwner = hwnd;
	cc.lpfnHook = lpfnHook;
	cc.lCustData = userdata;
	if (lpc) {
		cc.rgbResult = *lpc;
		BOOL ret = ChooseColorA(&cc);
		*lpc = cc.rgbResult;
		return ret;
	}
	return ChooseColorA(&cc);
}
void RemoveButtonDefStyle(HWND hwnd) {
	SetWindowLongA(hwnd, GWL_STYLE, GetWindowLongA(hwnd, GWL_STYLE) ^ BS_DEFPUSHBUTTON);
}

HWND CreateUpDnBuddy(HWND parent, HMENU hmenu, int x,int y,int width,int height)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_STANDARD_CLASSES;   
	InitCommonControlsEx(&icex);    //ÿ�ζ���Ҫ����  

	HWND _hwnd = CreateWindowExA(WS_EX_LEFT | WS_EX_CLIENTEDGE | WS_EX_CONTEXTHELP,  
		WC_EDITA,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER    
		| ES_NUMBER | ES_LEFT,                    
		x,y,
		width, height,
		parent,
		hmenu,
		dllself._module,
		NULL);

	return _hwnd;
}

HWND CreateUpDnCtl(HWND parent, HMENU hmenu, int min,int max)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_UPDOWN_CLASS;  
	InitCommonControlsEx(&icex);   
	//UDS_AUTOBUDDY �Զ�
	HWND _hwnd = CreateWindowExA(WS_EX_LEFT | WS_EX_LTRREADING,
		UPDOWN_CLASSA,
		NULL,
		WS_CHILD | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
		0, 0,
		0, 0,     
		parent,
		hmenu,
		dllself._module,
		NULL);

	SendMessageA(_hwnd, UDM_SETRANGE, 0, MAKELPARAM(max, min));    // Sets the controls direction 

	return _hwnd;
}

HWND CreateGroupBox(HWND parent, HMENU hmenu, const char* name, int x, int y, int width, int height)
{
	HWND _hwnd = CreateWindowExA(WS_EX_LEFT | WS_EX_CONTROLPARENT | WS_EX_LTRREADING,
		WC_BUTTONA,
		name,
		WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
		x, y,
		width, height,
		parent,
		hmenu,
		dllself._module,
		NULL);

	return (_hwnd);
}
//������
HWND CreateProgBar(HWND parent, HMENU hmenu, const char* name, int x, int y, int width, int height)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_PROGRESS_CLASS;    // Set the Initialization Flag value.
	InitCommonControlsEx(&icex);        // Initialize the Common Controls Library to use the Progress Bar control.

	HWND _hwnd = CreateWindowExA(WS_EX_STATICEDGE,
		PROGRESS_CLASSA,
		name,
		WS_CHILDWINDOW | WS_VISIBLE | PBS_SMOOTH,
		x,y,
		width, height,
		parent,
		hmenu,
		dllself._module,
		NULL);

	// Set the range and increment of the progress bar.
	SendMessageA(_hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendMessageA(_hwnd, PBM_SETSTEP, (WPARAM)1, 0);

	return (_hwnd);
}