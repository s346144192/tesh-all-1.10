#pragma once

// mingw needs this to get a definition of _off_t

#include <string.h>
#include <map>
#include <functional>
#include <wtypes.h>
#include <file/fpfile.h>
#include <text/simple_analys.h>

extern const char* JassKeywords;
extern const char* JassKeywords2;
extern const char* JassKeywords3;
extern const char* JassTypes;
extern const char* JassCjKeywords;  //后期加载
extern const char* JassBjKeywords;//后期加载
extern const char* JassConstantKeywords;//后期加载
extern const char* LuaKeywords;
class jass_map {
private:
	typedef std::map<std::string, std::string> jass_map_t;
	typedef std::function<void(std::string, std::string)> callback_t;
public:
	jass_map_t data;
	jass_map_t::iterator it;

	jass_map_t::iterator begin() {
		return data.begin();
	}
	jass_map_t::iterator end() {
		return data.end();
	}
	jass_map_t::iterator find(std::string name) {
		return data.find(name);
	}
	void foreach(callback_t callback)
	{
		for (it = data.begin(); it != data.end(); it++) {
			callback((*it).first, (*it).second);
		}
	}
	void add(const char* name, const char* value) {
		//it = data.find(name);
		//if (it != data.end()) {
		//	(*it).second = value;
		//}
		//assign 才能有效赋值。
		data[name].assign(value);
	}
	std::string get(const char* name) {
		auto it = data.find(name);
		if (it != data.end()) {
			return (*it).second;
		}
		return "";
	}
};

