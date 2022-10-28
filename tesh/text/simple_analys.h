#pragma once
#ifndef SIMPLE_ANALYS
#define SIMPLE_ANALYS
#include <wtypes.h>

static class syntax_symbols {
private:
	unsigned char symbol_table[256] = { 0 };
	const char* space_symbols = " \t\v\r\n";
	const char*  sng_symbols = "$-.*+<>!|=?:(){}[],^~;%&#@`/\\'\"";//_
	const char* word_chars = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
public:
	enum symboltype {
		null = 0,
		space = 1,
		symbol = 2,
		word = 3,
		other = 4,
	};
	syntax_symbols() {
		const char* str = nullptr;
		for (int i=1;i< sizeof(symbol_table); i++) { symbol_table[i] = other; }
		str = space_symbols; for (; *str; ++str) { symbol_table[(unsigned char)*str] = space; }
		str = sng_symbols;  for (; *str; ++str) { symbol_table[(unsigned char)*str] = symbol; }
		str = word_chars;   for (; *str; ++str) { symbol_table[(unsigned char)*str] = word; }
	}
	
	bool is_space(char ch) {
		return symbol_table[(unsigned char)ch] == space;
	}
	bool is_symbol(char ch) {
		return symbol_table[(unsigned char)ch] == symbol;
	}
	bool is_keyword(char ch) {
		return symbol_table[(unsigned char)ch] == word;
	}
	symboltype get_type(char ch) {
		return (symboltype)symbol_table[(unsigned char)ch];
	}
}symbols;



class simple_analys {

public:
	char* _text;
	char* _pos;
	char* _end;

	char get_next_char(const char* str, int next)
	{
		if (!str) {
			return 0;
		}
		const char* cc = str;
		for (int i = 0; i < next; i++)
		{
			++cc;
			if (!(*cc)) {
				return 0;
			}
		}
		return *cc;
	}
	void next_line(char* &str, BOOL skipsymbol) {
		if (!str) {
			return;
		}
		for (; *str; ++str) {
			switch (*str)
			{
			case '\r':
				if (skipsymbol) {
					++str;
					if (*str == '\n') {
						++str;
					}
				}
				return;
			case '\n':
				if (skipsymbol) {
					++str;
					if (*str == '\r') {
						++str;
					}
				}
				return;
			default:
				break;
			}
		}
	}
	void next_line(BOOL skipline) {
		next_line(_pos, skipline);
	}
	void next_comment(char* &str) {
		if (!str) {
			return;
		}
		for (; *str; ++str) {
			if (str[0] = '*' && str[1] == '/') {
				str += 2;
				return;
			}
		}
	}
	void next_comment() {
		next_comment(_pos);
	}
	void next_spacechar(char* &str, bool Multiline = false) {
		if (!str) {
			return;
		}
		for (; *str; ++str) {
			switch (*str)
			{
			case '\n':
				if (!Multiline) {
					return;
				}
			case '\t':
			case ' ':
				return;
			}
		}
	}
	void next_spacechar() {
		next_spacechar(_pos);
	}

	void  next_word(char* &str, bool Multiline = false) {
		if (!str) {
			return;
		}
		for (; *str; ++str) {
			switch (*str)
			{
			case '\n':
			case '\r':
				if (!Multiline) {
					return;
				}
			case '\v':
			case '\t':
			case ' ':
				break;
			default:
				return;
			}
		}
	}

	void  next_word(bool Multiline = false) {
		next_word(_pos, Multiline);
	}
	void  next_word_s(char* &str, bool Multiline = false) {
		next_spacechar(str, Multiline);
		next_word(str, Multiline);
	}
	void  next_word_s(bool Multiline = false) {
		next_word_s(_pos, Multiline);
	}
	int  get_space_width(char* str) {
		int width = 0;
		for (char ch= *str; ch;) {
			if (ch==' ') {
				width++;
			}
			else if (ch == '\t') {
				width+=4;
			}
			else {
				break;
			}
			++str; ch = *str;
		}
		return width;
	}
	int to_line_start(char* &str, char* start) {
		if (!str) {
			return 0;
		}
		char* savedpos = str;
		for (;str > start && *str; --str) {
			switch (*str)
			{
			case '\n':
				str += (savedpos > str);
				return savedpos-str;
			default:
				break;
			}
		}
		return savedpos - str;
	}
	int to_line_start() {
		return to_line_start(_pos, _text);
	}
	int to_line_end(char* &str, char* end) {
		if (!str) {
			return 0;
		}
		char* savedpos = str;
		for (; str < end && *str; ++str) {
			switch (*str)
			{
			case '\n':
				return str - savedpos;
			default:
				break;
			}
		}
		return str - savedpos;
	}
	void last_spacechar(char* &str, bool Multiline = false) {
		if (!str) {
			return;
		}
		for (; *str; --str) {
			switch (*str)
			{
			case '\n':
				if (!Multiline) {
					return;
				}
			case '\t':
			case ' ':
				return;
			}
		}
	}
	void last_spacechar() {
		last_spacechar(_pos);
	}

