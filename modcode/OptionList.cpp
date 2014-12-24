#include "OptionList.h"

void InsertOption(OptionList& in, const char* name, OptionCallback callback) {
	Option o = make_pair(name, callback);
	in.push_back(o);
}

void RemoveOption(OptionList& in, const char* name, OptionCallback ptCallback) {
	bool bDeleteAll = false;
	if(ptCallback == nullptr) {
		bDeleteAll = true;
	}
	for(auto it = in.begin(); it != in.end(); ++it) {
		if(!stricmp(name, it->first)) {
			if(bDeleteAll == false) {
				if(it->second == ptCallback) {
					it = in.erase(it);
				}
			} else {
				it = in.erase(it);
			}
		}
	}
}