class jass_analys:simple_analys {
private:
	int _globals_count ;
	int _nativefunctions_count;
	int _functions_count;
	
public:
	void put_globals(jass_map& globals)
	{
		if (test_word("globals")) {
			char name[260];
			char text[1024];
			char* textpos = 0;
			char* end = 0;
			char* mid = 0;
			int wlen = 0;
			_pos = _pos + sizeof("globals");
			char* p_endglobals = search_next_string("endglobals");
			if (p_endglobals) {
				for (;_pos < p_endglobals; ++_pos) {
					next_word(_pos);
					if (_pos >= p_endglobals) {
						break;
					}
					if (skip_comment(_pos,FALSE)) {
						continue;
					}
					end = _pos;
					int textlen= to_line_end(end, p_endglobals);
					if (textlen<=0) {
						next_line(_pos, FALSE);
						continue;
					}
					ZeroMemory(name, sizeof(name));
					ZeroMemory(text, sizeof(text));
					textpos = text;
					if (test_word("private")) {
						memcpy(textpos, "private ", 8); textpos += 8;
						next_word_s(_pos);
					}
					else if (test_word("public")) {
						memcpy(textpos, "public ", 7); textpos += 7;
						next_word_s(_pos);
					}
					if (test_word("constant")) {
						memcpy(textpos, "constant ", 9); textpos += 9;
						next_word_s(_pos);
					}
					wlen= get_word_len(_pos);
					if (wlen>0 && (textpos-text)+wlen<= textlen) {
						wlen++;
						memcpy(textpos, _pos, wlen); textpos += wlen;
					}
					next_word_s(_pos);
					if (test_word("array")) {
						memcpy(textpos, "array ", 6); textpos += 6;
						next_word_s(_pos);
					}
					wlen= get_word_len(_pos);
					memcpy(name, _pos, wlen);
					memcpy(textpos, _pos, wlen); textpos += wlen;
					mid=strchr(_pos,'=');
					if (mid>0 && mid< end) {
						*textpos = ' '; textpos++;
						*textpos = '='; textpos++;
						*textpos = ' '; textpos++;
						_pos = mid + 1;
						next_word(_pos);
						last_word(end);
						wlen = end - _pos;
						if (wlen>0) {
							memcpy(textpos, _pos, wlen); textpos += wlen;
						}
					}
					if (wlen>0) {
						name[sizeof(name) - 1] = 0;
						text[sizeof(text) - 1] = 0;
						globals.add(name, text);
						_globals_count++;
					}
					next_line(_pos, FALSE);
				}
				_pos = p_endglobals + sizeof("endglobals");
			}
		}
	}
	void put_function(jass_map& functions) {
		if (test_word("function")) {
			char* name = 0;
			char* text = 0;
			char* p_function = _pos;
			char* start=_pos;
			int space_width=0;
			to_line_start(start,_text);
			space_width = get_space_width(start);

			last_word_s(p_function);
			if (test_lastword(p_function,"public")) {
				p_function = p_function - sizeof("public") + 2;
			}
			else if (test_lastword(p_function, "private")) {
				p_function = p_function - sizeof("private") + 2;
			}
			else {
				p_function = _pos;
			}
			next_spacechar(_pos);
			next_word(_pos);
			name = get_word(_pos);

			char* p_endfunction = search_next_string("endfunction");
			_pos = _pos + sizeof("function");
			if (p_endfunction) {
				p_endfunction = p_endfunction + sizeof("endfunction");
				text = copy_newstring(p_function, p_endfunction - p_function);
				if (space_width>0) {
					char* cur_pos = text;
					next_line(cur_pos, false);
					std::string str("");
					if (cur_pos - text>0) {
						str.append(text, cur_pos - text);
					}
					int nwidth = 0;
					for (char ch=0; (ch = *cur_pos);) {
						if (ch=='\n' || ch == '\r') {
							str.push_back('\n');
							ch = *++cur_pos;
							if (ch == '\n' || ch == '\r') { ch = *++cur_pos; }
							start = cur_pos;
							nwidth = 0;
							for (; nwidth < space_width; ++start) {
								if (*start == ' ') {nwidth++;}
								else if (*start == '\t') {	nwidth += 4;}
								else {break;}
							}
							next_line(cur_pos,false);
							if (start< cur_pos) {
								str.append(start, cur_pos - start);
							}
						}
						else {
							++cur_pos;
						}
					}
					if (name && str.length()) {
						functions.add(name, str.data());
						_functions_count++;
					}
				}
				else if (name && text) {
					functions.add(name, text);
					_functions_count++;
				}
				_pos = p_endfunction; // 如更改为返回时+1;
			}
			free(name);
			free(text);
			name = 0;
			text = 0;
		}
	}
	void put_nativefunction(jass_map& nativefunctions) {
		if (test_word("native")) {
			int w_pos = 0;
			char word[260];
			char backword[260];
			char name[260];
			char* text= NULL;
			char* textpos = NULL;
			int textlen = 0;

			to_line_start();
			next_word();
			char* cc = _pos;
			char* end = _pos;
			to_line_end(end,_pos+strlen(_pos));
			textlen = (end - cc);
			if (textlen<=0) {
				next_line(_pos, FALSE);
				return;
			}
			text = (char*)malloc(textlen + 1);
			textpos = text;
			ZeroMemory(text, textlen + 1);
			ZeroMemory(word, sizeof(word));
			ZeroMemory(backword, sizeof(backword));
			ZeroMemory(name, sizeof(name));
			bool _EOF = false;
			for (;!_EOF && cc < _end  && w_pos < sizeof(word) && *cc; ++cc) {
				switch (*cc)
				{
				case '\t':
				case '\v':
				case ' ':
					if (w_pos) {
						memmove(textpos, word, w_pos);
						textpos += w_pos;
						if (!strcmp(backword, "native")) {
							strncpy_s(name, word, w_pos);
							*textpos = ' ';
							++textpos;
						}
						else if (!strcmp(backword, "returns")) {
							_EOF=true;
						}
						else {
							*textpos = ' ';
							++textpos;
						}
						strncpy_s(backword, word, w_pos);
						w_pos = 0;
					}
					break;
				case '\r':
				case '\n':
					if (w_pos) {
						memmove(textpos, word, w_pos);
						textpos += w_pos;
						strncpy_s(backword, word, w_pos);
						w_pos = 0;
					}
					_EOF = true;
					break;
				case '/':
					_EOF = true;
					/*if (cc[1] == '/') */
					if (cc[1] == '*') {
						next_comment(cc);
						if (cc < end) {
							_EOF = false;
						}
					}
					break;
				default:
					word[w_pos++] = *cc;
					break;
				}
			}
			if (w_pos) {
				memmove(textpos,word, w_pos);
				textpos += w_pos;
				w_pos = 0;
			}

			text[textlen] = 0;
			next_line(_pos,FALSE);

			if (*name && *text ) {
				nativefunctions.add(name, text);
				_nativefunctions_count++;
			}
			free(text);
			text = NULL;
		/*	free(name);
			free(text);
			name = 0;
			text = 0;*/
		}
	}
	void foreach(jass_map& globals, jass_map& nativefunctions, jass_map& functions) {
		for (; *_pos && _pos <= _end; ++_pos) {
			switch (*_pos)
			{
			case '\t':
			case ' ':
			case '\n':
			case '\r':
				break;
			case '/':
				skip_comment(_pos,FALSE);
				break;
			case 'g':
				put_globals(globals);
				break;
			case 'n':
				put_nativefunction(nativefunctions);
				break;
			case 'f':
				put_function(functions);
				break;
			default:
				break;
			}
		}

	}
	void load(char* jass_text, jass_map& globals, jass_map& nativefunctions, jass_map& functions) {
		_text = jass_text;
		_pos = jass_text;
		_end = 0;
		if (jass_text) {
			_end = _pos + strlen(jass_text);
		}
		_globals_count = 0;
		_nativefunctions_count = 0;
		_functions_count = 0;
		foreach(globals, nativefunctions, functions);
		_text = NULL;
		_pos = NULL;
		_end = NULL;
		//printf("globals_count:%d function_count:%d native_function_count:%d \n", _globals_count, _functions_count, _nativefunctions_count);

	}
	void load(char* jass_text, jass_map& globals, jass_map& functions) {
		load(jass_text, globals, functions, functions);
	}
	void load(char* jass_text, jass_map& all) {
		load(jass_text, all, all, all);
	}
	bool read(const char* path, jass_map& globals, jass_map& nativefunctions, jass_map& functions) {
		fpfile fp(path, "rt");
		if (!fp.fp) {
			return false;
		}
		fp.read_buffer();
		load(fp.buffer, globals, nativefunctions, functions);
		return true;
	}
	bool read(const char* path, jass_map& globals, jass_map& functions) {
		return read(path, globals, functions, functions);
	}
	bool read(const char* path, jass_map& all) {
		return read(path, all, all, all);
	}

