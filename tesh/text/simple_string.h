#pragma once
#include <stdio.h>
#include <corecrt_malloc.h>
#include <string.h>
//大量的字符串处理 效率不如std::string
namespace simple_string {
	int search_string(const char* src, size_t begin, size_t end, const char* str) {
		const char* srcback=src+end;
		const char* curstr = str;
		int cmpbool=0;
		src += begin;
		for (; *src && src< srcback;++src) {
			//不等于==1,等于==0
			cmpbool = (*src - *curstr) & 0xFF;
			cmpbool = ((cmpbool << 1) & 2)>>1;
			curstr = str;
		}

	}
	char* str_malloc(size_t _size) {
		char* newstr = (char*)malloc(_size + 1);
		memset(newstr, 0, _size + 1);
		return newstr;
	}
	char* create_str(const char* str) {
		size_t len = strlen(str);
		char* newstr = (char*)malloc(len + 1);
		memset(newstr, 0, len + 1);
		memmove(newstr, str, len);
		return newstr;
	}
	char* create_str(const char* str, size_t _size) {
		size_t len = strlen(str);
		size_t copylen = (_size < len) ? _size : len;
		char* newstr = (char*)malloc(_size + 1);
		memset(newstr, 0, _size + 1);
		memmove(newstr, str, copylen);
		return newstr;
	}

	void remove_str(char* &str) {
		free(str);
		str = nullptr;
	}
	//不保留之前的字符串
	void str_resise_notreserve(char* &str, size_t _size) {
		char* newstr = (char*)malloc(_size + 1);
		memset(newstr, 0, _size + 1);
		free(str);
		str = newstr;
	}
	void str_resise(char* &str,size_t _size) {
		size_t max_ = _msize(str);
		size_t copylen = (_size < max_ - 1) ? _size : max_ - 1;
		char* newstr = (char*)malloc(_size +1);
		memset(newstr, 0, _size + 1);
		memmove(newstr, str, copylen);
		free(str);
		str = newstr;
	}
	void str_push_back(char* &str,char ch) {
		size_t n = strlen(str);
		if (n + 2 > _msize(str)) {
			str_resise(str,n + 1);
		}
		str[n] = ch;
		str[n + 1] = 0;
	}
	void str_append(char* &str,const char* appendstr,size_t appendlen) {
		size_t max_ = _msize(str);
		size_t len = strlen(str);
		if (appendlen <=0) {
			appendlen = strlen(appendstr);
		}
		size_t maxlen = len + appendlen + 1;
		if (maxlen >max_) {
			char* newstr= (char*)malloc(maxlen);
			memmove(newstr,  str, len);
			memmove(newstr + len, appendstr, appendlen);
			free(str);
			str = newstr;
			str[maxlen - 1] = 0;
		}
		else {
			memmove(str + len, appendstr, appendlen);
			str[maxlen - 1] = 0;
		}
	}
	void str_append(char* &str, const char* addstr) {
		str_append(str, addstr,0);
	}
	void str_assign(char* &str, const char* assignstr, size_t assignlen) {
		size_t max_ = _msize(str);
		if (assignlen <= 0) {
			assignlen = strlen(assignstr);
		}
		if (assignlen< max_) {
			memset(str,0,max_);
		}
		else {
			str_resise_notreserve(str, assignlen+1);
		}
		strncpy_s(str, assignlen, assignstr, assignlen);
	}
	void str_assign(char* &str, const char* assignstr) {
		str_assign(str, assignstr,0);
	}
}
