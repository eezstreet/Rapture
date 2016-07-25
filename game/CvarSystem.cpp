#include "sys_local.h"

/*
 * The CvarSystem is a global class (FIXME: should be namespace?) that is in charge of managing all of our Cvars.
 * FIXME: Should be named CvarManager?
 */

// Registers a Cvar from a Cvar object.
// If there is a registered Cvar, it won't register two of them.
Cvar* CvarSystem::RegisterCvar(Cvar *cvar) {
	auto it = cvars.find(cvar->name);
	if(it != cvars.end() && it->second && it->second->registered) {
		return it->second;
	}
	cvar->registered = true;
	cvars[cvar->name] = cvar;
	Cmd::AddTabCompletion(cvar->name);
	return cvar;
}

// Registers a Cvar by creating a new Cvar object (a string-based one).
Cvar* CvarSystem::RegisterCvar(const char* sName, const char* sDesc, int iFlags, char* startValue) {
	auto it = cvars.find(sName);
	if(it != cvars.end()) {
		return it->second;
	}

	Cvar* cvar = Zone::New<Cvar>(Zone::TAG_CVAR);
	cvar->name = sName;
	cvar->description = sDesc;
	cvar->type = Cvar::CV_STRING;

	auto it2 = cache.find(sName); // Check the cache.
	if(it2 != cache.end()) {
		// use from the cache
		if(it2->second->archive) {
			cvar->flags = iFlags | (1 << CVAR_ARCHIVE);
		} else {
			cvar->flags = iFlags;
		}
		cvar->AssignHardValue((char*)it2->second->initvalue.c_str());
		Cache_Free(sName);
	} else {
		// register a new cvar altogether
		cvar->flags = iFlags;
		cvar->AssignHardValue(startValue);
	}

	cvar->registered = true;
	return RegisterCvar(cvar);
}

// Registers a Cvar by creating a new Cvar object (an integer-based one)
Cvar* CvarSystem::RegisterCvar(const char* sName, const char* sDesc, int iFlags, int startValue) {
	auto it = cvars.find(sName);
	if(it != cvars.end()) {
		return it->second;
	}

	Cvar* cvar = Zone::New<Cvar>(Zone::TAG_CVAR);
	cvar->name = sName;
	cvar->description = sDesc;
	cvar->type = Cvar::CV_INTEGER;

	auto it2 = cache.find(sName); // Check the cache.
	if(it2 != cache.end()) {
		// use from the cache
		if(it2->second->archive) {
			cvar->flags = iFlags | (1 << CVAR_ARCHIVE);
		} else {
			cvar->flags = iFlags;
		}
		cvar->AssignHardValue(atoi(it2->second->initvalue.c_str()));
		Cache_Free(sName);
	} else {
		// register a new cvar altogether
		cvar->flags = iFlags;
		cvar->AssignHardValue(startValue);
	}

	cvar->registered = true;
	return RegisterCvar(cvar);
}

// Registers a new Cvar by creating a new Cvar object (a floating point one)
Cvar* CvarSystem::RegisterCvar(const char* sName, const char* sDesc, int iFlags, float startValue) {
	// TODO: generalize most of this code into one func
	auto it = cvars.find(sName);
	if(it != cvars.end()) {
		return it->second;
	}

	Cvar* cvar = Zone::New<Cvar>(Zone::TAG_CVAR);
	cvar->name = sName;
	cvar->description = sDesc;
	cvar->type = Cvar::CV_FLOAT;

	auto it2 = cache.find(sName); // Check the cache.
	if(it2 != cache.end()) {
		// use from the cache
		if(it2->second->archive) {
			cvar->flags = iFlags | (1 << CVAR_ARCHIVE);
		} else {
			cvar->flags = iFlags;
		}
		cvar->AssignHardValue((float)atof(it2->second->initvalue.c_str()));
		Cache_Free(sName);
	} else {
		// register a new cvar altogether
		cvar->flags = iFlags;
		cvar->AssignHardValue(startValue);
	}
	cvar->registered = true;
	return RegisterCvar(cvar);
}

