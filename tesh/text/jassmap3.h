#pragma once

#include <text/simple_analys.h>
#include <file/fpfile.h>
#include <vector>
#include <map>

//
//
//class keyword_node {
//	static BYTE indexs[256];
//	char list[64];
//public:
//	BOOL initindexs(){
//		const char keystr[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//		for (int i = 0; i < sizeof(keystr); i++) {
//			indexs[(BYTE)keystr[i]] = i + 1;
//		}
//		return TRUE;
//	}
//	keyword_node() {
//		static BOOL isinit = initindexs();
//		ZeroMemory(list,sizeof(list));
//	}
//	char& operator [](BYTE i) {
//		return list[indexs[i]];
//	}
//};
//BYTE keyword_node::indexs[256] = { 0 };
//class keyword_table {
//	
//	keyword_node* keywords;
//	size_t maxlength;
//	void init(size_t _maxlength) {
//		maxlength = _maxlength;
//		keywords = new keyword_node[_maxlength];
//	}
//	void add(const char* str) {
//		for (size_t i = 0;*str &&  i < maxlength; ++i) {
//			++(keywords[i][(BYTE)*str]);
//			++str;
//		}
//	}
//	bool match(const char* str) {
//		int level = 0;
//		for (size_t i = 0; str[i] && i < maxlength; ++i) {
//			level = keywords[i][(BYTE)*str];
//			if (!level) {
//				return false;
//			}
//			if (!level) {
//				return false;
//			}
//		}
//	}
//};
//










//debug: 2MB大小jass文件,词法分解用时约1.5s,占用内存约12.7MB。
//release: 2MB大小jass文件,词法分解用时约60ms,占用内存约8MB。

//引用字符串,需保存此结构。
class jass_analys_scaner :public simple_analys {
#define WIN32_MALLOC
#ifdef WIN32_MALLOC
#define MALLOC(size) VirtualAlloc(NULL,size,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE)
#define FREE(address) VirtualFree(address,0,MEM_RELEASE)
#else
#define MALLOC(size) malloc(size)
#define FREE(address) free(address)
#endif
#define	enable_wordlist_init_reserve
#ifdef enable_wordlist_init_reserve
#define wordlist_init_reserve wordlist.reserve(128);
#define wordline_init_reserve line->reserve(8);
#else
#define wordlist_init_reserve
#define wordline_init_reserve
#endif
private:
	typedef std::vector<const char*> WordLine;
	typedef std::vector<WordLine> WordList;
	char* _buffer;
	char* _wsbufpos;
	size_t _bufsize = 0;
	size_t _wscount = 0;
	
	//int _globals_count;
	//int _nativefunctions_count;
	//int _functions_count;
public:
	
