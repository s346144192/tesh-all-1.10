#include <windows.h>
#include <file/fpfile.h>

#define MALLOC(size) VirtualAlloc(NULL,size,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE)
#define FREE(address) VirtualFree(address,0,MEM_RELEASE)

using namespace std;

fpfile::fpfile() {
	_size = 0;
	fp = NULL;
	buffer = NULL;
	_filename = NULL;
}
fpfile::fpfile(const char* _FileName, const char* _Mode) {
	
	_size = 0;
	buffer = NULL;
	fp = NULL;
	_filename = _FileName;
	if (!_FileName || !_Mode) {
		return;
	}
	fopen_s(&fp, _FileName, _Mode);
	if (!fp) {

		return ;
	}
}
fpfile::~fpfile() {
	close();
}
bool fpfile::open(const char * _FileName, const char* _Mode) {
	if (fp) {
		free_buffer();
		fclose(fp);
		fp = NULL;
	}
	_filename = _FileName;
	if (!_FileName || !_Mode) {
		return false;
	}
	fopen_s(&fp, _FileName, _Mode);
	if (!fp) {

		return false;
	}
	return true;
}
bool fpfile::reopen(const char* _Mode) {
	if (!_Mode) {
		return false;
	}
	if (!_filename) {
		return false;
	}
	return open(_filename, _Mode);
}
bool fpfile::empty() {
	return !fp;
}
const char* fpfile::get_filename() {
	return _filename;
}
size_t fpfile::size() {
	if (!fp) {
		return 0;
	}
	if(_size != 0) {
		return _size;
	}
	fseek(fp, 0L, SEEK_END);
	_size = ftell(fp);//获取文件长度.
	rewind(fp);
	return _size;
}
int fpfile::read_buffer() {
	if (buffer != NULL) {
		free_buffer();
	}
	if (!fp) {
		return 0;
	}
	if (_size == 0) {
		size();
	}
	buffer = (char*)malloc(_size+1);
	rewind(fp);
	size_t length =fread(buffer, 1, _size,fp);
	if (length < _size) {
		memset(buffer+ length, 0, _size- length);
	}
	buffer[_size] = 0;
	return length;

}
void fpfile::free_buffer() {
	_size = 0;
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
}
int fpfile::puts(const char* str) {
	if (!str) {
		return 0;
	}
	if (fp) {
		return fputs(str,fp);
	}
	return 0;
}
void fpfile::close() {
	_size = 0; 
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	_filename = NULL;
	free_buffer();
}