#include "sys_shared.h"

const string keycodeNames[] = {
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"Return",
	"Esc",
	"Backspace",
	"Tab",
	"Space",
	"-",
	"=",
	"[",
	"]",
	"\\",
	"#",
	";",
	"'",
	"`",
	",",
	".",
	"/",
	"CapsLock",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"PrintScrn",
	"ScrollLock",
	"Pause",
	"Insert",
	"Home",
	"PageUp",
	"Delete",
	"End",
	"PageDown",
	"Right",
	"Left",
	"Down",
	"Up",
	"NumLockClear",
	"KP Divide",
	"KP Multiply",
	"KP Minus",
	"KP Plus",
	"KP Enter",
	"KP 1",
	"KP 2",
	"KP 3",
	"KP 4",
	"KP 5",
	"KP 6",
	"KP 7",
	"KP 8",
	"KP 9",
	"KP 0",
	"KP .",
	"\\",
	"Application",
	"Power",
	"KP =",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"Execute",
	"Help",
	"Menu",
	"Select",
	"Stop",
	"Again",
	"Undo",
	"Cut",
	"Copy",
	"Paste",
	"Find",
	"Mute",
	"Volume Up",
	"Volume Down",
	"Locking Caps Lock",
	"Locking Num Lock",
	"Locking Scroll Lock",
	"KP ,",
	"KP = (AS400)",
	"INTNL 1",
	"INTNL 2",
	"INTNL 3",
	"INTNL 4",
	"INTNL 5",
	"INTNL 6",
	"INTNL 7",
	"INTNL 8",
	"INTNL 9",
	"LANG 1",
	"LANG 2",
	"LANG 3",
	"LANG 4",
	"LANG 5",
	"LANG 6",
	"LANG 7",
	"LANG 8",
	"LANG 9",
	"Alt Erase",
	"SysReq",
	"Cancel",
	"Clear",
	"Prior",
	"Return",
	"Separator",
	"Out",
	"Oper",
	"Clear Again",
	"CrSel",
	"ExSel",
	"KP 00",
	"KP 000",
	"Thousands Separator",
	"Decimal Separator",
	"Currency Unit",
	"Currency SubUnit",
	"KP (",
	"KP )",
	"KP {",
	"KP }",
	"KP Tab",
	"KP Backspace",
	"KP A",
	"KP B",
	"KP C",
	"KP D",
	"KP E",
	"KP F",
	"KP XOR",
	"KP Power",
	"KP %",
	"KP <",
	"KP >",
	"KP &",
	"KP &&",
	"KP |",
	"KP ||",
	"KP :",
	"KP #",
	"KP Space",
	"KP @",
	"KP !",
	"KP Mem Store",
	"KP Mem Recall",
	"KP Mem Clear",
	"KP Mem Add",
	"KP Mem Subtract",
	"KP Mem Multiply",
	"KP Mem Divide",
	"KP +/-",
	"KP Clear",
	"KP Clear Entry",
	"KP Binary",
	"KP Octal",
	"KP Decimal",
	"KP Hex",
	"Left Ctrl",
	"Left Shift",
	"Left Alt",
	"Left GUI",
	"Right Ctrl",
	"Right Shift",
	"Right Alt",
	"Right GUI",
	"Mode",
	"Audio Next",
	"Audio Prev",
	"Audio Stop",
	"Audio Play",
	"Audio Mute",
	"Media Select",
	"WWW",
	"Mail",
	"Calculator",
	"Computer",
	"AC Search",
	"AC Home",
	"AC Back",
	"AC Forward",
	"AC Stop",
	"AC Refresh",
	"AC Bookmarks",
	"Brightness Down",
	"Brightness Up",
	"Display Switch",
	"KBDIllum Toggle",
	"KBDIllum Down",
	"KBDIllum Up",
	"Eject",
	"Sleep"
};

// string functions
vector<string>& split(const string& str, const char delim, vector<string>& elems) {
	string itm;
	stringstream ss(str);
	// Strip out \r
	string s = "";
	while(getline(ss, itm, '\r')) {
		s += itm;
	}
	// ...and if our delimiter is not a newline, we need to strip that out too.
	ss = stringstream(s);
	if(delim != '\n') {
		s = "";
		while(getline(ss, itm, '\n')) {
			s += itm;
		}
		ss = stringstream(s);
	}
	while(getline(ss, itm, delim)) {
		elems.push_back(itm);
	}
	return elems;
}

static vector<wstring>& split_x(const wstring& str, const wchar_t delim, vector<wstring>& elems) {
	wstring itm;
	wstringstream ss(str);
	while(getline(ss, itm, delim)) {
		elems.push_back(itm);
	}
	return elems;
}

vector<wstring>& split(const wstring& str, const wchar_t delim) {
	vector<wstring> elems;
	split_x(str, delim, elems);
	return elems;
}

bool atob(const string& str) {
	bool b;
	istringstream(str) >> boolalpha >> b;
	return b;
}

bool atob(const char* str) {
	if(strlen(str) == 0) {
		return false;
	} else if(stricmp(str, "false") == 0) {
		return false;
	} else if(stricmp(str, "true") == 0) {
		return true;
	} else if(str[0] == '0') {
		return false;
	} else if(str[1] == '1') {
		return true;
	} else {
		return true;
	}
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
bool checkExtension (const string &fullString, const string &ending)
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

// http://stackoverflow.com/questions/6417817/easy-way-to-remove-extension-from-a-filename
string stripextension(const string& filename) {
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == string::npos) return filename;
	return filename.substr(0, lastdot); 
}