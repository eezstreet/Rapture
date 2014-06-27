#include "sys_local.h"

/* CvarSystem */
void CvarSystem::Initialize() {
	init = true;
}

void CvarSystem::Destroy() {
	ArchiveCvars();
	cvars.clear();
	init = false;
}

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
	R_Printf("Caching cvar %s\n", sName.c_str());
}

void CvarSystem::Cache_Free(const string& sName) {
	auto it = cache.find(sName);
	if(it == cache.end())
		return; // FIXME: use the iterator for the next two calls (performance)
	Zone::FastFree(cache[sName], "cvar");
	cache.erase(sName);
}

void CvarSystem::ArchiveCvars() {
	File* ff = File::Open("raptureconfig.cfg", "w");
	if(!ff)
		return; // also should not happen
	for(auto it = cvars.begin(); it != cvars.end(); ++it) {
		Cvar* cv = it->second;
		if(cv->flags & (1 << Cvar::CVAR_ARCHIVE)) {
			stringstream ss;
			ss << "seta " << cv->name << " ";
			switch(cv->type) {
				case Cvar::CV_BOOLEAN:
					ss << btoa(cv->b.currentVal);
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
			ff->WritePlaintext(ss.str());
		}
	}
	ff->Close();
	Zone::FastFree(ff, "files");
}

string CvarSystem::GetStringValue(const string& sName) {
	if(!Cvar::Exists(sName)) { // use cache
		return ""; // FIXME
	}
	else {
		return Cvar::Get<char*>(sName, "", 0, "")->s.currentVal;
	}
}

int CvarSystem::GetIntegerValue(const string& sName) {
	if(!Cvar::Exists(sName)) { // use cache
		return 0; // FIXME
	}
	else {
		return Cvar::Get<int>(sName, "", 0, 0)->i.currentVal;
	}
}

float CvarSystem::GetFloatValue(const string& sName) {
	if(!Cvar::Exists(sName)) {
		return 0.0f; // FIXME
	}
	else {
		return Cvar::Get<float>(sName, "", 0, 0.0f)->v.currentVal;
	}
}

bool CvarSystem::GetBooleanValue(const string& sName) {
	if(!Cvar::Exists(sName)) {
		return false; // FIXME
	}
	else {
		return Cvar::Get<bool>(sName, "", 0, false)->b.currentVal;
	}
}

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

void CvarSystem::ListCvars() {
	bool bFoundCommand = true;
	for(string s = GetFirstCvar(bFoundCommand); bFoundCommand; s = GetNextCvar(s, bFoundCommand)) {
		R_Printf("%s\n", s.c_str());
	}
}

bool CvarSystem::ProcessCvarCommand(const string& sName, const vector<string>& VArguments) {
	auto cv = cvars.find(sName);
	if(cv == cvars.end()) {
		return false;
	}

	Cvar* cvar = cv->second;
	if(VArguments.size() == 1) {
		R_Printf("%s\n", cvar->description.c_str());
		switch(cvar->GetType()) {
			case Cvar::CV_BOOLEAN:
				{
					string sCurrentValue = cvar->Bool() ? "true" : "false";
					string sDefaultValue = cvar->DefaultBool() ? "true" : "false";
					R_Printf("%s is %s, default: %s\n", sName.c_str(), sCurrentValue.c_str(), sDefaultValue.c_str());
				}
				break;
			case Cvar::CV_FLOAT:
				R_Printf("%s is %f, default: %f\n", sName.c_str(), cvar->Value(), cvar->DefaultValue());
				break;
			case Cvar::CV_INTEGER:
				R_Printf("%s is %i, default: %i\n", sName.c_str(), cvar->Integer(), cvar->DefaultInteger());
				break;
			default:
			case Cvar::CV_STRING:
				R_Printf("%s is \"%s\", default: \"%s\"\n", sName.c_str(), cvar->String(), cvar->DefaultString());
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

void CvarSystem::EXPORT_BoolValue(const char* name, bool* value) {
	if(!Cvar::Exists(name)) {
		value = NULL;
		R_Printf("WARNING: cvar %s does not exist!\n", name);
		return;
	}
	if(value == NULL) {
		R_Printf("WARNING: gamecode passed 'null' to CvarSystem::BoolValue\n");
		return;
	}
	if(GetCvarType(name) != Cvar::CV_BOOLEAN) {
		R_Printf("WARNING: cvar %s is not boolean type!\n", name);
		return;
	}
	bool retVal = GetBooleanValue(name);
	*value = retVal;
}

void CvarSystem::EXPORT_IntValue(const char* name, int* value) {
	if(!Cvar::Exists(name)) {
		value = NULL;
		R_Printf("WARNING: cvar %s does not exist!\n", name);
		return;
	}
	if(value == NULL) {
		R_Printf("WARNING: gamecode passed 'null' to CvarSystem::IntValue\n");
		return;
	}
	if(GetCvarType(name) != Cvar::CV_INTEGER) {
		R_Printf("WARNING: cvar %s is not integral type!\n", name);
		return;
	}
	int retVal = GetIntegerValue(name);
	*value = retVal;
}

void CvarSystem::EXPORT_StrValue(const char* name, char* value) {
	if(!Cvar::Exists(name)) {
		value = NULL;
		R_Printf("WARNING: cvar %s does not exist!\n", name);
		return;
	}
	if(value == NULL) {
		R_Printf("WARNING: gamecode passed 'null' to CvarSystem::StrValue\n");
		return;
	}
	if(GetCvarType(name) != Cvar::CV_STRING) {
		R_Printf("WARNING: cvar %s is not a string!\n", name);
		return;
	}
	string retVal = GetStringValue(name);
	strcpy(value, retVal.c_str());
}

void CvarSystem::EXPORT_Value(const char* name, float* value) {
	if(!Cvar::Exists(name)) {
		value = NULL;
		R_Printf("WARNING: cvar %s does not exist!\n", name);
		return;
	}
	if(value == NULL) {
		R_Printf("WARNING: gamecode passed 'null' to CvarSystem::Value\n");
		return;
	}
	if(GetCvarType(name) != Cvar::CV_FLOAT) {
		R_Printf("WARNING: cvar %s is not floating point type!\n", name);
		return;
	}
	float retVal = GetFloatValue(name);
	*value = retVal;
}

bool CvarSystem::init = false;
unordered_map<string, Cvar*> CvarSystem::cvars;
unordered_map<string, CvarCacheObject*> CvarSystem::cache;