	void operator()(char* jass_text, jass_map& globals, jass_map& nativefunctions, jass_map& functions) {
		load(jass_text, globals, nativefunctions, functions);
	}
	void operator()(char* jass_text, jass_map& globals, jass_map& functions) {
		load(jass_text, globals, functions, functions);
	}
	void operator()(char* jass_text, jass_map& all) {
		load(jass_text, all, all, all);
	}

	jass_analys() {
		_globals_count = 0;
		_nativefunctions_count = 0;
		_functions_count = 0;
		_text = 0;
		_pos = 0;
		_end = 0;
		//printf("globals_count:%d function_count:%d native_function_count:%d \n", globals_count, function_count, native_function_count);
	}
	jass_analys(char* jass_text, jass_map& globals, jass_map& nativefunctions, jass_map& functions) {
		load(jass_text,globals, nativefunctions, functions);
	}
	jass_analys(char* jass_text, jass_map& globals, jass_map& functions) {
		load(jass_text,globals, functions, functions);
	}
	jass_analys(char* jass_text, jass_map& all) {
		load(jass_text, all, all, all);
	}
	~jass_analys() {
		_globals_count = 0;
		_nativefunctions_count = 0;
		_functions_count = 0;
		_text = 0;
		_pos = 0;
		_end = 0;
	}
};

extern jass_map cj_text, bj_text, custom_function_text;
extern jass_map cj_globals_text, bj_globals_text, custom_globals_text;


static class jass_keywords {
private:
	char* _cj;
	char* _bj;
	char* _constant;
public:
	void clear() {
		if (_cj) { free(_cj); }
		if (_bj) { free(_bj); }
		if (_constant) { free(_constant); }
		_cj = nullptr;
		_bj = nullptr;
		_constant = nullptr;
		JassCjKeywords = "";
		JassBjKeywords = "";
		JassConstantKeywords = "";
	}
	void init() {

		clear();

		size_t maxlen = 0;
		size_t len = 0;
		char* pos=nullptr;
		//cj
		for (auto it = cj_text.begin(); it != cj_text.end();++it) {
			maxlen += (*it).first.length() + 1;
		}
		_cj = (char*)malloc(maxlen + 1);
		memset(_cj,0,maxlen+1);
		pos = _cj;
		for (auto it = cj_text.begin(); it != cj_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos,(*it).first.c_str(), len);
			pos[len] = ' ';
			pos+=static_cast<int>(len+1);
		}
		//bj+custom
		maxlen = 0;
		for (auto it = bj_text.begin(); it != bj_text.end(); ++it) {
			maxlen += (*it).first.length() + 1;
		}
		for (auto it = custom_function_text.begin(); it != custom_function_text.end(); ++it) {
			maxlen += (*it).first.length() + 1;
		}
		_bj = (char*)malloc(maxlen + 1);
		memset(_bj, 0, maxlen + 1);
		pos = _bj;
		for (auto it = bj_text.begin(); it != bj_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos, (*it).first.c_str(), len);
			pos[len] = ' ';
			pos += static_cast<int>(len + 1);
		}
		for (auto it = custom_function_text.begin(); it != custom_function_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos, (*it).first.c_str(), len);
			pos[len] = ' ';
			pos += static_cast<int>(len + 1);
		}
		//constant
		for (auto it = cj_globals_text.begin(); it != cj_globals_text.end(); ++it) {
			maxlen += (*it).first.length() + 1;
		}
		for (auto it = bj_globals_text.begin(); it != bj_globals_text.end(); ++it) {
			maxlen += (*it).first.length() + 1;
		}
		for (auto it = custom_globals_text.begin(); it != custom_globals_text.end(); ++it) {
			maxlen += (*it).first.length() + 1;
		}
		_constant = (char*)malloc(maxlen + 1);
		memset(_constant, 0, maxlen + 1);
		pos = _constant;
		for (auto it = cj_globals_text.begin(); it != cj_globals_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos, (*it).first.c_str(), len);
			pos[len] = ' ';
			pos += static_cast<int>(len + 1);
		}
		for (auto it = bj_globals_text.begin(); it != bj_globals_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos, (*it).first.c_str(), len);
			pos[len] = ' ';
			pos += static_cast<int>(len + 1);
		}
		for (auto it = custom_globals_text.begin(); it != custom_globals_text.end(); ++it) {
			len = (*it).first.length();
			memcpy(pos, (*it).first.c_str(), len);
			pos[len] = ' ';
			pos += static_cast<int>(len + 1);
		}

		JassCjKeywords = _cj;
		JassBjKeywords = _bj;
		JassConstantKeywords = _constant;
	}
	jass_keywords() {
		_cj = nullptr;
		_bj = nullptr;
		_constant = nullptr;
	}
	~jass_keywords() {
		clear();
	}
}JassFuncKeywords;