// Register a boolean Cvar by creating a new Cvar object.
Cvar* CvarSystem::RegisterCvar(const char* sName, const char* sDesc, int iFlags, bool startValue) {
	auto it = cvars.find(sName);
	if(it != cvars.end()) {
		return it->second;
	}
	Cvar* cvar = Zone::New<Cvar>(Zone::TAG_CVAR);
	cvar->name = sName;
	cvar->description = sDesc;
	cvar->type = Cvar::CV_BOOLEAN;


	auto it2 = cache.find(sName); // Check the cache.
	if(it2 != cache.end()) {
		// use from the cache
		if(it2->second->archive) {
			cvar->flags = iFlags | (1 << CVAR_ARCHIVE);
		} else {
			cvar->flags = iFlags;
		}
		cvar->AssignHardValue(atob(it2->second->initvalue));
		Cache_Free(sName);
	} else {
		// register a new cvar altogether
		cvar->flags = iFlags;
		cvar->AssignHardValue(startValue);
	}


	cvar->registered = true;
	return RegisterCvar(cvar);
}

// FIXME: remove
void CvarSystem::Initialize() {
	init = true;
}

// Shuts down the Cvar manager by archiving any cvars flagged with CVAR_ARCHIVE and clearing the cvar cache.
void CvarSystem::Destroy() {
	ArchiveCvars();
	cvars.clear();
	init = false;
}

// Caches a Cvar to be created later.
// Cvars are only cached in one place: the command-line.
void CvarSystem::CacheCvar(const string& sName, const string& sValue, bool bArchive) {
	// check and make sure the cvar isn't already cached
	auto it = cache.find(sName);
	if(it == cache.end()) {
		CvarCacheObject* cv = Zone::New<CvarCacheObject>(Zone::TAG_CVAR);
		cv->initvalue = sValue;
		cv->archive = bArchive;
		cache[sName] = cv;
	}
	else {
		cache[sName]->archive = bArchive;
		cache[sName]->initvalue = sValue;
	}
	R_Message(PRIORITY_NOTE, "Caching cvar %s\n", sName.c_str());
}

// Removes a cvar from the cache.
void CvarSystem::Cache_Free(const string& sName) {
	auto it = cache.find(sName);
	if(it == cache.end())
		return; // FIXME: use the iterator for the next two calls (performance)
	Zone::FastFree(cache[sName], "cvar");
	cache.erase(sName);
}

// Creates the configuration file which is loaded later.
void CvarSystem::ArchiveCvars() {
	File* ff = File::OpenSync("raptureconfig.cfg", "w");
	for(auto it = cvars.begin(); it != cvars.end(); ++it) {
		Cvar* cv = it->second;
		if(cv->flags & (1 << CVAR_ARCHIVE)) {
			stringstream ss;
			ss << "seta " << cv->name << " ";
			switch(cv->type) {
				case Cvar::CV_BOOLEAN:
					ss << boolalpha << cv->b.currentVal;
					break;
				case Cvar::CV_FLOAT:
					ss << cv->v.currentVal;
					break;
				case Cvar::CV_INTEGER:
					ss << cv->i.currentVal;
					break;
				case Cvar::CV_STRING:
					ss << '\"' << cv->s.currentVal << '\"';
					break;
			}
			ss << ";\r\n";
			File::WriteSync(ff, (void*)ss.str().c_str(), ss.str().length());
		}
	}
	File::CloseSync(ff);
}

// Returns the string value of a Cvar.
string CvarSystem::GetStringValue(const char* sName) {
	if(!Cvar::Exists(sName)) { // use cache
		return ""; // FIXME
	}
	else {
		return Cvar::Get<char*>(sName, "", 0, "")->s.currentVal;
	}
}

// Returns the integer value of a cvar.
int CvarSystem::GetIntegerValue(const char* sName) {
	if(!Cvar::Exists(sName)) { // use cache
		return 0; // FIXME
	}
	else {
		return Cvar::Get<int>(sName, "", 0, 0)->i.currentVal;
	}
}

// Returns the floating point value of a cvar.
float CvarSystem::GetFloatValue(const char* sName) {
	if(!Cvar::Exists(sName)) {
		return 0.0f; // FIXME
	}
	else {
		return Cvar::Get<float>(sName, "", 0, 0.0f)->v.currentVal;
	}
}

// Returns the boolean value of a cvar.
bool CvarSystem::GetBooleanValue(const char* sName) {
	if(!Cvar::Exists(sName)) {
		return false; // FIXME
	}
	else {
		return Cvar::Get<bool>(sName, "", 0, false)->b.currentVal;
	}
}

