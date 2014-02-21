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

const char* btoa(bool b) {
	if(b) return "true";
	return "false";
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

// http://stackoverflow.com/questions/12851379/c-how-do-i-convert-hex-integer-to-string
string hexstring(const int address) {
	std::stringstream ss;
	ss << address;
	return ss.str();
}

// http://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar- inspiration

void tostring(const wstring& in, string& out) {
	string temp(in.begin(), in.end());
	out = temp;
}

void towstring(const string& in, wstring& out) {
	wstring temp(in.begin(), in.end());
	out = temp;
}

// http://stackoverflow.com/questions/874134/find-if-string-endswith-another-string-in-c
bool checkExtension (string const &fullString, string const &ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

// http://stackoverflow.com/questions/10030626/replace-char-in-string-with-some-string-inplace
void stringreplace(string& fullString, const string& sequence, const string& replace) {
	size_t pos;
	while ((pos = fullString.find(sequence)) != string::npos) {
		fullString.replace(pos, sequence.length(), replace);
	}
}