#pragma once

#include <text/simple_analys.h>
#include <text/jassmap.h>
#include <vector>


//debug: 2MB��Сjass�ļ�,�ʷ��ֽ���ʱ5~6s,ռ���ڴ�Լ60MB��
//release: 2MB��Сjass�ļ�,�ʷ��ֽ���ʱ63~94ms,ռ���ڴ�Լ17MB��

typedef std::vector<std::string> WordLine;
typedef std::vector<WordLine> WordList;



struct jass_args {
	std::string _type;
	std::string _name;
	std::string _value;
};
struct jass_variable {
	std::string _type;
	std::string _name;
	size_t _array;
	std::vector<std::string> _values;
};

typedef std::vector<jass_args> jass_argslist;
typedef std::vector<jass_variable> jass_varlist;

struct jass_function {
	jass_argslist _argslist;
	jass_varlist _varlist;
	
};


class jass_library {
	std::string _name;
	std::string _initializer;
	WordLine _requires;
	bool _uses;
	bool _needs;
	bool _optional;
	jass_map _globals;
	jass_map _functions;
};

class jass_context {
	WordList& wordlist;
	jass_map& globals;
	jass_map& nativefunctions;
	jass_map& functions;

};


class jass_analys2:public simple_analys {
private:
	int _globals_count;
	int _nativefunctions_count;
	int _functions_count;
public:
	WordList wordlist;

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
	//static char chars[] = "(){}[],^~;%$&#@";