// Retrieves the first registered Cvar.
// Used by list command
string CvarSystem::GetFirstCvar(bool& bFoundCommand) {
	auto it = cvars.begin();
	if(it == cvars.end()) {
		bFoundCommand = false;
		return "";
	}
	bFoundCommand = true;
	return it->first;
}

// Retrieves the next registered cvar.
string CvarSystem::GetNextCvar(const string& previous, bool& bFoundCommand) {
	auto it = cvars.find(previous);
	if(it == cvars.end()) {
		bFoundCommand = false;
		return "";
	}
	++it;
	if(it == cvars.end()) {
		bFoundCommand = false;
		return "";
	}
	bFoundCommand = true;
	return it->first;
}

// Sorting function for the cvars
bool CvarSystem::CompareCvars(pair<string, Cvar*> pFirst, pair<string, Cvar*> pSecond) {
	return pFirst.first.compare(pSecond.first) < 0;
}

// Prints out a list of all of the registered Cvars.
void CvarSystem::ListCvars() {
	bool bFoundCommand = true;
	vector<pair<string, Cvar*>> vValues(cvars.begin(), cvars.end());
	sort(vValues.begin(), vValues.end(), CvarSystem::CompareCvars);

	int maxNameSize = 0;	// Size of the name
	int maxSize = 6;		// Floating point = ##.### = 6 characters

	// We need to do a pass here to determine the longest cvar value length
	for (auto value = vValues.begin(); value != vValues.end(); ++value) {
		size_t namelen = value->first.length();
		if (namelen > maxNameSize) {
			maxNameSize = namelen;
		}

		if (value->second->GetType() == Cvar::CV_STRING) {
			size_t len = strlen(value->second->String());
			if (maxSize < len) {
				maxSize = len;
			}
		}
	}

	if (maxNameSize <= 0) {
		R_Message(PRIORITY_MESSAGE, "No cvars found.\n");
		return;
	}

	for (auto value = vValues.begin(); value != vValues.end(); ++value) {
		switch (value->second->GetType()) {
			default:
				R_Message(PRIORITY_MESSAGE, "%-*s = %-*s | %s (unknown)\n", 
					maxNameSize, value->first.c_str(), maxSize, value->second->String(), value->second->GetDescription().c_str());
				break;
			case Cvar::CV_STRING:
				R_Message(PRIORITY_MESSAGE, "%-*s = %-*s | %s (string)\n",
					maxNameSize, value->first.c_str(), maxSize, value->second->String(), value->second->GetDescription().c_str());
				break;
			case Cvar::CV_INTEGER:
				R_Message(PRIORITY_MESSAGE, "%-*s = %-*i | %s (integer)\n",
					maxNameSize, value->first.c_str(), maxSize, value->second->Integer(), value->second->GetDescription().c_str());
				break;
			case Cvar::CV_FLOAT:
				R_Message(PRIORITY_MESSAGE, "%-*s = %-*.3f | %s (floating-point)\n",
					maxNameSize, value->first.c_str(), maxSize, value->second->Value(), value->second->GetDescription().c_str());
				break;
			case Cvar::CV_BOOLEAN:
				if (value->second->Bool()) {
					R_Message(PRIORITY_MESSAGE, "%-*s = %-*s | %s (boolean)\n",
						maxNameSize, value->first.c_str(), maxSize, "true", value->second->GetDescription().c_str());
				}
				else {
					R_Message(PRIORITY_MESSAGE, "%-*s = %-*s | %s (boolean)\n",
						maxNameSize, value->first.c_str(), maxSize, "false", value->second->GetDescription().c_str());
				}
				break;
		}
	}
	// Close it off with a nice little line at the bottom
	R_Message(PRIORITY_MESSAGE, "-------------------------------\n");
}