	void last_word(char* &str, bool Multiline = false) {
		if (!str) {
			return;
		}
		for (; *str; --str) {
			switch (*str)
			{
			case '\n':
			case '\r':
				if (!Multiline) {
					return;
				}
			case '\v':
			case '\t':
			case ' ':
				break;
			default:
				return;
			}
		}
	}
	void last_word(bool Multiline = false) {
		last_word(_pos, Multiline);
	}
	void last_word_s(char* &str, bool Multiline = false) {
		last_spacechar(str, Multiline);
		last_word(str, Multiline);
	}
	void last_word_s(bool Multiline = false) {
		last_word_s(_pos, Multiline);
	}

	bool skip_comment(char* &str, BOOL skipsymbol) {
		if (*str != '/') {
			return false;
		}
		if (str[1] == '/') {
			next_line(str, skipsymbol);
			return true;
		}
		else if (str[1] == '*') {
			next_comment(str);
			return true;
		}
		return false;
	}
	bool skip_comment(BOOL skipline) {
		return skip_comment(_pos, skipline);
	}
	char* search_text(char* str, const char* text_eof) {
		if (!str || !text_eof) {
			return 0;
		}
		char* _eof_pos = (char*)text_eof;
		char* cc = str;
		char c = 0;
		for (; (c = *cc) && _eof_pos; ++cc) {
			_eof_pos = (char*)text_eof;
			for (; *_eof_pos; ++_eof_pos) {
				if (c == *_eof_pos) {
					--cc;
					_eof_pos = 0;
					break;
				}
			}
		}
		int len = cc - str;
		if (len == 0) {
			return nullptr;
		}
		/*if (str + len > end && end > str) {
			len = end - str;
		}*/
		return cc;
	}
	char* search_line_string(char* src, const char* str) {
		if (!src || !str ) {
			return 0;
		}
		int len = strlen(str);
		if (len == 0) {
			return 0;
		}

		int i = 0;
		char* cc = src;
		char c = 0;
		for (; *cc && *cc != '\n'; ++cc) {
			if (skip_comment(cc, TRUE)) {
				return 0;
			}
			for (i = 0; (c = get_next_char(cc, i)) && i < len; i++) {
				if (c != str[i]) {
					break;
				}
			}
			if (i == len) {
				return cc;
			}
		}
		return 0;
	}
	char* search_line_string(const char* str) {
		return search_line_string(_pos, str);
	}
	char* search_next_string(char* src, const char* str) {
		if (!src || !str) {
			return 0;
		}
		int len = strlen(str);
		if (len == 0) {
			return 0;
		}

		int i = 0;
		char* cc = src;
		char c = 0;
		for (; *cc; ++cc) {
			skip_comment(cc, TRUE);
			for (i = 0; (c = get_next_char(cc, i)) && i < len; i++) {
				if (c != str[i]) {
					break;
				}
			}
			if (i == len) {
				return cc;
			}
		}
		return 0;
	}
	char* search_next_string(const char* str) {
		return search_next_string(_pos, str);
	}
	bool test_charboundary(char c) {
		switch (c)
		{
		case '\0':
		case '\t':
		case ' ':
		case '\n':
		case '\r':
			return true;
		}
		return false;
	}

	bool test_word(char* &src, const char* text) {
		if (!src || !text) {
			return false;
		}
		char* cc = src;
		int len = strlen(text);
		for (int i = 0; i < len; i++) {
			if (!(*cc) || *cc != text[i])
			{
				return false;
			}
			++cc;
		}
		if (!test_charboundary(*cc)) {
			return false;
		}
		return true;
	}
	bool test_word(const char* text) {
		return test_word(_pos, text);
	}
	bool test_lastword(char* &str, const char* text) {
		if (!str || !text) {
			return false;
		}
		char* cc = str;
		int len = strlen(text);
		for (int i = len - 1; i >= 0; i--) {
			if (!(*cc) || *cc != text[i])
			{
				return false;
			}
			--cc;
		}
		if (!test_charboundary(*cc)) {
			return false;
		}
		return true;
	}
	bool test_lastword(const char* text) {
		return test_lastword(_pos, text);
	}
	char* copy_newstring(char* start, int len) {
		if (!start) {
			return 0;
		}
		if (len <= 0) {
			return 0;
		}
		char* str = (char*)malloc(len + 1);
		memcpy(str, start, len);
		str[len] = 0;
		return str;
	}