	WordList wordlist;
	jass_analys_scaner() {
		_buffer = nullptr;
		_wsbufpos = nullptr;
		_bufsize = 0;
	}
	~jass_analys_scaner(){
		if (_buffer) {
			FREE(_buffer);
		}
		_buffer = nullptr;
		_wsbufpos = nullptr;
		_bufsize = 0;
	}
	void clear_buffer() {
		if (_buffer) {
			FREE(_buffer);
		}
		_buffer = nullptr;
		_wsbufpos = nullptr;
		_bufsize = 0;
	}
	//2倍缓存较为安全。
	char* create_buffer(size_t size) {
		clear_buffer();
		_wscount = 0;
		_wsbufpos = nullptr;
		_bufsize = size;
		_buffer = (char*)MALLOC(size + 16);
		#ifndef WIN32_MALLOC
		memset(_buffer,0, size + 16);
		#endif
		return _buffer+8;
	}
	char* get_start_pos() {
		if (!_buffer) {
			return nullptr;
		}
		return _buffer + 8;
	}
	size_t get_buffer_size() {
		if (!_buffer) {
			return 0;
		}
		return (size_t)_buffer - 16;
	}
	//计算所需的缓冲大小。(不太准确)
	size_t scan_text_tobuffer_size(char* str) {
		size_t chwarp_t[256] = { 0 };
		size_t chspace_t[256] = { 0 };
		chwarp_t['\n'] = TRUE;
		chwarp_t['\r'] = TRUE;
		chspace_t[' '] = TRUE;
		chspace_t['\t'] = TRUE;
		char* strpos = str;
		size_t warp_count = 0;
		size_t delimiter_count = 0;
		size_t delimiter_on_off = TRUE;
		for(unsigned char ch;(ch=(signed char)*strpos); ++strpos) {
			warp_count += chwarp_t[ch];
			delimiter_count += chspace_t[ch] & delimiter_on_off;
			delimiter_on_off = !chspace_t[ch];
		}
		size_t strsize = (size_t)strpos - (size_t)str;
		size_t buf_size = strsize + ((warp_count*2+delimiter_count + 3) * 2);
		//最小缓冲大小
		size_t buf_minsize = strsize + (strsize / 100 * 20);//120%
		if (buf_size < buf_minsize) {
			buf_size = buf_minsize;
		}
		return buf_size;
	}

	
	void buffer_put(WordLine* line,char* &_bufpos,char ch) {
		if (_bufpos[-1]) { ++_bufpos; }
		line->push_back(_bufpos);
		*_bufpos = ch;
		_bufpos+=2;
		_wscount++;
	}
	void buffer_put(WordLine* line, char* &_bufpos, char ch1,char ch2) {
		if (_bufpos[-1]) { ++_bufpos; }
		line->push_back(_bufpos);
		*_bufpos++ = ch1;
		*_bufpos = ch2;
		_bufpos += 2;
		_wscount++;
	}
	void buffer_put(WordLine* line, char* &_bufpos,const char* str,size_t len) {
		if (_bufpos[-1]) { ++_bufpos; }
		line->push_back(_bufpos);
		for (; len; --len) {
			*_bufpos++ = *str++;
		}
		_bufpos++;
		_wscount++;
	}
	void buffer_put(WordLine* line, char* &_bufpos, const char* str) {
		if (_bufpos[-1]) { ++_bufpos; }
		line->push_back(_bufpos);
		for (; *str;++str) {
			*_bufpos++ = *str;
		}
		_bufpos++;
		_wscount++;
	}
	WordLine* put_line(WordList& wslist) {
		wslist.push_back(WordLine());
		return &wslist.back();
	}
	WordLine* put_line() {
		return put_line(wordlist);
	}

