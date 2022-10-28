#include <windows.h>
#include <text/ini.h>

#include <text/simple_analys.h>

namespace ini {
	int keyword::get_int(int default_value) {
		simple_analys analys((char*)value.c_str());
		size_t len = 0;
		int i = analys.get_int(analys._pos, &len);
		if (len) {
			return i;
		}
		return default_value;
	}
	float keyword::get_float() {
		simple_analys analys((char*)value.c_str());
		return analys.get_float();
	}
	int keyword::get_rgb(int default_value) {
		if (value.length() == 0) {
			return default_value;
		}
		simple_analys analys((char*)value.c_str());
		bool issyscolor = false;
		if (strncmp(analys._pos, "system(", 7) == 0) {
			analys._pos += 7;
			issyscolor = true;
		}
		size_t ilen = 0;
		int i=analys.get_int(&ilen);
		if (ilen>0) {
			if (issyscolor) {
				return TO_BGR(GetSysColor(i));
			}
			return i;
		}
		//if (strcmp(analys._pos,"default")==0) {
		//	return -1;
		//}
		return default_value;
	}
	bool keyword::is_empty() {
		return (value.length() == 0);
	}
	void ini::reset() {
		for (_it = begin(); _it != end(); ++_it) {
			(*_it).keywords.clear();
		}
		clear();
	}
	node* ini::get(const char* _name) {
		for (auto it = begin(); it != end(); ++it) {
			if (strcmp((*it).name.c_str(), _name) == 0) {
				return &(*it);
			}
		}
		return NULL;
	}
	keyword* ini::get(const char* _name, const char* key_name) {
		node* _node = get(_name);
		if (!_node) {
			return NULL;
		}
		return _node->get(key_name);
	}
	int ini::get_int(const char* _name, const char* key_name,int default_value) {
		keyword* k = get(_name, key_name);
		if (!k) {
			return 0;
		}
		return k->get_int(default_value);
	}
	void ini::set(const char* _name, const char* key_name, const char* key_value) {
		node* _node = get(_name);
		if (!_node) {
			return;
		}
		_node->set(key_name, key_value);
	}
	void ini::set(const char* _name, const char* key_name, uint32_t key_value) {
		node* _node = get(_name);
		if (!_node) {
			return;
		}
		keyword* k = _node->get(key_name);
		if (!k) {
			return;
		}
		*k = key_value;
	}
	void ini::set(const char* _name, const char* key_name, int key_value) {
		node* _node = get(_name);
		if (!_node) {
			return;
		}
		_node->set(key_name, key_value);
	}
	void ini::set(const char* _name, const char* key_name, float key_value) {
		node* _node = get(_name);
		if (!_node) {
			return;
		}
		_node->set(key_name, key_value);
	}

	node* ini::create_node(const char* name) {
		node* _node = get(name);
		if (_node) {
			return _node;
		}
		push_back(node(name));
		return &back();
	}


