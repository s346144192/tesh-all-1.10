#pragma once
#include <stdio.h>

class fpfile {
private:
	size_t _size;
	const char* _filename;
public:
	fpfile();
	fpfile(const char*  _FileName, const char* _Mode);
	~fpfile();
	FILE* fp;
	char* buffer;
	bool open(const char* _FileName, const char* _Mode);
	bool reopen(const char* _Mode);
	bool empty();
	const char* get_filename();
	size_t size();
	int read_buffer();
	void free_buffer();
	int puts(const char* str);
	void close();
};