	bool is_word(char ch) {
		return (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
	}
	bool is_hex(char ch) {
		return  (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
	}
	bool is_number(char ch) {
		return  (ch >= '0' && ch <= '9');
	}
	bool is_octal(char ch) {
		return  (ch >= '0' && ch <= '7');
	}



	void next_lua_comment(char* &str, char* end, BOOL skipsymbol) {
		bool lua_comment_mltstart = false;
		int lua_comment_mlt = 0;
		int lua_comment_mlttype = 0;
		int lua_comment_n = 2;
		int lua_comment_level = 0;
		char*str2 = str + 2;
		bool _EOF = false;

		if (!str || (end - str) < 2) {
			return;
		}
		//ydwe_lua_import不是注释  (" import 'main.lua'[[ ]] ")
		if (str[0] != '-' || str[1] != '-') {
			return;
		}
		std::string dbgstr("");
		for (; !_EOF && str2 < end && *str2; ++str2) {
			//dbgstr.push_back(*str2);
			switch (*str2)
			{
			case '-':
				if (lua_comment_mlt == 0)
				{
					lua_comment_n++;
				}
				break;
			case '=':
				if (lua_comment_mlt == 1)
				{
					if (lua_comment_mltstart) {
						lua_comment_level--;
					}
					else {
						lua_comment_level++;
					}
				}
				break;
			case '[':
				if (lua_comment_n == 2 && lua_comment_mlt < 2 && (lua_comment_mlttype == 0 || lua_comment_mlttype == 1)) {
					if (lua_comment_mltstart) {
						break;
					}
					lua_comment_mlt++;
					lua_comment_mlttype = 1;
					if (lua_comment_mlt == 2) {
						lua_comment_mltstart = true;
						lua_comment_mlttype = 0;
						lua_comment_mlt = 0;
					}
				}
				break;
			case ']':
				if (lua_comment_mlt < 2 && (lua_comment_mlttype == 0 || lua_comment_mlttype == 2)) {
					lua_comment_mlt++;
					if (lua_comment_mlt == 2 && (lua_comment_level >= 0 || !lua_comment_mltstart)) {
						_EOF = true;
					}
				}
				break;
			case '\\':
				++str2;
				break;
			case '\n':
				if (!lua_comment_mltstart) {
					if (!skipsymbol) {
						str2--;
					}
					_EOF = true;
				}
				else {
					lua_comment_n = 0;
				}
				break;
			case '?':
				//按照ydwe的处理方法:单行注释的行?>有效,多行注释内无效。
				if (str2[1] == '>' && !lua_comment_mltstart) {
					str2 -= 1;
					_EOF = true;
				}
			default:
				lua_comment_mlt = 0;
				break;
			}
		}
		str = str2;

	}

	std::string get_line_text(size_t i) {
		std::string str = "";
		if (i < wordlist.size()) {
			for (auto it = wordlist[i].begin(); it!=wordlist[i].end(); ++it) {
				str.append((*it));
			}
		}
		return str;
	}
	std::string get_cur_line_string(size_t maxnchar = 0) {
		std::string line("");
		char* str1 = _pos;
		char* str2 = _pos;
		to_line_start(str1, _text);
		next_line(str2, FALSE);
		if (str1 < str2) {
			if (maxnchar <= 0) { maxnchar = 0xFFFF; }
			line.assign(str1, min(maxnchar, str2 - str1));

		}
		return line;
	}

	//void put_globals(jass_map& globals) {

	//}
	//void put_nativefunction(jass_map& nativefunctions) {

	//}
	//void put_function(jass_map& functions) {

	//}


	void init_wordlist() {
		// table 索引 相对 if 快数倍。
		static char utf8textflag[4] = { 0xEF,0xBB,0xBF,0 };
		//char pchar[4] = { 0 };
		char* str1 = nullptr;
		char* start = 0;
		char* word_start = 0;
		char* word_pos = 0;
		int numlen = 0;
		int numbit = 0;
		bool lua_range = false;
		bool is_next_newline = false;
		if (_end - _pos > 3) {
			if (strncmp(utf8textflag, _pos, 3) == 0) {
				_pos += 3;
			}
		}
		//创建2倍缓冲。(虽然忽略空白字符,符号等会增加一个字符大小)
		_wsbufpos =create_buffer((_end - _pos) * 2);
		wordlist.clear();
		wordlist_init_reserve
		WordLine*line = put_line();
		wordline_init_reserve
		unsigned char space_table[256] = { 0 };
		space_table[(unsigned char)'\t'] = 1;
		space_table[(unsigned char)' '] = 1;
		start = _pos;
		for (; _pos < _end; ++_pos) {
			//pchar[0] = *_pos; pchar[1] = 0; pchar[2] = 0;
			switch (*_pos)
			{
			case '\t':
			case ' ':
				/*if (line->size() > 0) {buffer_put(line, _wsbufpos, pchar[0]);}*/
				for (;(space_table[_pos[1]]); ++_pos) {}
				break;
			case '\n':
				//printf("%d: %s\n", wordlist.size(), get_line_text(wordlist.size() - 1).data());
				if (_pos[1] == '\r') {++_pos;}
				buffer_put(line, _wsbufpos, '\n');
				line = put_line();
				wordline_init_reserve
				break;
			case '\r':
				if (_pos[1] == '\n') {++_pos;}
				buffer_put(line, _wsbufpos, '\n');
				line = put_line();
				//wordline_init_reserve
				break;
				//忽略
			case '\v':
				break;
				//上一个字符不等于word
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				/*if (word_pos == _pos - 1) {
					*_wsbufpos++= *_pos;
					word_pos = _pos; break;
				}*/
				//$ jass16进制
			case '$':
				numlen = get_number_len(_pos, &numbit);
				if (numlen > 0) {
					buffer_put(line, _wsbufpos, _pos, numlen);
					_pos = _pos + numlen - 1;
				}
				else {
					buffer_put(line, _wsbufpos, *_pos);
				}
				break;
				//-- 自减, - -减负数
			case '-':
				if (lua_range && _pos[1] == '-') {
					str1 = _pos;
					next_lua_comment(_pos, _end, FALSE);
					buffer_put(line, _wsbufpos, str1, _pos - str1);
					--_pos; break;
				}
				if (_pos[1] == '-' || _pos[1] == '>' || _pos[1] == '=') {
					buffer_put(line, _wsbufpos, _pos[0],_pos[1]); ++_pos; break;
				}
				numlen = get_number_len(_pos, &numbit);
				if (numlen > 1) {
					buffer_put(line, _wsbufpos, _pos, numlen);
					_pos = _pos + numlen - 1;
				}
				else {
					buffer_put(line, _wsbufpos, *_pos);
				}
				break;
				//跳到下列 与.的处理方法一致。(需更改为get_number_len)
			case '.':
				numlen = get_float_len(_pos, 255);
				if (numlen > 1) {
					buffer_put(line, _wsbufpos, _pos, numlen);
					_pos = _pos + numlen - 1;
				}
				else {
					buffer_put(line, _wsbufpos, *_pos);
					if (lua_range && _pos[1] == '.') { 
						*--_wsbufpos = _pos[1]; _wsbufpos += 2;
					}
					 break;
				}
				break;
			case '*':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '/' || _pos[1] == '='){ *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '+':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '+' || _pos[1] == '=') {*--_wsbufpos = _pos[1]; _wsbufpos += 2;  ++_pos; }
				break;
			case '>':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '>' || _pos[1] == '=') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '<':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '?') {
					*--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; lua_range = true; break;
					//printf("进入lua范围\n");
				}
				if (_pos[1] == '<' || _pos[1] == '=') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '!':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '=') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '|':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '|') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '=':
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '=') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '?':
				buffer_put(line, _wsbufpos, *_pos);
				if (lua_range && _pos[1] == '>') {
					*--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; lua_range = false;
					//printf("离开lua范围\n");
				}
				break;
			case ':':
				buffer_put(line, _wsbufpos, *_pos);
				if (lua_range && _pos[1] == ':') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
			case '(':
			case ')':
			case '{':
			case '}':
			case '[':
			case ']':
			case ',':
			case '^':
			case '~':
			case ';':
			case '%':
			case '&':
			case '#':
			case '@':
				buffer_put(line, _wsbufpos, *_pos);
				break;
				//未知
			case '`':
				buffer_put(line, _wsbufpos, *_pos);
				break;
			case '/':
				str1 = _pos;
				if (_pos[1] == '/') {
					//文本宏
					if (_pos + 2 < _end) {
						if (_pos[2] == '!') {
							_pos += 2;
							buffer_put(line, _wsbufpos, "//!");
							break;
						}
					}
					next_line(_pos, FALSE);
					buffer_put(line, _wsbufpos, str1, _pos - str1);
					--_pos;
					break;
				}
				else if (_pos[1] == '*') {
					next_comment(_pos);
					buffer_put(line, _wsbufpos, str1, _pos - str1);
					--_pos;
					break;
				}
				buffer_put(line, _wsbufpos, *_pos);
				if (_pos[1] == '=') { *--_wsbufpos = _pos[1]; _wsbufpos += 2; ++_pos; }
				break;
				//转义符
			case '\\':
				//是否连接下一行,需判断.换行后的空格不自动跳过.
				if (isspace(_pos[1])) {
					bool isexist_line_symbol = false;
					str1 = _pos + 1;
					for (; str1 < _end && isspace(*str1); ++str1) {
						if (*str1 == '\n') { isexist_line_symbol = true; break; }
					}
					if (isexist_line_symbol) {
						_pos = str1;
						buffer_put(line, _wsbufpos, "\\\n");
						break;
					}
				}
				buffer_put(line, _wsbufpos, _pos[0], _pos[1]);
				++_pos;
				break;

				//中间的字符大于4位(A-Z,a-z,0-9)内字符则不符合jass,lua为文本。
			case '\'':
				if (!lua_range) {
					str1 = _pos; ++_pos; numlen = 0;
					for (; _pos < _end && *_pos; ++_pos) {
						if (*_pos == '\\') { ++_pos; numlen++; }
						else if (*_pos == '\'') { break; }
						else { numlen++; }
					}
					//错误! 超过4个字符
					//if (numlen>4) {//printf("''超出范围!,%d行,%s\n",  wordlist.size(), get_cur_line_string().c_str());
						//return;
					//}
					buffer_put(line, _wsbufpos, str1, _pos - str1 + 1);
					break;
				}
				// " ' 使用相同处理方法,以兼容lua.
			case '"':
				str1 = _pos;
				++_pos;
				is_next_newline = false;
				for (; _pos < _end && *_pos; ++_pos) {
					if (*_pos == '\\') { ++_pos; }
					else if (*_pos == '\n') { is_next_newline = true; }
					else if (*_pos == str1[0]) { break; }
				}
				//循环结束能_pos+1,这里_pos若直接+1会少一个字符.
				buffer_put(line, _wsbufpos, str1, _pos - str1 + 1);
				if (is_next_newline) {
					buffer_put(line, _wsbufpos, '\n');
					line = put_line();
					wordline_init_reserve
				}
				break;
			default:
				/*if (is_word(pchar[0])) {*/
					word_pos = _pos+1;
					buffer_put(line, _wsbufpos, *_pos);
					--_wsbufpos;
					for (; symbols.get_type(*word_pos) >= 3; ++word_pos) {
						*_wsbufpos++ = *word_pos;
					}
					_pos= word_pos-1;
					/*if (word_pos < _pos - 1) {
						word_start = _pos;
						buffer_put(line, _wsbufpos, pchar[0]);
						--_wsbufpos;
					}
					else {
						*_wsbufpos++ = *_pos;
					}
					word_pos = _pos;*/
				//}
				//else {
				//	//错误!
				//	//printf("错误的结束符char(%d),%d行,%s\n\0", pchar[0], wordlist.size(), get_cur_line_string().c_str());
				//	return;
				//}
				break;
			}
		}
		if (_pos >= _end) {
			printf("词法分解:%d行.\n", wordlist.size());
		}

	}

	void save_buffer_tofile(const char* name) {
		if (_buffer == NULL) {
			return;
		}
		size_t bufsize = _bufsize;

		char* loc_buffer = (char*)malloc(bufsize+1);
		char*str = loc_buffer;
		memcpy(loc_buffer, _buffer, bufsize);
		for (;str< loc_buffer+ bufsize && ! *str;++str) {}
		char*str_s = str;
		for (; str[0] || str[1] || str[2]; ++str) {
			if (! *str) {
				*str = ' ';
			}
		}
		fpfile fp(name,"wt");
		if (fp.fp != NULL) {
			fp.puts(str_s);
		}
		free(loc_buffer);
		loc_buffer = NULL;
	}

	jass_analys_scaner(const char* text) {
		_buffer = nullptr;
		_wsbufpos = nullptr;
		load((char*)text);
		init_wordlist();
	}