// This function is 
bool CvarSystem::ProcessCvarCommand(const string& sName, const vector<string>& VArguments) {
	auto cv = cvars.find(sName);
	if(cv == cvars.end()) {
		return false;
	}

	Cvar* cvar = cv->second;
	if(VArguments.size() == 1) {
		R_Message(PRIORITY_MESSAGE, "%s\n", cvar->description.c_str());
		switch(cvar->GetType()) {
			case Cvar::CV_BOOLEAN:
				{
					string sCurrentValue = cvar->Bool() ? "true" : "false";
					string sDefaultValue = cvar->DefaultBool() ? "true" : "false";
					R_Message(PRIORITY_MESSAGE, "%s is %s, default: %s\n", sName.c_str(), sCurrentValue.c_str(), sDefaultValue.c_str());
				}
				break;
			case Cvar::CV_FLOAT:
				R_Message(PRIORITY_MESSAGE, "%s is %f, default: %f\n", sName.c_str(), cvar->Value(), cvar->DefaultValue());
				break;
			case Cvar::CV_INTEGER:
				R_Message(PRIORITY_MESSAGE, "%s is %i, default: %i\n", sName.c_str(), cvar->Integer(), cvar->DefaultInteger());
				break;
			default:
			case Cvar::CV_STRING:
				R_Message(PRIORITY_MESSAGE, "%s is \"%s\", default: \"%s\"\n", sName.c_str(), cvar->String(), cvar->DefaultString());
				break;
		}
	} else {
		switch(cvar->GetType()) {
			case Cvar::CV_BOOLEAN:
				cvar->SetValue(atob(VArguments[1]));
				break;
			case Cvar::CV_FLOAT:
				cvar->SetValue((float)atof(VArguments[1].c_str()));
				break;
			case Cvar::CV_INTEGER:
				cvar->SetValue(atoi(VArguments[1].c_str()));
				break;
			case Cvar::CV_STRING:
			default:
				cvar->SetValue((char*)VArguments[1].c_str());
				break;
		}
	}
	return true;
}

// Exported to the modcode
bool CvarSystem::EXPORT_BoolValue(Cvar* cvar, bool* value) {
	if (cvar == nullptr) {
		R_Message(PRIORITY_WARNING, "Tried to get nonexistent cvar value!\n");
		return false;
	}
	if(value == nullptr) {
		R_Message(PRIORITY_WARNING, "Modcode passed 'null' to CvarSystem::BoolValue\n");
		return false;
	}
	if (cvar->type != Cvar::CV_BOOLEAN) {
		R_Message(PRIORITY_WARNING, "%s is not boolean type\n", cvar->name.c_str());
		return false;
	}
	*value = cvar->b.currentVal;
	return *value;
}

// Exported to the modcode
int CvarSystem::EXPORT_IntValue(Cvar* cvar, int* value) {
	if (cvar == nullptr) {
		R_Message(PRIORITY_WARNING, "Tried to get nonexistent cvar value!\n");
		return -1;
	}
	if (value == nullptr) {
		R_Message(PRIORITY_WARNING, "Modcode passed 'null' to CvarSystem::IntValue\n");
		return -1;
	}
	if (cvar->type != Cvar::CV_INTEGER) {
		R_Message(PRIORITY_WARNING, "%s is not integral type\n", cvar->name.c_str());
		return -1;
	}
	*value = cvar->i.currentVal;
	return *value;
}

// Exported to the modcode
char* CvarSystem::EXPORT_StrValue(Cvar* cvar, char* value) {
	if (cvar == nullptr) {
		R_Message(PRIORITY_WARNING, "Tried to get nonexistent cvar value!\n");
		return "";
	}
	if (value == nullptr) {
		R_Message(PRIORITY_WARNING, "Modcode passed 'null' to CvarSystem::BoolValue\n");
		return "";
	}
	if (cvar->type != Cvar::CV_STRING) {
		R_Message(PRIORITY_WARNING, "%s is not string type\n", cvar->name.c_str());
		return "";
	}
	strcpy(value, cvar->s.currentVal);
	return value;
}

// Exported to the modcode
float CvarSystem::EXPORT_Value(Cvar* cvar, float* value) {
	if (cvar == nullptr) {
		R_Message(PRIORITY_WARNING, "Tried to get nonexistent cvar value!\n");
		return NAN;
	}
	if (value == nullptr) {
		R_Message(PRIORITY_WARNING, "Modcode passed 'null' to CvarSystem::Value\n");
		return NAN;
	}
	if (cvar->type != Cvar::CV_FLOAT) {
		R_Message(PRIORITY_WARNING, "%s is not floating point\n", cvar->name.c_str());
		return NAN;
	}
	*value = cvar->v.currentVal;
	return *value;
}

bool CvarSystem::init = false;
unordered_map<string, Cvar*> CvarSystem::cvars;
unordered_map<string, CvarCacheObject*> CvarSystem::cache;

