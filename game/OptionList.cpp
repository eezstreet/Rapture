#include "OptionList.h"

void InsertOption(OptionList& in, const char* name, OptionCallback callback) {
	Option o = make_pair(name, callback);
	in.push_back(o);
}