#undef wordlist_init_reserve 
#undef wordline_init_reserve 
};

struct jass_node {
	jass_node* _last;
	jass_node* _next;
	std::string _value;
	jass_node(jass_node* last, std::string value) {
		_last = last;
		_next = NULL;
		_value = value;
	}
};
struct jass_syntax_node {
	jass_syntax_node* _parent;
	jass_syntax_node* _chids;
	jass_syntax_node* _last;
	jass_syntax_node* _next;
	std::string _value;
	void init(jass_syntax_node* parent, jass_syntax_node* last, std::string value) {
		_parent = parent;
		_chids = NULL;
		_last = last;
		_next = NULL;
		_value = value;
		if (_last) {
			_last->_next = this;
		}
		if (_parent) {
			if (_parent->_chids) {
				_parent->_chids->_next=this;
			}
			_parent->_chids = this;
		}
	}
	void remove() {
		jass_syntax_node* node1 = _chids;
		jass_syntax_node* node2 = NULL;
		while (node1) { node2 = node1; node1 = node1->_last; delete node2; }
		_chids = NULL;
	}

	jass_syntax_node() {
		_parent = NULL;
		_chids = NULL;
		_last = NULL;
		_next = NULL;
	}
	
	jass_syntax_node(jass_syntax_node* parent,  const char* value) {
		init(parent,NULL,value);
	}
	jass_syntax_node(jass_syntax_node* parent, jass_syntax_node* last, const char* value) {
		init(parent, last, value);
	}
	