	size_t get_hex_len(char* src, int max_len) {
		if (!src) {
			return 0;
		}
		char* cc = src;
		if (*cc == '-'){++cc;}
		if (!*cc) {return 0;}
		if (*cc!='$' && (*cc != '0' || toupper(cc[1]) != 'X')) {
			return 0;
		}
		cc += 2;char c = 0;
		for (; *cc && src - cc < max_len; ++cc) {
			c = (char)toupper(*cc);
			if (c < 'A' || c >'F') {
				break;
			}
		}
		return cc - src;
	}
	size_t get_octal_len(char* src, int max_len) {
		if (!src) {
			return 0;
		}
		char* cc = src;
		if (*cc == '-')
		{
			++cc;
		}
		if (*cc != '0') {
			return 0;
		}
		++cc;
		for (; *cc && src - cc < max_len; ++cc) {
			if (*cc < '0' || *cc >'7') {
				break;
			}
		}
		return cc - src;
	}
	size_t get_float_len(char* src, int max_len) {
		if (!src) {
			return 0;
		}
		char* cc = src;
		if (*cc == '-')
		{
			++cc;
		}
		BOOL ExistPoint = FALSE;
		char c = 0;
		for (; *cc && src - cc < max_len; ++cc) {
			c = *cc;
			if (c == '.') {
				if (!ExistPoint) {
					ExistPoint = TRUE;
				}
				else {
					--cc;
					break;
				}
			}
			else if (c<'0' || c>'9') {
				--cc;
				break;
			}
		}
		/*if (ExistPoint) {

		}*/
		return cc - src;
	}
	size_t get_number_len(char* src, int* bit) {
		*bit = -1;
		if (!src) {
			return 0;
		}
		BOOL noctal = FALSE;
		BOOL nhex = FALSE;
		BOOL nfloat = FALSE;

		BOOL StartJassHex = FALSE;
		BOOL Start0 = FALSE;
		BOOL ExistA_F = FALSE;
		BOOL ExistPoint = FALSE;

		BOOL skip = FALSE;
		char* once8_9 = 0;
		char* cc = src;
		if (*cc == '-') {++cc;	}
		for (; !skip && *cc; ++cc) {
			switch (toupper(*cc))
			{
			case '0':
			{
				if (src == cc) {
					Start0 = TRUE;nhex = (toupper(cc[1]) == 'X');cc += (nhex) ? 1 : 0;
				}
			}
			break;
			case '$':
			{
				if (src == cc) {StartJassHex = TRUE;nhex = TRUE;}
				else {--cc; skip = TRUE; continue;}
			}
			break;
			case '.':
				if (ExistPoint) {
					--cc; skip = TRUE;continue;
				}
				ExistPoint = TRUE;
				if (ExistA_F || StartJassHex) {
					--cc; skip = TRUE;continue;
				}
				if (cc - src == 0 && (cc[1] < '0' || cc[1] > '9')) {
					--cc; skip = TRUE;continue;
				}
				nfloat = TRUE;
				break;
			case '1':case '2':case '3':case '4':case '5':case '6':case '7':
				break;
			case '8':case '9':
				if (!once8_9) {
					once8_9 = cc;
				}
				break;
			case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
				if (!nhex) {
					--cc; skip = TRUE;continue;
				}
				ExistA_F = TRUE;
				break;
			default:
			{
				--cc;skip = TRUE;
			}
			break;
			}

		}
		noctal = (Start0 && !nhex && !ExistPoint);
		if (noctal) {
			if (ExistA_F) {
				return 0;
			}
			if (once8_9 > src) {
				cc = once8_9 - 1;
			}
		}
		if (nfloat) {
			*bit = 0;
		}
		else if (noctal) {
			*bit = 8;
		}
		else if (nhex) {
			*bit = 16;
		}
		else {
			*bit = 10;
		}
		return cc - src;;
	}
	char* get_number_text(char* src, char* end = 0) {

		int bit = 0;
		size_t len = get_number_len(src, &bit);
		if (bit == -1 || len < 1) {
			return NULL;
		}
		if (src + len > end && end > src) {
			len = end - src;
		}
		return copy_newstring(src, len);
	}
	int get_int(char* src, size_t* textlen) {
		int bit = 0;
		*textlen = get_number_len(src, &bit);
		if (bit <= 0 || *textlen<=0) {
			return 0;
		}
		char* str1 = src;
		char* end = src + *textlen;
		BOOL minus = FALSE;
		int n = 0;
		if (*str1 == '-') {
			minus = TRUE;
			++str1;
		}
		if ((*str1 == '0' && toupper(get_next_char(str1, 1)) == 'X')) {
			str1+=2;
		}
		else if (*str1 == '0') {
			++str1;
		}
		char c = 0;
		for (; *str1 && str1 <= end; ++str1) {
			c = *str1;
			if (c >= '0' && c <= '9') {
				n = n * bit;
				n = n + (c - '0');
			}
			else {
				c = (char)toupper(c);
				if (c >= 'A' && c <= 'F') {
					n = n * bit;
					n = n + (c - 'A' + 10);
				}
				else {
					break;
				}
			}
		}
		if (minus) {
			return -n;
		}
		return n;
	}
	int get_int(size_t* len) {
		int i = get_int(_pos, len);
		return i;
	}
	int get_int() {
		size_t len = 0;
		int i = get_int(_pos, &len);
		return i;
	}
	float get_float(char* src, size_t* textlen) {
		*textlen = get_float_len(src, 64);
		if (*textlen <= 0) {
			return 0;
		}
		char* str1 = src;
		char* end = src + *textlen;
		BOOL minus = FALSE;
		float f = 0;
		if (*str1 == '-') {
			minus = TRUE;
			++str1;
		}
		int decimalbit = 1;
		char c = 0;
		for (; *str1 && str1 <= end; ++str1) {
			c = *str1;
			if (c == '.') {
				if (decimalbit) {
					break;
				}
				decimalbit = 10;
			}
			if (c >= '0' && c <= '9') {
				if (decimalbit == 1) {
					f = f * 10;
				}
				else {
					decimalbit = decimalbit * 10;
				}
				if (decimalbit) {
					f = f + (((float)(c - '0')) / (float)decimalbit);
				}
			}
			else {
				break;
			}
		}
		if (minus) {
			return -f;
		}
		return f;
	}
	float get_float() {
		size_t len = 0;
		float f = get_float(_pos, &len);
		return f;
	}
	int get_word_len(char* str) {
		char* cc = str;
		for (; *cc; ++cc) {
			if (test_charboundary(*cc)) {
				break;
			}
		}
		return cc - str;
	}
	char* get_word(char* str, char* end = 0) {
		int len = get_word_len(str);
		if (len == 0) {
			return nullptr;
		}
		if (str + len > end && end > str) {
			len = end - str;
		}
		return copy_newstring(str, len);
	}
	char* get_word() {
		return get_word(_pos);
	}
	char* get_text(char* str, const char* text_eof, char* end = 0) {
		/*if (!str || !text_eof) {
			return 0;
		}*/
		char* cc = search_text(str, text_eof);
		if (!cc) {
			return nullptr;
		}
		int len = cc - str;
		if (len == 0) {
			return nullptr;
		}
		if (str + len > end && end > str) {
			len = end - str;
		}
		return copy_newstring(str, len);
	}

	char* get_ini_line_s(char* src) {
		if (!src) {
			return 0;
		}
		char* cc = src;
		char* start = cc;
		char* end = cc;
		next_word(start);
		next_line(end, FALSE);
		last_word(end);

		cc = start;
		for (; cc < end; ++cc) {
			if (*cc == '/')
			{
				if (get_next_char(cc, 1) == '/' || get_next_char(cc, 1) == '*') {
					break;
				}
			}
			if (*cc == ';') {
				break;
			}
		}
		end = cc;

		if (start < end) {
			char* text = get_number_text(start);
			if (text) {
				return text;
			}
			return copy_newstring(start, end - start);
		}
		return nullptr;
	}

	void load(char* text) {
		_text = text;
		_pos = text;
		_end = 0;
		if (text) {
			_end = _pos + strlen(text);
		}
	}
	simple_analys() {
		_text = 0;
		_pos = 0;
		_end = 0;
	}
	simple_analys(char* text) {
		load(text);
	}
	~simple_analys() {
		_text = 0;
		_pos = 0;
		_end = 0;
	}
};



#endif