	void next_lua_comment(char* &str,char* end, BOOL skipsymbol) {
		bool lua_comment_mltstart = false;
		int lua_comment_mlt = 0;
		int lua_comment_mlttype = 0;
		int lua_comment_n = 2;
		int lua_comment_level = 0;
		char*str2 = str + 2;
		bool _EOF = false;

		if (!str || (end-str) < 2) {
			return;
		}
		//ydwe_lua_import����ע��  (" import 'main.lua'[[ ]] ")
		if (str[0]!='-' || str[1] != '-') {
			return;
		}
		std::string dbgstr("");
		for (;!_EOF && str2 < end && *str2; ++str2) {
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
				if (lua_comment_mlt ==1)
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
				if (lua_comment_n ==2 && lua_comment_mlt < 2 && (lua_comment_mlttype == 0 || lua_comment_mlttype == 1)) {
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
					if (lua_comment_mlt == 2 && (lua_comment_level>=0 || !lua_comment_mltstart)) {
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
				//����ydwe�Ĵ�����:����ע�͵���?>��Ч,����ע������Ч��
				if (str2[1]=='>' && !lua_comment_mltstart) {
					str2 -= 1;
					_EOF = true;
				}
			default:
				lua_comment_mlt = 0;
				break;
			}
		}
		str = str2 ;
	
	}

	std::string get_line_text(size_t i) {
		std::string str = "";
		if (i< wordlist.size()) {
			for (auto it = wordlist[i].begin(); it != wordlist[i].end();++it) {
				str.append((*it).data());
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
		// table ���� ��� if ��������
		static char utf8textflag[4] = {0xEF,0xBB,0xBF,0};
		char lpchar[4];
		char pchar[4] = { 0 };
		char* str1 = nullptr;
		char* start = 0;
		char* word_start = 0;
		char* word_pos = 0;
		int numlen = 0;
		int numbit = 0;
		bool lua_range = false;
		bool is_next_newline = false;
		if (_end-_pos>3) {
			if (strncmp(utf8textflag, _pos, 3) == 0) {
				_pos += 3;
			}
		}
		wordlist.push_back(WordLine());
		WordLine* line = &wordlist.back();
		std::string dbgstr;
		start = _pos;
		for (; _pos < _end && *_pos ; ++_pos) {
			lpchar[0] = pchar[1]; lpchar[1] = pchar[1]; lpchar[2] = pchar[2];
			pchar[0] = *_pos; pchar[1] = 0; pchar[2] = 0; pchar[3] = 0;
			switch (*_pos)
			{
			case '\t':
			case ' ':
				if (line->size() > 0) {
					line->push_back(pchar);
				}
				for (; _pos < _end && (_pos[1]=='\t'|| _pos[1] == ' '); ++_pos) {}
				break;
			case '\n':
				//printf("%d: %s\n", wordlist.size(), get_line_text(wordlist.size() - 1).data());
				wordlist.push_back(WordLine());
				line = &wordlist.back();
				//�����Ƿ�����.
				//line = wordlist.back();
				break;

				//����.����һ�ַ�����Ϊ�����½�һ���ַ���.
			case '\v':
			case '\r':
				/*if (line->back().length()>0) {
					line->push_back("");
				}*/
				break;
				//��һ���ַ�������word
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
				if (word_pos == _pos - 1) {
					line->back().push_back(pchar[0]);
					word_pos = _pos; break;
				}
				numlen = get_number_len(_pos, &numbit);
				if (numlen > 0) {
					line->push_back(std::string(_pos, numlen));
					_pos = _pos + numlen - 1;
				}
				else {
					line->push_back(pchar);
				}
				break;
				//-- �Լ�, - -������
			case '-':
				if (lua_range && _pos[1] == '-') {
					str1 = _pos;
					next_lua_comment(_pos,_end,FALSE);
					line->push_back(std::string(str1, _pos - str1));
					--_pos; break;
				}
				if (_pos[1] == '-' || _pos[1] == '>' || _pos[1] == '=') {
					pchar[1] = _pos[1]; ++_pos; line->push_back(pchar); break;
				}
				numlen = get_number_len(_pos, &numbit);
				if (numlen > 1) {
					line->push_back(std::string(_pos, numlen));
					_pos = _pos + numlen - 1;
				}
				else {
					line->push_back(pchar);
				}
				break;
				//�������� ��.�Ĵ�����һ�¡�(�����Ϊget_number_len)
			case '.':
				numlen = get_float_len(_pos,255);
				if (numlen > 1) {
					line->push_back(std::string(_pos, numlen));
					_pos = _pos + numlen - 1;
				}
				else {
					if (lua_range && _pos[1] == '.') { pchar[1] = _pos[1]; ++_pos; }
					line->push_back(pchar); break;
				}
				break;
			case '*':
				if (_pos[1] == '/' || _pos[1] == '=') { pchar[1] = _pos[1]; ++_pos;}
				line->push_back(pchar); break;
			case '+':
				if (_pos[1] == '+' || _pos[1] == '=') { pchar[1] = _pos[1];  ++_pos; }
				line->push_back(pchar); break;
			case '>':
				if (_pos[1] == '>' || _pos[1] == '=') { pchar[1] = _pos[1]; ++_pos;}
				line->push_back(pchar); break;
			case '<':
				if (_pos[1] == '?') {
					pchar[1] = _pos[1]; ++_pos; lua_range = true;
					//printf("����lua��Χ\n");
				}
				if (_pos[1] == '<' || _pos[1] == '=') { pchar[1] = _pos[1];++_pos; }
				line->push_back(pchar); break;
			
			case '!':
				if (_pos[1] == '=') { pchar[1] = _pos[1]; ++_pos; }
				line->push_back(pchar); break;
			case '|':
				if (_pos[1] == '|') { pchar[1] = _pos[1]; ++_pos; }
				line->push_back(pchar); break;
			case '=':
				if (_pos[1] == '=') { pchar[1] = _pos[1]; ++_pos; }
				line->push_back(pchar); break;
			case '?':
				if (lua_range && _pos[1] == '>') {
					pchar[1] = _pos[1]; ++_pos;lua_range = false;
					//printf("�뿪lua��Χ\n");
				}
				line->push_back(pchar);break;
			case ':':
				if (lua_range && _pos[1] == ':') {pchar[1] = _pos[1]; ++_pos;}
				line->push_back(pchar);break;
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
			case '$':
			case '&':
			case '#':
			case '@':
				line->push_back(pchar);
				break;
				//δ֪
			case '`':
				break;
			case '/':
				str1 = _pos;
				if (_pos[1] == '/') {
					//�ı���
					if (_pos + 2 < _end) {
						if (_pos[2] == '!') {
							_pos += 2;
							line->push_back("//!");
							break;
						}
					}
					next_line(_pos, FALSE);
					line->push_back(std::string(str1, _pos - str1));
					--_pos;
					break;
				}
				else if (_pos[1] == '*') {
					next_comment(_pos);
					line->push_back(std::string(str1, _pos - str1));
					--_pos;
					break;
				}
				if (_pos[1] == '=') { pchar[1] = _pos[1]; ++_pos; }
				line->push_back(pchar);
				break;
				//ת���
			case '\\':
				//�Ƿ�������һ��,���ж�.���к�Ŀո��Զ�����.
				if (isspace(_pos[1])) {
					bool isexist_line_symbol = false;
					str1 = _pos + 1;
					for (; str1 < _end && isspace(*str1); ++str1) {
						if (*str1 == '\n') { isexist_line_symbol = true; break; }
					}
					if (isexist_line_symbol) {
						_pos = str1;
						pchar[1] = '\n';
						line->push_back("\\\n");
						break;
					}
				}
				++_pos;
				pchar[1] = *_pos;
				line->push_back(pchar);
				break;

				//�м���ַ�����4λ(A-Z,a-z,0-9)���ַ��򲻷���jass,luaΪ�ı���
			case '\'':
				if (!lua_range) {
					str1 = _pos; ++_pos; numlen = 0;
					for (; _pos < _end && numlen< 4 && *_pos; ++_pos) {
						if (*_pos == '\\') { ++_pos; numlen++; }
						else if (*_pos == '\'') { break; }
						else { numlen++; }
					}
					//����! ����4���ַ�
					if (*_pos!='\'') {
						printf("''������Χ!,%d��,%s\n", wordlist.size(), get_cur_line_string().c_str());
						return;
					}
					line->push_back(std::string(str1, _pos - str1 + 1));
					break;
				}
				// " ' ʹ����ͬ������,�Լ���lua.
			case '"':
				str1 = _pos;
				++_pos;
				is_next_newline = false;
				for (; _pos < _end && *_pos; ++_pos) {
					if (*_pos == '\\') {++_pos;}
					else if (*_pos == '\n') {is_next_newline = true;}
					else if (*_pos == pchar[0]) {break;}
				}
				//ѭ��������_pos+1,����_pos��ֱ��+1����һ���ַ�.
				line->push_back(std::string(str1, _pos - str1+1));
				if(is_next_newline){
					wordlist.push_back(WordLine());
					line = &wordlist.back(); 
				}
				break;
			default:
				if (is_word(pchar[0])) {
					if (word_pos < _pos - 1) {
						word_start = _pos;
						line->push_back(pchar);
					}
					else {
						line->back().push_back(pchar[0]);
					}
					word_pos = _pos;
				}
				else {
					//����!
					printf("����Ľ�����char(%d),%d��,%s\n\0",pchar[0], wordlist.size(),get_cur_line_string().c_str());
					return;
				}
				break;
			}
		}
		if (_pos>=_end) {
			printf("�ʷ��ֽ�:%d��.\n", wordlist.size());
		}
		
	}

};