	~jass_syntax_node() {
		remove();
		if (_last) {_last->_next = _next;}
		if (_next) {_next->_last = _last;}
		_parent = NULL;
		_chids = NULL;
		_last = NULL;
		_next = NULL;
	}
	jass_syntax_node* operator[](const char* name) {
		jass_syntax_node* node = _parent->_chids;
		while (node) {
			if (strcmp(name, node->_value.c_str()) == 0) {
				return node;
			}
			node = node->_next;
		}
		return NULL;
	}

	void operator()(const char* value) {
		jass_syntax_node* node= new jass_syntax_node(this, value);
	}
	void operator()(const char* value1, const char* value2) {
		new jass_syntax_node(this, value1);
		new jass_syntax_node(this, value2);
	}
	void operator()(const char* value1,const char* value2,const char* value3) {
		new jass_syntax_node(this, value1);
		new jass_syntax_node(this, value2);
		new jass_syntax_node(this, value3);
	}
	jass_syntax_node* nextadd(const char* value) {
		_next=new jass_syntax_node(_parent, value);
		return _next;
	}
	
};
struct jass_syntax_tree:public jass_syntax_node {
	typedef std::vector<const char*> WordLine;
	typedef std::vector<WordLine> WordList;
	jass_analys_scaner _scaner;

