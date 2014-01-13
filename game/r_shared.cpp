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

// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
string trim(const string& str, const string& trim) {
	const auto strBegin = str.find_first_not_of(trim);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(trim);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}