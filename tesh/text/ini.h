#pragma once
#include <string.h>
#include <list>
#include <file/fpfile.h>
#define TO_RGB(c) ((c & 0xFFFFFF) >> 16)+((c & 0xFFFF) >> 8 << 8)+((c & 0xFF) << 16)
#define TO_BGR(c) TO_RGB(c)

/*
r	打开一个已有的文本文件，允许读取文件。
w	打开一个文本文件，允许写入文件。如果文件不存在，则会创建一个新文件。在这里，您的程序会从文件的开头写入内容。如果文件存在，则该会被截断为零长度，重新写入。
a	打开一个文本文件，以追加模式写入文件。如果文件不存在，则会创建一个新文件。在这里，您的程序会在已有的文件内容中追加内容。
r+	打开一个文本文件，允许读写文件。
w+	打开一个文本文件，允许读写文件。如果文件已存在，则文件会被截断为零长度，如果文件不存在，则会创建一个新文件。
a+	打开一个文本文件，允许读写文件。如果文件不存在，则会创建一个新文件。读取会从文件的开头开始，写入则只能是追加模式。
如果处理的是二进制文件，则需使用下面的访问模式来取代上面的访问模式：

"rb", "wb", "ab", "rb+", "r+b", "wb+", "w+b", "ab+", "a+b"

*/

namespace ini {
	struct keyword {
	public:
		std::string name;
		std::string value;
		std::string comment;
		void set(const char* _value) {
			value = _value;
		}
		void set(int _value, int bit = 10) {
			char strtmp[24] = { 0 };
			switch (bit)
			{
			case 8:
				sprintf_s(strtmp, 24, "0%o", _value);
				break;
			case 16:
				sprintf_s(strtmp, 24, "0x%06X", _value);
				break;
			default:
				sprintf_s(strtmp, 24, "%d", _value);
				break;
			}
			value = strtmp;
		}
		void set(float _value) {
			char strtmp[32] = { 0 };
			sprintf_s(strtmp, 32, "%f", _value);
			value = strtmp;
		}
		void operator=(const char* _value) {
			value = _value;
		}
		void operator=(int _value) {
			set(_value, 10);
		}
		void operator=(uint32_t _value) {
			set(_value, 16);
		}
		void operator=(float _value) {
			set(_value);
		}
		const char* operator()() {
			return value.c_str();
		}
		keyword(const char* _name, const char* _value) {
			name = _name;
			value = _value;
		}
		keyword(const char* _name, int _value, int bit) {
			name = _name;
			set(_value, bit);
		}
		keyword(const char* _name, float _value) {
			name = _name;
			set(_value);
		}
		int get_int(int default_value=0);
		float get_float();
		int get_rgb(int default_value = 0);
		bool is_empty();
	};
	struct node {
	public:
		std::string front_comment;
		std::string name;
		std::string right_comment;
		std::list<keyword> keywords;
		node(const char* _name) {
			name = _name;
		}
		keyword* get(const char* _name) {
			if (!_name) {
				return NULL;
			}
			for (auto it = keywords.begin(); it != keywords.end(); ++it) {
				if (strcmp((*it).name.c_str(), _name) == 0) {
					return &(*it);
				}
			}
			return NULL;
		}
		keyword& add(const char* _name, const char* _value) {
			keyword* k = get(_name);
			if (k) {
				k->value = _value;
				return *k;
			}
			keywords.push_back(keyword(_name, _value));
			return keywords.back();
		}
		keyword& operator[](const char* _name) {
			keyword* k = get(_name);
			if (k) {
				return *k;
			}
			return add(_name, "");
		}
		void set(const char* _name, const char* _value) {
			if (!_name || !_value) {
				return;
			}
			for (auto it = keywords.begin(); it != keywords.end(); ++it) {
				if (strcmp((*it).name.c_str(), _name) == 0) {
					(*it).value = _value;
				}
			}
		}
		void set(const char* _name, int _value) {
			if (!_name) {
				return;
			}
			for (auto it = keywords.begin(); it != keywords.end(); ++it) {
				if (strcmp((*it).name.c_str(), _name) == 0) {
					(*it).set(_value, 10);
				}
			}
		}
		void set(const char* _name, float _value) {
			if (!_name) {
				return;
			}
			for (auto it = keywords.begin(); it != keywords.end(); ++it) {
				if (strcmp((*it).name.c_str(), _name) == 0) {
					(*it).set(_value);
				}
			}
		}
		size_t size() {
			return keywords.size();
		}

	};
	//std::list<ini_keyword>
	class ini :std::list<node> {
	private:
		std::list<node>::iterator _it;
		fpfile* _fp;
	public:
		std::string cur_path;
		std::string comment;

		size_t _size() { return size(); }
		std::list<node>::iterator _begin() { return begin(); }
		std::list<node>::iterator _end() { return end(); }

		void reset();
		node* get(const char* _name);
		keyword* get(const char* _name, const char* key_name);
		int get_int(const char* _name, const char* key_name,int default_value = 0);
		void set(const char* _name, const char* key_name, const char* key_value);
		void set(const char* _name, const char* key_name, uint32_t key_value);
		void set(const char* _name, const char* key_name, int key_value);
		void set(const char* _name, const char* key_name, float key_value);
		node* create_node(const char* name);
		//& 需引用,否则每次都将新建一个node.
		node& operator[](const char* _name) { return *create_node(_name); }

		void load(char* str);
		bool open(const char*  _FileName,bool _closefp=false);
		bool save(bool _closefp = false);
		void save(const char*  _FileName, bool _closefp = false);
		bool save(const char*  _FileName,ini &newini, bool _closefp = false);
		void close_fp();
		ini();
		~ini();
	};

	class config_t :public ini {
	public:
		ini _default;
		const char* get_str(const char* _name, const char* key_name);
		int get_int(const char* _name, const char* key_name, int default_value = 0);
		float get_float(const char* _name, const char* key_name);
		int get_rgb(const char* _name, const char* key_name, int default_value = 0);
		void save_config(const char*  _FileName);
	};

}