	jass_syntax_tree(const char* syntax_text){
		_scaner.load((char*)syntax_text);
		_scaner.init_wordlist();
		jass_syntax_node* cur_node = this;
		WordLine* line = NULL;
		const char* word = nullptr;
		for (auto it = _scaner.wordlist.begin(); it != _scaner.wordlist.end(); ++it) {
			line = &(*it);for (auto __it = line->begin(); __it != line->end(); ++__it) {
				word = (*__it);switch (symbols.get_type(word[0]))
				{
				case syntax_symbols::null:
					break;
				case syntax_symbols::word:
					cur_node = new jass_syntax_node(cur_node->_parent, cur_node, word);
					//printf("%s\n", word);
					break;
				case syntax_symbols::symbol:
				{
					switch (word[0])
					{
					case '(':
						cur_node = new jass_syntax_node(cur_node, word);
						//printf("-> %s\n", word);
						break;
					case ')':
						cur_node = new jass_syntax_node(cur_node->_parent, cur_node, word);
						cur_node = cur_node->_parent;
						//printf("<- %s\n", word);
						break;
					case '|':
					case '&':
						cur_node = new jass_syntax_node(cur_node->_parent, cur_node, word);
						//printf("%s\n", word);
						break;
					case '\\':
						cur_node = new jass_syntax_node(cur_node->_parent, cur_node, word + 1);
						//printf("%s\n", word+1);
						break;
					default:
						cur_node = new jass_syntax_node(cur_node->_parent, cur_node, word);
						//printf("%s\n", word);
						break;
					}
				}
				break;
				case syntax_symbols::other:break;
				}
			}}
	}
	~jass_syntax_tree(){
		jass_syntax_node* node1 = _next;
		jass_syntax_node* node2 = NULL;
		while (node1) { node2 = node1; node1 = node1->_next; delete node2; }
		_next = NULL;
	}
};