	void ini::load(char* str) {
		reset();
		simple_analys analys(str);
		node* _node = NULL;
		char* comment_start = 0;
		char* comment_end = 0;
		for (; *analys._pos; ++analys._pos) {
			switch (*analys._pos) {
			case ';':
				if (!comment_start) {
					comment_start = analys._pos;
				}
				analys.next_line(TRUE);
			case '/':
			{
				char* cc = analys._pos;
				if (!comment_start && analys.skip_comment(FALSE)) {
					comment_start = cc;
				}
			}
			break;
			case '[': {
				comment_end = analys._pos - 1;
				analys.last_word(comment_end);
				char* end = analys.search_line_string("]");
				if (end) {
					char* right_comment_start = end + 1;
					char* right_comment = analys.search_text(right_comment_start, "/;\n");
					++analys._pos;
					analys.next_word();
					--end;
					analys.last_word(end);
					++end;
					char* node_name = analys.get_text(analys._pos, "\n]", end);
					if (node_name) {
						//printf("[%s]\n",node_name);
						_node = create_node(node_name);
						free(node_name);
						if (comment_start && comment_start < comment_end) {
							_node->front_comment.assign(comment_start, comment_end - comment_start);
						}
						comment_start = 0;
					}
					comment_end = right_comment;
					if (right_comment && analys.skip_comment(comment_end, FALSE)) {
						analys._pos = comment_end;
						if (node_name && _node && right_comment_start < comment_end) {
							_node->right_comment.assign(right_comment_start, comment_end - right_comment_start);
						}

					}
					else {
						analys.next_line(FALSE);
					}
				}
			}
					  break;
			case '=': {
				char* pos = analys._pos;
				analys.to_line_start(pos, analys._text);

				comment_end = pos - 1;
				analys.last_word(comment_end);

				analys.next_word(pos);
				if (analys.search_line_string(pos, "=") == analys._pos) {
					char* end = analys._pos;
					--end;
					analys.last_word(end);
					++end;
					char* name = analys.get_text(pos, "=", end);
					char c = analys.get_next_char(analys._pos, 1);
					if (c == ' ' || c == '\t') {
						++analys._pos;
						analys.next_word(analys._pos);
					}
					else {
						++analys._pos;
					}
					pos = analys._pos;
					char* text = analys.get_ini_line_s(pos);
					if (name) {
						if (!text) {
							text = (char*)malloc(1);
							text[0] = 0;
						}
						if (_node) {
							//printf("%s=%s\n", name,text);
							keyword& _keyword = (*_node).add(name, text);
							if (comment_start && comment_start < comment_end) {
								_keyword.comment.assign(comment_start, comment_end - comment_start);
							}
							comment_start = 0;
						}
					}
					free(name);
					free(text);
				}
			}
			}
		}
		comment_end = analys._pos - 1;
		if (comment_start && comment_start < comment_end) {
			comment.assign(comment_start, comment_end - comment_start);
		}
		comment_start = 0;

	}
	bool ini::open(const char*  _FileName, bool _closefp) {
		if (!_fp) {
			_fp = new fpfile();
		}
		_fp->open(_FileName, "r+");
		if (!_fp->read_buffer()) {
			return false;
		}
		cur_path = _FileName;
		load(_fp->buffer);
		if (_closefp) {
			close_fp();
		}
		return true;
	}
	bool ini::save(bool _closefp) {
		if (!_fp) { 
			if (cur_path.length()>0) {
				_fp = new fpfile();
				_fp->open(cur_path.data(), "w+");
				_fp->reopen("r+");
			}
		}
		else {
			_fp->reopen("w+");
			_fp->reopen("r+");
		}
		if (!_fp) {
			return false;
		}
		const char* node_name = 0;
		const char* name = 0;
		const char* text = 0;
		for (_it = begin(); _it != end(); ++_it) {
			//需&引用，否则可能创建且复制为新结构
			node& _node = (*_it);
			if (_node.front_comment.length() > 0) {
				fwrite(_node.front_comment.c_str(), 1, _node.front_comment.length(), _fp->fp);
				fputc('\n', _fp->fp);
			}
			fputc('[', _fp->fp);
			node_name = _node.name.c_str();
			fwrite(node_name, 1, strlen(node_name), _fp->fp);
			fputc(']', _fp->fp);
			if (_node.right_comment.length() > 0) {
				fwrite(_node.right_comment.c_str(), 1, _node.right_comment.length(), _fp->fp);
			}
			fputc('\n', _fp->fp);
			auto& keywords = _node.keywords;
			for (auto it2 = keywords.begin(); it2 != keywords.end(); ++it2) {
				keyword& _keyword = (*it2);
				if (_keyword.comment.length() > 0) {
					fwrite(_keyword.comment.c_str(), 1, _keyword.comment.length(), _fp->fp);
					fputc('\n', _fp->fp);
				}
				name = _keyword.name.c_str();
				text = _keyword.value.c_str();
				fwrite(name, 1, strlen(name), _fp->fp);
				fputc('=', _fp->fp);
				fwrite(text, 1, strlen(text), _fp->fp);
				fputc('\n', _fp->fp);
			}
		}
		if (comment.length() > 0) {
			fwrite(comment.c_str(), 1, comment.length(), _fp->fp);
			fputc('\n', _fp->fp);
		}
		if (_closefp) {
			close_fp();
		}
		return true;
	}
	void ini::save(const char*  _FileName, bool _closefp) {
		if (!_fp) {
			_fp = new fpfile();
		}
		cur_path = _FileName;
		_fp->open(_FileName, "w+");
		_fp->reopen("r+");
		save(_closefp);
	}
	bool ini::save(const char*  _FileName, ini &newini, bool _closefp) {
		if (!_fp) {
			_fp = new fpfile();
		}
		cur_path = _FileName;
		_fp->open(_FileName, "w+");
		_fp->reopen("r+");
		if (!_fp) {
			return false;
		}
		const char* node_name = 0;
		const char* name = 0;
		const char* text = 0;
		for (_it = begin(); _it != end(); ++_it) {
			//需&引用，否则可能创建且复制为新结构
			node& _node = (*_it);
			node* _node_new = newini.get(_node.name.data());
			node* _cur_node = &_node;
			if (_node_new) {
				_cur_node = _node_new;
			}
			if (_cur_node->front_comment.length() > 0) {
				fwrite(_cur_node->front_comment.c_str(), 1, _cur_node->front_comment.length(), _fp->fp);
				fputc('\n', _fp->fp);
			}
			fputc('[', _fp->fp);
			node_name = _node.name.c_str();
			fwrite(node_name, 1, strlen(node_name), _fp->fp);
			fputc(']', _fp->fp);
			if (_cur_node->right_comment.length() > 0) {
				fwrite(_cur_node->right_comment.c_str(), 1, _cur_node->right_comment.length(), _fp->fp);
			}
			fputc('\n', _fp->fp);
			auto& keywords = _node.keywords;
			for (auto it2 = keywords.begin(); it2 != keywords.end(); ++it2) {
				keyword& _keyword = (*it2);
				keyword* _keyword_new = newini.get(_node.name.data(), _keyword.name.data());
				keyword* _cur_keyword = &_keyword;
				if (_keyword_new) {
					_cur_keyword = _keyword_new;
				}
				if (_cur_keyword->comment.length() > 0) {
					fwrite(_cur_keyword->comment.c_str(), 1, _cur_keyword->comment.length(), _fp->fp);
					fputc('\n', _fp->fp);
				}
				name = _keyword.name.c_str();
				text = _cur_keyword->value.c_str();
				fwrite(name, 1, strlen(name), _fp->fp);
				fputc('=', _fp->fp);
				fwrite(text, 1, strlen(text), _fp->fp);
				fputc('\n', _fp->fp);
			}
		}
		if (newini.comment.length() > 0) {
			fwrite(newini.comment.c_str(), 1, newini.comment.length(), _fp->fp);
			fputc('\n', _fp->fp);
		}
		if (_closefp) {
			close_fp();
		}
		return true;


	}
	void ini::close_fp() {
		if (_fp) {
			_fp->close();
			delete _fp;
		}
		_fp = 0;
	}
	ini::ini() {
		_fp = 0;
	}
	ini::~ini() {
		if (_fp) {
			_fp->close();
			delete _fp;
		}
		_fp = 0;
	}

