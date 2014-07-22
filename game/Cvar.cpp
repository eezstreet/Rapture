#include "sys_local.h"

/* Cvar: the building block of a console variable (Cvar) */
Cvar::Cvar(Cvar&& other) {
}

Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, char* startValue) :
  name(sName),
  description(sDesc),
  type(Cvar::CV_STRING),
  flags(iFlags),
  ptsChangeCallback(NULL) {
	  s.AssignBoth(startValue);
}

Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, int startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_INTEGER),
flags(iFlags),
ptiChangeCallback(NULL) {
	i.AssignBoth(startValue);
}

Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, float startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_FLOAT),
flags(iFlags),
ptfChangeCallback(NULL) {
	v.AssignBoth(startValue);
}

Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, bool startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_BOOLEAN),
flags(iFlags),
ptbChangeCallback(NULL) {
	b.AssignBoth(startValue);
}

Cvar::Cvar() :
  flags(0),
  name(""),
  description(""),
  type(Cvar::CV_BOOLEAN),
  ptbChangeCallback(NULL) {
	b.AssignBoth(false);
}

Cvar::~Cvar() {
	// do we need something here?
}

Cvar& Cvar::operator= (char* str) {
	if(type != CV_STRING) return *this;
	strncpy(s.currentVal, str, sizeof(s.currentVal));
	return *this;
}

Cvar& Cvar::operator= (int val) {
	if(type != CV_INTEGER) return *this;
	i.currentVal = val;
	return *this;
}

Cvar& Cvar::operator= (float val) {
	if(type != CV_FLOAT) return *this;
	v.currentVal = val;
	return *this;
}

Cvar& Cvar::operator= (bool val) {
	if(type != CV_BOOLEAN) return *this;
	b.currentVal = val;
	return *this;
}

bool Cvar::Exists(const string& sName) {
	try {
		auto it = CvarSystem::cvars.find(sName);
		if(it == CvarSystem::cvars.end())
			return false;
		return true;
	}
	catch( out_of_range e ) {
		return false;
	}
}

/* CvarSystem: the major handler and dealing of cvars */
Cvar* CvarSystem::RegisterCvar(Cvar *cvar) {
	auto it = cvars.find(cvar->name);
	if(it != cvars.end() && it->second && it->second->registered) {
		return it->second;
	}
	cvar->registered = true;
	cvars[cvar->name] = cvar;
	return cvar;
}

Cvar* CvarSystem::RegisterCvar(const string& sName, const string& sDesc, int iFlags, char* startValue) {
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
			cvar->flags = iFlags | (1 << Cvar::CVAR_ARCHIVE);
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

Cvar* CvarSystem::RegisterCvar(const string& sName, const string& sDesc, int iFlags, int startValue) {
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
			cvar->flags = iFlags | (1 << Cvar::CVAR_ARCHIVE);
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

Cvar* CvarSystem::RegisterCvar(const string& sName, const string& sDesc, int iFlags, float startValue) {
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
			cvar->flags = iFlags | (1 << Cvar::CVAR_ARCHIVE);
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

Cvar* CvarSystem::RegisterCvar(const string& sName, const string& sDesc, int iFlags, bool startValue) {
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
			cvar->flags = iFlags | (1 << Cvar::CVAR_ARCHIVE);
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

void Cvar::AssignHardValue(char* value) { 
	s.AssignBoth(value); 
	if(ptsChangeCallback) 
		ptsChangeCallback(value); 
}

void Cvar::AssignHardValue(int value) {
	i.AssignBoth(value); 
	if(ptiChangeCallback) 
		ptiChangeCallback(value); 
}

void Cvar::AssignHardValue(float value) { 
	v.AssignBoth(value); 
	if(ptfChangeCallback) 
		ptfChangeCallback(value); 
}

void Cvar::AssignHardValue(bool value) {
	b.AssignBoth(value);
	if(ptbChangeCallback)
		ptbChangeCallback(value); 
}

void Cvar::SetValue(char* value) { 
	if(type != CV_STRING) return; 
	strncpy(s.currentVal, value, sizeof(s.currentVal)); 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %s\n", name.c_str(), value); 
	RunCallback();
}

void Cvar::SetValue(int value) { 
	if(type != CV_INTEGER) return; 
	i.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %i\n", name.c_str(), value); 
	RunCallback();
}

void Cvar::SetValue(float value) { 
	if(type != CV_FLOAT) return; 
	v.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %f\n", name.c_str(), value); 
	RunCallback();
}

void Cvar::SetValue(bool value) { 
	if(type != CV_BOOLEAN) return; 
	b.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %s\n", name.c_str(), btoa(value)); 
	RunCallback();
}

void Cvar::AddCallback(void* function) {
	switch(type) {
		default:
		case CV_STRING:
			ptsChangeCallback = (void(*)(char*))function;
			break;
		case CV_INTEGER:
			ptiChangeCallback = (void(*)(int))function;
			break;
		case CV_FLOAT:
			ptfChangeCallback = (void(*)(float))function;
			break;
		case CV_BOOLEAN:
			ptbChangeCallback = (void(*)(bool))function;
			break;
	}
}

void Cvar::RunCallback() {
	switch(type) {
		default:
		case CV_STRING:
			if(ptsChangeCallback) {
				ptsChangeCallback(s.currentVal);
			}
			break;
		case CV_INTEGER:
			if(ptiChangeCallback) {
				ptiChangeCallback(i.currentVal);
			}
			break;
		case CV_FLOAT:
			if(ptfChangeCallback) {
				ptfChangeCallback(v.currentVal);
			}
			break;
		case CV_BOOLEAN:
			if(ptbChangeCallback) {
				ptbChangeCallback(b.currentVal);
			}
			break;
	}
}