//大型list是否会栈溢出? 查看地址似乎不会,stl容器数据在堆。
class jass_syntax {
	typedef std::vector<const char*> WordLine;
	typedef std::vector<WordLine> WordList;
	typedef std::vector<jass_node> NodeList;
	typedef std::map<std::string,jass_node> NodeMap;

	jass_analys_scaner _scaner;
	NodeList _nodes;

	const char* _pos;

	WordLine* _line;
	jass_node* _node;
	size_t _line_id;
	size_t _line_count ;
	size_t _linenode;
	size_t _linenode_count;

	NodeMap _globals;
	NodeMap _locval;

	/*void initline() {
		_nodes.clear();
		line_id = 0;
		line_count = _scaner.wordlist.size();
		line = NULL;
		_it = _scaner.wordlist.begin();
		linenode = 0;
		linenode_count = 0;
		_pos = nullptr;
		if (_it!= _scaner.wordlist.end()) {
			line = &(*_it);
			linenode_count = line->size();
			linenode=0;
			_pos = line->at(linenode);
		}
	}
	void nextline() {
		line = NULL;
		linenode_count = 0;
		linenode = 0;
		_pos = nullptr;
		if (_it == _scaner.wordlist.end()) {
			return;
		}
		if (++_it != _scaner.wordlist.end()) {
			line = &(*_it);
			linenode_count = line->size();
			linenode = 0;
			_pos = line->at(linenode);
		}
	}
	void nextword() {
		if (linenode+1 >= linenode_count) {
			nextline();
			return;
		}
		if (++linenode < linenode_count) {
			_pos = line->at(linenode);
		}
	}*/
	void reset() {
		_pos = 0;
		_nodes.clear();
		_line = 0;
		_node = 0;
		_line_id = 0;
		_line_count = 0;
		_linenode = 0;
		_linenode_count=0;

		_globals.clear();
		_locval.clear();
	}
	void analys() {
		//initline();

		reset();

		for (_line_id = 0; _line_id < _line_count; ++_line_id) {
			_line = &_scaner.wordlist[_line_id];
			size_t word_id = 0;
			size_t word_count = _line->size();
			for (word_id = 0; word_id < word_count; ++word_id) {
				_pos = _line->at(word_id);
				switch (symbols.get_type(_pos[0]))
				{
				case syntax_symbols::null:
					break;
				case syntax_symbols::word:
					if (strcmp(_pos, "globals") == 0) {
						_nodes.push_back(jass_node(NULL, _pos));
						_node = &_nodes.back();
						word_id = word_count;
					}
					break;
				case syntax_symbols::symbol:
					if (_pos[0] =='/' && (_pos[1]=='/' || _pos[1]=='*') ) {
						break;
					}
					if (strcmp(_pos, "endglobals") == 0) {
						_nodes.push_back(jass_node(_nodes.back()._last, _pos));
						_node = NULL;
					}
					break;
				case syntax_symbols::other:
					break;

				}
			}
		}
	}
};