	const char* config_t::get_str(const char* _name, const char* key_name) {
		keyword* k=get(_name, key_name);
		const char* str = "";
		if (k) {
			str = k->value.data();
		}
		if (!strlen(str)) {
			k = _default.get(_name, key_name);
			if (k) {
				str = k->value.data();
			}
		}
		return str;
	}
	int config_t::get_int(const char* _name, const char* key_name, int default_value) {
		keyword* k = get(_name, key_name);
		if (k) {
			if (!k->is_empty()) {
				return k->get_int(default_value);
			}
		}
		k = _default.get(_name, key_name);
		if (k) {
			return k->get_int(default_value);
		}
		return default_value;
	}
	float config_t::get_float(const char* _name, const char* key_name) {
		keyword* k = get(_name, key_name);
		if (k) {
			if (!k->is_empty()) {
				return k->get_float();
			}
		}
		k = _default.get(_name, key_name);
		if (k) {
			return k->get_float();
		}
		return 0;
	}
	int config_t::get_rgb(const char* _name, const char* key_name, int default_value) {
		keyword* k = get(_name, key_name);
		if (k) {
			if (!k->is_empty()) {
				return k->get_rgb(default_value);
			}
		}
		k = _default.get(_name, key_name);
		if (k) {
			return k->get_rgb(default_value);
		}
		return default_value;
	}
	void config_t::save_config(const char*  _FileName) {
		_default.save(_FileName,*this,true);
	}

}


