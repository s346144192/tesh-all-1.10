#include <windows.h>
#include <stdio.h>

int __atoi(const char*  pc) {
	int n = 0;
	int i = 0;
	int s = *pc;
	if (s == '-') { ++pc; }
	for (; *pc && i < 10; ++pc)
	{
		if (*pc < 48 || *pc>57)
		{
			break;
		}
		n = n * 10 + (*pc - 48);
		i++;
	}
	if (s == '-') { return -n; }
	return n;
}
char static_path[MAX_PATH] = { 0 };
char* get_path() {
	//获取当前程序所在路径
	GetModuleFileNameA(NULL, static_path, MAX_PATH);
	//获取当前程序所在目录
	char* p = strrchr(static_path, '\\');
	if (p != NULL) {
		p[0] = 0;
	}
	return static_path;
}
int get_pchar_length(const char* c) {
	int n = 0;
	while (c != NULL) {
		n++;
		c++;
	}
	return n;
}
char static_locpath[MAX_PATH] = { 0 };
char* to_locfile_path(const char* name) {
	if (name == 0) {
		return 0;
	}
	sprintf_s(static_locpath, "%s\\%s", get_path(), name);
	return static_locpath;
}

double time_to_timestamp(const char* time) {
	if (!time) {
		return 0;
	}
	int len = strlen(time);
	if (!len) {
		return 0;
	}
	const char* it = time;
	char c = 0;
	int idx = 0; int flag = 0;
	int ymd_hms_ms[7] = { 0,0,0,0,0,0,0 };
	for (; *it && idx < 7; ++it) {
		c = *it;
		switch (c) {
		case '-':
			flag = 1;
			idx++;
			break;
		case ' ':case ':':
			idx++;
			break;
		case '.':
			idx++;
			break;
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
			ymd_hms_ms[idx] = ymd_hms_ms[idx] * 10 + (c - '0');
			break;
		default:
			break;
		}
	}
	for (int i = 0; !flag && idx < 6 && i <= idx; i++) {
		ymd_hms_ms[6 - i] = ymd_hms_ms[idx - i];
		ymd_hms_ms[idx - i] = 0;
	}

	int sec = 0;
	int y_day = 365;
	for (int i = 1970; i < ymd_hms_ms[0]; i++) {
		y_day = 365;
		if (i == i / 4 * 4 && (i != i / 100 * 100 || i == i / 400 * 400)) {
			y_day++;
		}
		sec = sec + y_day * 86400;
	}
	int months[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	for (int i = 0; i < ymd_hms_ms[1] - 1; i++) {
		sec = sec + months[i] * 86400;
	}
	if (ymd_hms_ms[1] >= 2) {
		sec = sec - (2 - (y_day - 365)) * 86400;
	}
	sec = sec + ymd_hms_ms[2] * 86400 - 28800;
	sec = sec + ymd_hms_ms[3] * 3600;
	sec = sec + ymd_hms_ms[4] * 60;
	sec = sec + ymd_hms_ms[5];

	double ms = ymd_hms_ms[6];

	while (ms >= 1) {
		ms = ms / 10;
	}
	double retvalue = sec + ms;
	return retvalue;
}
