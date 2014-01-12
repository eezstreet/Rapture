#include "sys_shared.h"

// string functions
static vector<string>& split_x(const string& str, const char delim, vector<string>& elems) {
	string itm;
	stringstream ss(str);
	while(getline(ss, itm, delim))
		elems.push_back(itm);
	return elems;
}

vector<string>& split(const string& str, const char delim) {
	vector<string> elems;
	split_x(str, delim, elems);
	return elems;
}

static vector<wstring>& split_x(const wstring& str, const wchar_t delim, vector<wstring>& elems) {
	wstring itm;
	wstringstream ss(str);
	while(getline(ss, itm, delim))
		elems.push_back(itm);
	return elems;
}

vector<wstring>& split(const wstring& str, const wchar_t delim) {
	vector<wstring> elems;
	split_x(str, delim, elems);
	return elems;
}

bool atob(const string& str) {
	bool b;
	istringstream(str) >> b;
	if(!b)
		istringstream(str) >> boolalpha >> b;
	return b;
}

bool atob(const char* str) {
	bool b;
	istringstream(str) >> b;
	if(!b)
		istringstream(str) >> boolalpha >> b;
	return b;
}