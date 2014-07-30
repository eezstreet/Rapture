#pragma once
#include "Entity.h"

typedef void(*OptionCallback)(Entity* us, Entity* interacter);
typedef pair<const char*, OptionCallback> Option;
typedef vector<Option> OptionList;

void InsertOption(OptionList& in, const char* name, OptionCallback callback);