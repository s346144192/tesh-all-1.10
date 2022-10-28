#pragma once

#include "resource.h"
#include <Options.h>
#include <FuncList.h>
#include <CodeEdit.h>
#include <CodeEdit.h>
#include <TeshAbout.h>

class TESH {
public:
	static ini::config_t config, styles;
	FuncListView FuncListWin;
	CodeEditView CodeEditBox;
	Options _Options;
	TeshAbout _TeshAbout;
	void Resize();
	void ReadJassText();
	void InitConfig();
	BOOL Init(HWND parent);
	
};